/*
 * Copyright (c) 2004 Dan Villiom Podlaski Christiansen <danchr@daimi.au.dk>
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

#define LATENCY_FRAMES 8192

#define super OSObject	

OSDefineMetaClassAndStructors(ODAudioBSDClient, OSObject);

int ODAudioBSDClient::ninitialised = 0;
static int nwrites = 0;


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

uint64_t ODAudioBSDClient::bytesToNanos(uint64_t bytes)
{
  return bytesToNanos(this->outputstream->getFormat(), bytes);
}

uint64_t ODAudioBSDClient::bytesToFrames(uint64_t bytes)
{
  return bytesToFrames(this->outputstream->getFormat(), bytes);
}

uint64_t ODAudioBSDClient::framesToBytes(uint64_t frames)
{
  return framesToBytes(this->outputstream->getFormat(), frames);
}

uint64_t ODAudioBSDClient::nanosToBytes(uint64_t nanos)
{
  return nanosToBytes(this->outputstream->getFormat(), nanos);
}

uint64_t ODAudioBSDClient::bytesToFrames(const IOAudioStreamFormat *f,
					 uint64_t bytes)
{
  return bytes / f->fBitDepth * 8 / f->fNumChannels;
}

uint64_t ODAudioBSDClient::framesToBytes(const IOAudioStreamFormat *f,
					 uint64_t frames)
{
  return frames * f->fBitDepth / 8 * f->fNumChannels;
}

uint64_t ODAudioBSDClient::bytesToNanos(const IOAudioStreamFormat *f,
					 uint64_t bytes)
{
  const IOAudioSampleRate *rate = this->engine->getSampleRate();
  return bytes * max(rate->fraction,1) / (f->fBitDepth / 8) / f->fNumChannels
    * 1000000000LL / rate->whole;
}

uint64_t ODAudioBSDClient::nanosToBytes(const IOAudioStreamFormat *f,
					 uint64_t nanos)
{
  const IOAudioSampleRate *rate = this->engine->getSampleRate();
  return nanos * rate->whole * f->fNumChannels * (f->fBitDepth / 8)
    / max(rate->fraction, 1) / 1000000000LL;
}

AbsoluteTime ODAudioBSDClient::getTime()
{
  AbsoluteTime r;
  engine->getLoopCountAndTimeStamp(&loopcount, &r);
  clock_get_uptime(&r);
  return r;
}

ODAudioBSDClient *ODAudioBSDClient::withAudioEngine(IOAudioEngine *engine,
						    int minor)
{
  DEBUG_FUNCTION();

  if (!engine)
    return NULL;

  ODAudioBSDClient *client = new ODAudioBSDClient;  

  if (!client->init()) {
    DEBUG("Failed to initialise client!\n");
    client->release();
    return NULL;
  }

  if (!ninitialised++)
    major = cdevsw_add(major, &chardev);

  if (major < 0) {
    DEBUG("Failed to allocate major device number!\n");
    client->release();
    return NULL;
  }

  client->engine = engine;
  client->minor = minor;
  client->is_open = false;

  client->devnode =
    devfs_make_node(makedev(major, client->minor),
		    DEVFS_CHAR, UID_ROOT, GID_OPERATOR, 
		    UMASK, "dsp%x", client->minor);

  if (!client->devnode) {
    DEBUG("Failed to allocate minor device number!\n");
    client->release();
    return NULL;
  }
 
  if (client->minor == 0)
    devfs_link(client->devnode, "dsp");

  DEBUG("ODAudioBSDClient successfully initialised (%u/%u)\n",
	major, client->minor);

  return client;
}

void ODAudioBSDClient::free()
{
  DEBUG_FUNCTION();

  if (this->devnode)
    devfs_remove(this->devnode);

  if (!--ninitialised && major != -1)
    cdevsw_remove(major, &chardev);

  super::free();
}


bool ODAudioBSDClient::open()
{
  DEBUG_FUNCTION();

  if (this->is_open || !engine->outputStreams) {
    DEBUG("%s is in use!\n", engine->getName());
    return false;
  }
  
  this->outputstream =
    CAST(IOAudioStream, engine->outputStreams->getCount() > 0 ?
	 engine->outputStreams->getObject(0) : NULL);

  if (!this->outputstream) return false;
  nwrites = 0;

  this->is_open = true;

  DEBUG("Opening %s!\n", engine->getName());

  return true;
}

void ODAudioBSDClient::close()
{
  DEBUG_FUNCTION();
  DEBUG("Closing %s!\n", engine->getName());

  this->is_open = false;
  engine->performFlush();
}

int ODAudioBSDClient::write(struct uio *uio)
{
  DEBUG_FUNCTION();

  IOAudioEnginePosition *endpos = &engine->audioEngineStopPosition;
  AbsoluteTime now = getTime();
  unsigned int buflen = outputstream->getSampleBufferSize();
  void *buf = outputstream->getSampleBuffer();
  nwrites++;

  {
    uint64_t t1, t2;
    
    absolutetime_to_nanoseconds(next_call, &t1);
    absolutetime_to_nanoseconds(now, &t2);

    if (t2 < t1) {
#if 1
      unsigned delay = t1 - t2;
      int frame_diff = 
	(endpos->fLoopCount * engine->numSampleFramesPerBuffer +
	 endpos->fSampleFrame) - 
	(engine->getStatus()->fCurrentLoopCount * engine->numSampleFramesPerBuffer + engine->getCurrentSampleFrame());

      if (frame_diff <= 0) {
	      nwrites, (unsigned)-frame_diff);
	delay = 0;
      } else if (frame_diff < LATENCY_FRAMES) {
	unsigned correction = 
	  bytesToNanos(framesToBytes(LATENCY_FRAMES - frame_diff));
	DEBUG("RACE CONDITION EMMINENT: %u %u off %u corrected\n",
	      nwrites, (unsigned)frame_diff, correction);
	delay -= (delay > correction) ? correction : delay;
      }


      IOSleep(delay / 1000000);
      IODelay((delay / 1000) % 1000);
#else
      DEBUG("%u \n", (unsigned)(tick/delay));
      //tsleep(this, 0, "audio", (t1 - t2) / 1000);
#endif
    } else {
      DEBUG("NOT SLEEPING\n");
    }
  }

  if (engine->getState() != kIOAudioEngineRunning) {
    DEBUG("%u: Restarting %s!\n", nwrites, engine->getName());

    engine->stopAudioEngine();
    engine->clearAllSampleBuffers();
    engine->resetStatusBuffer();
    getTime();

    endpos->fSampleFrame = engine->getCurrentSampleFrame();
    endpos->fLoopCount = loopcount + 1;

    next_call = now;

    bzero(buf, buflen);
  }

  unsigned int offset = framesToBytes(endpos->fSampleFrame);  
  /* calculate offset */
  unsigned written = 
    MIN(MIN(buflen - offset, (uint64_t)uio->uio_resid & 0xffffff00),
	buflen / engine->numErasesPerBuffer);
  int r = uiomove((caddr_t)buf + offset, written, uio);
  uint64_t delay = bytesToNanos(written);
  int64_t correction = 0;

  /* pospone end of playback */
  if (offset + written == buflen) {
    endpos->fSampleFrame = 0;
    endpos->fLoopCount++;
  } else {
    endpos->fSampleFrame = bytesToFrames(offset + written);
  }

  switch (CMP_ABSOLUTETIME(&now, &next_call)) {
  case 1: {
    /* now > next_call */
    AbsoluteTime diff = now;
    uint64_t d;

    SUB_ABSOLUTETIME(&diff, &next_call);
    absolutetime_to_nanoseconds(diff, &d);

    if (delay < d)
      DEBUG("%u: now > next_call: c=%llu d=%llu\n", nwrites, d, delay);

#if 0
    correction = (delay >= d) ? -d : -delay;
#else
    correction = (delay >= d) ? -d : 0;
#endif

    break;
  }
  case -1: {
    /* now < next_call 
     * this hardly ever happens */
    AbsoluteTime diff = next_call;
    uint64_t d;

    SUB_ABSOLUTETIME(&diff, &now);
    absolutetime_to_nanoseconds(diff, &d);
    correction = d;

    if (delay < d)
      DEBUG("%u: now < next_call: delay=%llu d=%llu\n", nwrites, delay, d);

    break;
  }
  case 0:
    /* now == next_call */
    DEBUG("%u: now = next_call: d=%llu %llu == %llu\n", nwrites, delay,
	  AbsoluteTime_to_scalar(&now), AbsoluteTime_to_scalar(&next_call));
    correction = -delay;
    break;
  default:
    /* now ?? next_call */
    DEBUG("%u: now ?? next_call\n", nwrites);
    break;
  }
  next_call = now;
  nanoseconds_to_absolutetime(delay + correction, &now);
  ADD_ABSOLUTETIME(&next_call, &now);

  delay += correction;

  if (engine->getState() != kIOAudioEngineRunning) {
    engine->startAudioEngine();
    engine->performFlush();
  }

#if 0
  int frame_diff = endpos->fLoopCount * endpos->fSampleFrame - 
    (engine->getStatus())->fCurrentLoopCount *  engine->getCurrentSampleFrame();

  if (frame_diff < 0) {
    DEBUG("RACE CONDITION DETECTED: %u %u off\n",
	  nwrites, (unsigned)-frame_diff);
    delay = 0;
  } else {
    IOSleep(delay / 1000000);
    IODelay((delay / 1000) % 1000);
  }
#endif

  return r;
}
