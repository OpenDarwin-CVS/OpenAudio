/*
 * OpenDarwin Audio BSD Client
 * An IOService bridging an IOAudioEngine with an entry in /dev
 *
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

#include "audio_util.h"

#include "audio/audio_ioctl.h"

#include <IOKit/audio/IOAudioEngine.h>
#include <IOKit/audio/IOAudioDefines.h>
#include <IOKit/audio/IOAudioStream.h>
#include <IOKit/audio/IOAudioControl.h>
#include <IOKit/IOWorkLoop.h>

extern "C" {
#include <sys/param.h>
#include <sys/uio.h>
#include <sys/proc.h>
}

#define LATENCY_FRAMES 8192

#define super ODBSDClient

OSDefineMetaClassAndStructors(ODAudioBSDClient, ODBSDClient);
static int nwrites = 0;


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

uint64_t ODAudioBSDClient::bytesToFrames(uint64_t bytes)
{
  const IOAudioStreamFormat *f = this->outputstream->getFormat();
  return bytes / f->fBitDepth * 8 / f->fNumChannels;
}

uint64_t ODAudioBSDClient::framesToBytes(uint64_t frames)
{
  const IOAudioStreamFormat *f = this->outputstream->getFormat();
  return frames * f->fBitDepth / 8 * f->fNumChannels;
}

uint64_t ODAudioBSDClient::bytesToNanos(uint64_t bytes)
{
  const IOAudioStreamFormat *f = this->outputstream->getFormat();
  const IOAudioSampleRate *rate = this->engine->getSampleRate();
  return bytes * max(rate->fraction,1) / (f->fBitDepth / 8) / f->fNumChannels
    * 1000000000LL / rate->whole;
}

uint64_t ODAudioBSDClient::nanosToBytes(uint64_t nanos)
{
  const IOAudioStreamFormat *f = this->outputstream->getFormat();
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

#if 0
ODAudioBSDClient *ODAudioBSDClient::withAudioEngine(IOAudioEngine *engine,
						    int minor)
{
  DEBUG_FUNCTION();

  if (!engine)
    return NULL;

  ODAudioBSDClient *client = new ODAudioBSDClient;  

  if (!client->init()) {
    IOLog("Failed to initialise client!\n");
    client->release();
    return NULL;
  }

  if (!ninitialised++)
    major = cdevsw_add(major, &chardev);

  if (major < 0) {
    IOLog("Failed to allocate major device number!\n");
    client->release();
    return NULL;
  }

  client->engine = engine;
  client->minor = minor;

  client->is_open = false;

  client->devnode =
    devfs_make_node(makedev(major, client->minor),
		    DEVFS_CHAR, UID_ROOT, GID_OPERATOR, 
		    UMASK, DEVICE_NAME "%x", client->minor);

  if (!client->devnode) {
    IOLog("Failed to allocate minor device number!\n");
    client->release();
    return NULL;
  }
 
  if (client->minor == 0)
    devfs_link(client->devnode, DEVICE_NAME);

  IOLog("Engine has %u controls\n", engine->defaultAudioControls->getCount());

  OSIterator *i =
    OSCollectionIterator::withCollection(engine->defaultAudioControls);

  DEBUG("ODAudioBSDClient successfully initialised for %s (%u/%u)\n",
	engine->getName(), major, client->minor);

  return client;
}
#endif

bool ODAudioBSDClient::start(IOService *provider)
{
  DEBUG_FUNCTION();

  if (!super::start(provider)) return false;

  /* obtain engine and output stream */
  engine = CAST(IOAudioEngine, this->getProvider());

  if (!engine || !engine->outputStreams) return false;

  this->outputstream =
    CAST(IOAudioStream, engine->outputStreams->getCount() > 0 ?
	 engine->outputStreams->getObject(0) : NULL);

  if (!this->outputstream) return false;

  nwrites = 0;

  /* obtain audio controls */

  if (engine->defaultAudioControls && 
      engine->defaultAudioControls->getCount() > 0) {
    IOLog("Engine has %u controls\n", engine->defaultAudioControls->getCount());

    OSIterator *i =
      OSCollectionIterator::withCollection(engine->defaultAudioControls);
    
    for (unsigned n = 0; n < engine->defaultAudioControls->getCount(); n++) {
      IOAudioControl *c = CAST(IOAudioControl, i->getNextObject());

      unsigned t = c->getType(), s = c->getSubType(), u = c->getUsage();

      IOLog("%s:%u type=%.4s subtype=%.4s usage=%.4s\n", c->getName(),
	    n, (char *)&t, (char *)&s, (char *)&u);
    }

  } else {
    IOLog("Engine has no audio controls!\n");
  }

  /* pair with the engine */
  setProperty(kIOAudioEngineGlobalUniqueIDKey, engine->getGlobalUniqueID());

  return true;
}

