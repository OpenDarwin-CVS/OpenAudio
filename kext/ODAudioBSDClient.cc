/*
 * Copyright (c) 2004 Dan Villiom Podlaski Christiansen
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "ODAudioBSDClient.h"

#include "ds_module.h"
#include "ds_util.h"

#include <IOKit/audio/IOAudioEngine.h>
#include <IOKit/audio/IOAudioStream.h>
#include <IOKit/IOWorkLoop.h>

#define super OSObject	

OSDefineMetaClassAndStructors(ODAudioBSDClient, OSObject);

int ODAudioBSDClient::ninitialised = 0;

/* A struct describing which functions will get invoked for certain
 * actions.
 */
struct cdevsw ODAudioBSDClient::chardev = {
  ds_open,         /* open */
  ds_close,        /* close */
  eno_rdwrt,       /* read */
  ds_write,        /* write */
  eno_ioctl,       /* ioctl */
  eno_stop,        /* stop */
  eno_reset,       /* reset */
  NULL,            /* tty's */
  eno_select,      /* select */
  eno_mmap,        /* mmap */
  eno_strat,       /* strategy */
  eno_getc,        /* getc */
  eno_putc,        /* putc */
  0                /* type */
};

int ODAudioBSDClient::major = -1;

const char *ODAudioBSDClient::statusString()
{
  DEBUG_FUNCTION();

  switch (engine->getState()) {
  case kIOAudioEngineStopped:
    return "stopped";
  case kIOAudioEngineRunning:
    return "running";
  case kIOAudioEnginePaused:
    return "paused";
  case kIOAudioEngineResumed:
    return "resumed";
  default:
    return "weird";
  }
}

int ODAudioBSDClient::bytesToFrames(const IOAudioStreamFormat *f, int bytes)
{
  return bytes * f->fBitDepth / 8 * f->fNumChannels;
}

int ODAudioBSDClient::calculateDelayMicros(const IOAudioStreamFormat *f,
					   int bytes)
{
  const IOAudioSampleRate *rate = this->engine->getSampleRate();
  uint64_t delay;

  delay = bytes;
  delay *= max(rate->fraction,1);
  delay /= f->fBitDepth / 8;
  delay /= f->fNumChannels;
  delay *= 1000000;
  delay /= rate->whole;

#if 0
  DEBUG("rate=%u/%u bytes=%u delay=%llu\n",
	(int)rate->whole, (int)rate->fraction, bytes, delay);
#endif

  return (int)delay;
}

ODAudioBSDClient *ODAudioBSDClient::withAudioEngine(IOAudioEngine *engine,
						    int minor)
{
  DEBUG_FUNCTION();

  ODAudioBSDClient *driver = new ODAudioBSDClient;

  if (!engine)
    return NULL;
  
  if (!driver->init()) {
    driver->release();
    return NULL;
  }

  if (!ninitialised++)
    major = cdevsw_add(major, &chardev);
  
  if (major < 0) {
    driver->release();
    return NULL;
  }

  driver->engine = engine;
  driver->minor = minor;
  driver->is_open = false;

  driver->devnode =
    devfs_make_node(makedev(major, driver->minor),
		    DEVFS_CHAR, UID_ROOT, GID_OPERATOR, 
		    UMASK, "dsp%x", driver->minor);

  if (!driver->devnode) {
    driver->release();
    return NULL;
  }
 
  if (driver->minor == 0)
    devfs_link(driver->devnode, "dsp");

  return driver;
}

void ODAudioBSDClient::free()
{
  DEBUG_FUNCTION();

  if (this->devnode)
    devfs_remove(this->devnode);

  if (!ninitialised-- && major != -1)
    cdevsw_remove(major, &chardev);

  super::free();
}


bool ODAudioBSDClient::open()
{
  DEBUG_FUNCTION();

  if (this->is_open || !engine->outputStreams) {
    return false;
  }
  
  this->outputstream =
    CAST(IOAudioStream, engine->outputStreams->getCount() > 0 ?
	 engine->outputStreams->getObject(0) : NULL);

  if (!outputstream) return false;

  this->is_open = true;
  engine->startAudioEngine();

  return true;
}

void ODAudioBSDClient::close()
{
  DEBUG_FUNCTION();

  engine->stopAudioEngine();

  this->is_open = false;
}

int ODAudioBSDClient::write(struct uio *uio)
{
  DEBUG_FUNCTION();

  AbsoluteTime time, interval;
  UInt32 loopcount;
  IOAudioEnginePosition endpos;
  uint64_t n;
  int bytes = MIN((long)outputstream->getSampleBufferSize(),
		  (long)uio->uio_resid);

  int r = uiomove((caddr_t)this->outputstream->getSampleBuffer(), bytes, uio);

  n = calculateDelayMicros(outputstream->getFormat(), bytes);

  nanoseconds_to_absolutetime(n * 1000, &interval);

  engine->getLoopCountAndTimeStamp(&loopcount, &time);
  //ADD_ABSOLUTETIME(&time, &interval);
  //engine->takeTimeStamp(true, &time);

  endpos.fSampleFrame = bytesToFrames(outputstream->getFormat(), bytes);
  endpos.fLoopCount = loopcount + 2;

  if (!engine->getState() == kIOAudioEngineRunning) {
    DEBUG("%s: loopcount=%u n=%llu\n", __FUNCTION__,
	  (int)loopcount, AbsoluteTime_to_scalar(&time));
    engine->startAudioEngine();
  }

  engine->stopEngineAtPosition(&endpos);

  IOSleep(n / 1000);

  return r;
}

        
#if 0
    IOReturn ior = engine->mixOutputSamples((char *)buf,
					    outputstream->getMixBuffer(),
					    0, 1,
					    &outputstream->format,
					    outputstream);

    if (ior != kIOReturnSuccess) {
      debug("%s: MIX FAILED\n", __FUNCTION__);
    } else {
      debug("%u written to a %s %s: %u\n",
	    bytes, statusString(), engine->getName(), r);
    }
#endif