int ODAudioBSDClient::open(int flags, int devtype, struct proc *pp)
{
  DEBUG_FUNCTION();

  /* declared and maintained by the VFS subsystem */
  extern uid_t console_user;

  /* only allow root and the console user to access the device */
  if (pp->p_ucred->cr_uid && console_user != pp->p_ucred->cr_uid)
    return EACCES;


  if (this->is_open) return EBUSY;

  this->is_open = true;

  DEBUG("Opening %s!\n", engine->getName());

  return 0;
}

int ODAudioBSDClient::close(int flags, int mode, struct proc *pp)
{
  DEBUG_FUNCTION();

  this->is_open = false;
  engine->performFlush();

  return 0;
}

int ODAudioBSDClient::write(struct uio *uio, int ioflag)
{
  DEBUG_FUNCTION();

  IOAudioEnginePosition endpos = engine->audioEngineStopPosition;
  AbsoluteTime now = getTime();
  unsigned int buflen = outputstream->getSampleBufferSize();
  void *buf = outputstream->getSampleBuffer();
  nwrites++;

  /* delay until next_call */
  {
    uint64_t t1, t2;
    
    absolutetime_to_nanoseconds(next_call, &t1);
    absolutetime_to_nanoseconds(now, &t2);

    if (t2 < t1) {
      unsigned delay = t1 - t2;
      int frame_diff = 
	(endpos.fLoopCount * engine->numSampleFramesPerBuffer +
	 endpos.fSampleFrame) - 
	(engine->getStatus()->fCurrentLoopCount * 
	 engine->numSampleFramesPerBuffer + engine->getCurrentSampleFrame());

      if (frame_diff <= 0) {
	/* The end position has been overtaken by the current frame.
	 * This means we're behind and shouldn't wait at all */
	VERBOSE_DEBUG("RACE CONDITION DETECTED: %u %u off\n",
		      nwrites, -frame_diff);
	delay = 0;
      } else if (frame_diff < LATENCY_FRAMES) {
	/* The end position is less than LATENCY_FRAMES ahead of the
	 * current frame. LATENCY_FRAMES is the safe value we chose to
	 * avoid skipping. Decrease the delay to ensure the latency. */
	unsigned correction = 
	  bytesToNanos(
		       framesToBytes(LATENCY_FRAMES - frame_diff));
	VERBOSE_DEBUG("RACE CONDITION EMMINENT: %u %u off %u corrected\n",
		      nwrites, (unsigned)frame_diff, correction);
	delay -= (delay > correction) ? correction : delay;
      }

#if 1
      IOSleep(delay / 1000000);
      IODelay((delay / 1000) % 1000);
#else
      VERBOSE_DEBUG("%u \n", (unsigned)(tick/delay));
      //tsleep(this, 0, "audio", (t1 - t2) / 1000);
#endif
    } else {
      VERBOSE_DEBUG("NOT SLEEPING\n");
    }
  }

  /* The audio engine is stopped. This means that a) there is nothing
   * buffered so we should clear it and b) we should restart it when
   * we copied the audio to it. */
  if (engine->getState() != kIOAudioEngineRunning) {
    if (nwrites > 1)
      IOLog("%u: Restarting %s!\n", nwrites, engine->getName());
    else
      IOLog("Starting %s!\n", engine->getName());

    /* stop it */
    engine->stopAudioEngine();
    engine->clearAllSampleBuffers();
    engine->resetStatusBuffer();
    getTime();

    /* there is nothing buffered */
    endpos.fSampleFrame = engine->getCurrentSampleFrame();
    endpos.fLoopCount = loopcount + 1;

    /* there is no previous call */
    next_call = now;

    /* just in case */
    bzero(buf, buflen);
  }

  unsigned int offset = framesToBytes(endpos.fSampleFrame);  
  /* calculate offset */
  unsigned written = 
    min(min(buflen - offset, (uint64_t)uio->uio_resid),
	buflen / engine->numErasesPerBuffer);
  int r = uiomove((caddr_t)buf + offset, written, uio);
  uint64_t delay = bytesToNanos(written);
  int64_t correction = 0;

  /* pospone end of playback */
  if (offset + written == buflen) {
    endpos.fSampleFrame = 0;
    endpos.fLoopCount++;
  } else {
    endpos.fSampleFrame = bytesToFrames(offset + written);
  }

  engine->audioEngineStopPosition = endpos;
  engine->stopEngineAtPosition(&engine->audioEngineStopPosition);

  /* handle the three different possible relations between the chosen
   * time for this call, 'next_call', and the actual time for this call,
   * 'now'. */
  switch (CMP_ABSOLUTETIME(&now, &next_call)) {
  case 1: {
    /* now > next_call - we are late */
    AbsoluteTime diff = now;
    uint64_t d;

    SUB_ABSOLUTETIME(&diff, &next_call);
    absolutetime_to_nanoseconds(diff, &d);

    if (delay < d)
      VERBOSE_DEBUG("%u: now > next_call: c=%llu d=%llu\n", nwrites, d, delay);

#if 0
    correction = (delay >= d) ? -d : -delay;
#else
    correction = (delay >= d) ? -d : 0;
#endif

    break;
  }
  case -1: {
    /* now < next_call - we are a bit early  */
    AbsoluteTime diff = next_call;
    uint64_t d;

    SUB_ABSOLUTETIME(&diff, &now);
    absolutetime_to_nanoseconds(diff, &d);
    correction = d;

    if (delay < d)
      VERBOSE_DEBUG("%u: now < next_call: delay=%llu d=%llu\n",
		    nwrites, delay, d);

    break;
  }
  case 0:
    /* now == next_call - the two times are equal. This ONLY happens
     * when the audio engine was just started, so we don't wait at
     * all.  */
    VERBOSE_DEBUG("%u: now = next_call: d=%llu %llu == %llu\n", nwrites, delay,
		  AbsoluteTime_to_scalar(&now),
		  AbsoluteTime_to_scalar(&next_call));
    correction = -delay;
    break;
  default:
    /* now ?? next_call - undefined behaviour */
    IOLog("%s: call #%u => now ?? next_call\n", engine->getName(), nwrites);
    break;
  }

  /* calculate the time of the next call */
  delay += correction;
  next_call = now;
  nanoseconds_to_absolutetime(delay, &now);
  ADD_ABSOLUTETIME(&next_call, &now);

  if (engine->getState() != kIOAudioEngineRunning) {
    engine->startAudioEngine();
    engine->performFlush();
  }

  return r;
}

int ODAudioBSDClient::ioctl(u_long cmd, caddr_t data, int fflag, struct proc *p)
{
  DEBUG_FUNCTION();

  int r = kIOReturnSuccess;

  switch (cmd) {
  case AUDIOGETOFMT: {
    const IOAudioStreamFormat *f = outputstream->getFormat();
    DEBUG("AUDIOGETOFMT\n");
    if (f)
      bcopy(f, data, sizeof(IOAudioStreamFormat));
    else
      r = EINVAL;
    break;
  }

  case AUDIOSETOFMT:
    DEBUG("AUDIOSETOFMT\n");
    r = outputstream->setFormat((IOAudioStreamFormat *)data) ? EINVAL : r;
    break;

  case AUDIOGETIFACES:
  case AUDIOGETVOL:
  case AUDIOSETVOL:
  case AUDIOGETMUTE:
  case AUDIOSETMUTE:
  default:
    DEBUG("!?\n");
    r = ENOTTY;
  }

  return r;
}

const char *ODAudioBSDClient::getDeviceBase() const
{
  return "dsp";
}
