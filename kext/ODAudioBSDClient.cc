/*
 * OpenDarwin Audio BSD Client
 * An IOService bridging an IOAudioEngine with an entry in /dev
 *
 * Copyright (c) 2004 Dan Villiom Podlaski Christiansen <danchr@opendarwin.org>
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

#include "../include/audioio.h"

#include <IOKit/IOLib.h>
#include <IOKit/audio/IOAudioEngine.h>
#include <IOKit/audio/IOAudioDefines.h>
#include <IOKit/audio/IOAudioStream.h>
#include <IOKit/audio/IOAudioControl.h>

extern "C" {
#include <sys/param.h>
#include <sys/uio.h>
#include <sys/proc.h>

#include <sys/fcntl.h>
#include <sys/errno.h>
#include <sys/filio.h>
}

#define super ODBSDClient

/* this definition toggles a hack which makes us assume complete
   control over the audio engine */
#define NICE_LOCKS 0

OSDefineMetaClassAndStructors(ODAudioBSDClient, ODBSDClient);

static int nwrites = 0;

static bool check_channel_id(IOAudioControl *c, int id)
{
  DEBUG_FUNCTION();
  bool b = id != kIOAudioControlChannelNumberInactive &&
    (int)c->getChannelID() != kIOAudioControlChannelNumberInactive &&
    (id == kIOAudioControlChannelIDAll || id == (int)c->getChannelID());

  DEBUG("c-id()=%u %s id=%u\n", (int)c->getChannelID(), b ? "==" : "!=", id);

  return b;
}

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

int ODAudioBSDClient::bytesToFrames(int bytes)
{
  DEBUG_FUNCTION();

  const IOAudioStreamFormat *f = this->outputstream->getFormat();
  return bytes / f->fBitDepth * 8 / f->fNumChannels;
}

int ODAudioBSDClient::framesToBytes(int frames)
{
  DEBUG_FUNCTION();

  const IOAudioStreamFormat *f = this->outputstream->getFormat();
  return frames * f->fBitDepth / 8 * f->fNumChannels;
}

int ODAudioBSDClient::bytesToNanos(int bytes)
{
  DEBUG_FUNCTION();

  const IOAudioStreamFormat *f = this->outputstream->getFormat();
  const IOAudioSampleRate *rate = this->engine->getSampleRate();
  return bytes * max(rate->fraction,1) / (f->fBitDepth / 8) / f->fNumChannels
    * 1000000000LL / rate->whole;
}

int ODAudioBSDClient::nanosToBytes(int nanos)
{
  DEBUG_FUNCTION();

  const IOAudioStreamFormat *f = this->outputstream->getFormat();
  const IOAudioSampleRate *rate = this->engine->getSampleRate();
  return nanos * rate->whole * f->fNumChannels * (f->fBitDepth / 8)
    / max(rate->fraction, 1) / 1000000000LL;
}

AbsoluteTime ODAudioBSDClient::getTime()
{
  DEBUG_FUNCTION();

  AbsoluteTime r;
  clock_get_uptime(&r);
  return r;
}

void ODAudioBSDClient::reset()
{
  DEBUG_FUNCTION();

  /* stop it */
  engine->stopAudioEngine();

  /* reset it */
  engine->clearAllSampleBuffers();
  engine->resetStatusBuffer();

  /* there is nothing buffered */
  engine->audioEngineStopPosition.fSampleFrame = 0;
  engine->audioEngineStopPosition.fLoopCount = 0;

  /* there is no previous call */
  next_call = getTime();

  /* just in case */
  bzero(outputstream->getSampleBuffer(), outputstream->getSampleBufferSize());
}

bool ODAudioBSDClient::start(IOService *provider)
{
  DEBUG_FUNCTION();

  this->owner = -1;
  this->is_open = false;
  this->blocking = true;

  if (!super::start(provider)) return false;

  /* obtain engine and output stream */
  engine = CAST(IOAudioEngine, this->getProvider());

  if (!engine || !engine->outputStreams) return false;

  this->outputstream =
    CAST(IOAudioStream, engine->outputStreams->getCount() > 0 ?
	 engine->outputStreams->getObject(0) : NULL);

  if (!this->outputstream) return false;

  nwrites = 0;

  this->chunksize =
    outputstream->getSampleBufferSize() / engine->numErasesPerBuffer;

  IOLog("chunksize=%u bufsize=%u\n", 
	chunksize, (unsigned)outputstream->getSampleBufferSize());

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

  /* only allow root and the console user to access the device */
  if (pp->p_ucred->cr_uid && this->getOwner() != pp->p_ucred->cr_uid)
    return EACCES;

  if (this->is_open && (flags & FWRITE)) return EBUSY;

  this->blocking = !(flags & FNONBLOCK);
  this->is_open = true;

  IOLog("Opened %s %sblocking%s\n", engine->getName(),
	this->blocking ? "" : "non",
	flags & FWRITE ? " with write-access" : "");

#if !NICE_LOCKS
  /* we assume complete control over the device */
  engine->lockAllStreams();
#endif

  return 0;
}

int ODAudioBSDClient::close(int flags, int mode, struct proc *pp)
{
  DEBUG_FUNCTION();

  this->is_open = false;
  engine->performFlush();

#if !NICE_LOCKS
  /* we give up control of the device */
  engine->unlockAllStreams();
#endif

  return 0;
}

int ODAudioBSDClient::getBufferedFrames()
{
  DEBUG_FUNCTION();

  return
    (engine->audioEngineStopPosition.fLoopCount *
     engine->numSampleFramesPerBuffer +
     engine->audioEngineStopPosition.fSampleFrame) - 
    (engine->getStatus()->fCurrentLoopCount * 
     engine->numSampleFramesPerBuffer + engine->getCurrentSampleFrame());
}

int ODAudioBSDClient::getLatency()
{
  DEBUG_FUNCTION();

  return 
    bytesToNanos(framesToBytes(this->getBufferedFrames()));
}

int ODAudioBSDClient::getDelay(AbsoluteTime now)
{
  DEBUG_FUNCTION();

  uint64_t t1, t2;

  absolutetime_to_nanoseconds(next_call, &t1);
  absolutetime_to_nanoseconds(now, &t2);

  if (engine->getState() != kIOAudioEngineRunning || t2 >= t1) {
    DEBUG("NO DELAY\n");
    return 0;
  } else {
    unsigned delay = t1 - t2;
    int byte_diff = framesToBytes(this->getBufferedFrames());
    const int min_diff = outputstream->getSampleBufferSize() / 2;

    if (byte_diff < min_diff) {
      /* The end position is in the same half of the buffer as the
       * current frame. This can cause race conditions, so we decrease
       * the delay to ensure the latency. */
      unsigned correction = bytesToNanos(min_diff - byte_diff);
      DEBUG("%u: delay=%u correction=%u\n",
	    nwrites, delay, correction);
      if (blocking && delay > correction) {
	delay -= correction;
      } else {
	/* the correction is so large that we shouldn't wait */
	delay = 0;
      }
    } else if (byte_diff < chunksize) {
      /* The end position is in the same quarter as the current
       * frame. This means we're behind and shouldn't wait. */
      IOLog("RACE CONDITION PREVENTED: %u diff=%u\n", nwrites, byte_diff);
      delay = 0;
    } else if (byte_diff < 0) {
      /* The end position has been overtaken by the current frame.
       * This also means we shouldn't wait at all. */
      IOLog("RACE CONDITION DETECTED: %u diff=-%u\n", nwrites, -byte_diff);
      delay = 0;
    } 

    return delay;
  }
}

int ODAudioBSDClient::write(struct uio *uio, int ioflag)
{
  DEBUG_FUNCTION();

#if NICE_LOCKS
  IORecursiveLockLock(outputstream->streamIOLock);
#endif

  /* The audio engine is stopped. This means that a) there is nothing
   * buffered so we should clear it and b) we should restart it when
   * we copied the audio to it. */
  if (engine->getState() != kIOAudioEngineRunning) {
    IOLog("%u: Restarting %s!\n", nwrites, engine->getName());
    this->reset();
  }

  IOAudioEnginePosition endpos = engine->audioEngineStopPosition;
  AbsoluteTime now = getTime();
  unsigned buflen = outputstream->getSampleBufferSize();
  void *buf = outputstream->getSampleBuffer();
  unsigned delay = this->getDelay(now);
  nwrites++;

  if (!this->blocking && delay)
    return EAGAIN;

  /* calculate offset */
  unsigned offset = framesToBytes(endpos.fSampleFrame);  
  unsigned written = min(chunksize, uio->uio_resid);
  int r;
  int64_t correction = 0;

  /* pospone end of playback */
  if (buflen - offset < written) {
    unsigned avail = buflen - offset;
    r = uiomove((caddr_t)buf + offset, avail, uio);
    endpos.fSampleFrame = engine->numSampleFramesPerBuffer;
    engine->performFlush();
    r = uiomove((caddr_t)buf, written - avail, uio);
    offset = written - avail;
    endpos.fLoopCount++;
  } else {
    r = uiomove((caddr_t)buf + offset, written, uio);
    offset += written;
  }

  endpos.fSampleFrame = bytesToFrames(offset);

#if 1
  /* start audio engine */
  if (engine->getState() != kIOAudioEngineRunning)
    engine->startAudioEngine();
  engine->stopEngineAtPosition(&endpos);
#endif

#if NICE_LOCKS
  /* HACK: by locking the audio stream when sleeping, we ensure that
     it isn't stopped. */
  IORecursiveLockUnlock(outputstream->streamIOLock);
#endif

  /* delay until next_call */
  if (this->blocking) {
    IOSleep(delay / 1000000);
    IODelay((delay / 1000) % 1000);
  }

#if NICE_LOCKS
  IORecursiveLockLock(outputstream->streamIOLock);
#endif

  /* begin calculating the next delay */
  delay = bytesToNanos(written);

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

#if 0
  if (engine->getState() != kIOAudioEngineRunning) {
    engine->startAudioEngine();
    engine->performFlush();
  }
#endif

#if NICE_LOCKS
  IORecursiveLockUnlock(outputstream->streamIOLock);
#endif
  
  return r;
}

int ODAudioBSDClient::ioctl(u_long cmd, caddr_t data, int fflag, struct proc *p)
{
  DEBUG_FUNCTION();

#if NICE_LOCKS
  engine->lockAllStreams();
#endif
  
  int r = 0;

  switch (cmd) {
  case AUDIOGETOFMT: {
    const IOAudioStreamFormat *f = outputstream->getFormat();
    VERBOSE_DEBUG("AUDIOGETOFMT\n");
    if (f)
      bcopy(f, data, sizeof(IOAudioStreamFormat));
    else
      r = EINVAL;
    break;
  }

  case AUDIOSETOFMT:
    VERBOSE_DEBUG("AUDIOSETOFMT\n");
    r = outputstream->setFormat((IOAudioStreamFormat *)data) ? EINVAL : 0;
    break;

  case AUDIOLATENCY:
    VERBOSE_DEBUG("AUDIOLATENCY\n");
    *((int *)data) = this->getLatency();
    break;

  case AUDIOGETDELAY:
    VERBOSE_DEBUG("AUDIOGETDELAY\n");
    *((int *)data) = this->getDelay(this->getTime());
    break;

  case AUDIOCHUNKSIZE:
    VERBOSE_DEBUG("AUDIOGETDELAY\n");
    *((int *)data) = this->chunksize;
    break;

  case AUDIOGETOPORT:
  case AUDIOSETOPORT: {
    OSIterator *i =
      OSCollectionIterator::withCollection(engine->defaultAudioControls);
    r = ENOTTY;
    if (!i) break;

    for (unsigned n = 0; n < engine->defaultAudioControls->getCount(); n++) {
      IOAudioControl *c = CAST(IOAudioControl, i->getNextObject());
      if (c && c->getType() == kIOAudioControlTypeSelector &&
	  c->getSubType() == kIOAudioSelectorControlSubTypeOutput &&
	  c->getUsage() == kIOAudioControlUsageOutput) {
	if (cmd == AUDIOGETOPORT) {
	  *((int *)data) = c->getIntValue();
	  r = 0;
	} else if (cmd == AUDIOSETOPORT) {
	  IOAudioSelectorControl *s =
	    CAST(IOAudioSelectorControl, c);
	  int v = *((int *)data);
      
	  if (!s->valueExists(v)) { r = EINVAL; break; }

	  s->setValue(v);
	  *((int *)data) = s->getIntValue();
	  if (*((int *)data) == v) {
	    r = 0;
	  } else {
	    r = EAGAIN;
	  }
	}
      }
    }

    break;
  }

  case AUDIOGETVOL:
  case AUDIOSETVOL: {
    audio_volume_t *volume = (audio_volume_t *)data;
    unsigned nfound = 0;
    unsigned result = 0;
    OSIterator *i =
      OSCollectionIterator::withCollection(engine->defaultAudioControls);

    if (!i) { r = ENOTTY; break; }

    for (unsigned n = 0; n < engine->defaultAudioControls->getCount(); n++) {
      IOAudioControl *c = CAST(IOAudioControl, i->getNextObject());

      if (c && c->getType() == kIOAudioControlTypeLevel &&
	  c->getSubType() == kIOAudioLevelControlSubTypeVolume &&
	  c->getUsage() == kIOAudioControlUsageOutput && 
	  check_channel_id(c, volume->id)) {
	nfound++;
	IOAudioLevelControl *l = CAST(IOAudioLevelControl, c);

	DEBUG("%s:%u/%u\tid=%u\treq=%u\n",
	      l->getName(), nfound, n, (int)l->getChannelID(), volume->id);

	if (cmd == AUDIOSETVOL) {
	  int v = volume->value * l->getMaxValue() / 
	    AUDIO_VOL_MAX + l->getMinValue();
	  int ovalue = 
	    AUDIO_VOL_MAX * (l->getIntValue() - l->getMinValue())
	    / l->getMaxValue();

	  if (v == l->getIntValue())
	    v = volume->value > ovalue ? v + 1 : 
	      volume->value < ovalue ? v - 1 : v;
	    
	  l->setValue(v);
	  DEBUG("value=%u corrected=%u\n", volume->value, v);
	} else {
	  result += AUDIO_VOL_MAX * (l->getIntValue() - l->getMinValue())
	    / l->getMaxValue();
	  DEBUG("value=%u result=%u\n", (int)l->getIntValue(), result);
	}
      } else if (c && c->getType() == kIOAudioControlTypeToggle &&
	  c->getSubType() == kIOAudioToggleControlSubTypeMute &&
	  c->getUsage() == kIOAudioControlUsageOutput && 
	  check_channel_id(c, volume->id)) {
	r = 0;

	if (cmd == AUDIOSETVOL)
	  volume->muted = c->getIntValue();
	else if (cmd == AUDIOGETVOL)
	  if (!c->setValue(volume->muted)) r = EINVAL;

	DEBUG("Setting muted to %u\n", volume->muted);

      }
    }

    if (!nfound) {
      r = EINVAL;
    } else {
      volume->value = result / nfound;
      r = 0;
    }
    
    break;
  }

  case FIONBIO:
    VERBOSE_DEBUG("FIONBIO\n");
    this->blocking = *(int *)data;
    break;

  case FIOSETOWN:
    VERBOSE_DEBUG("FIOSETOWN\n");
    owner = *(int *)data;
    break;

  case FIOGETOWN: {
    VERBOSE_DEBUG("FIOGETOWN\n");
    *(int *)data = this->getOwner();
    break;
  }

  default:
    IOLog("%s recieved an nrecognised ioctl (%c,%u)\n", this->getName(),
	  (int)IOCGROUP(cmd), (int)cmd & 0xf);
    r = ENOTTY;
  }

#if NICE_LOCKS
  engine->unlockAllStreams();
#endif

  return r;
}

const char *ODAudioBSDClient::getDeviceBase() const
{
  return "dsp";
}

uid_t ODAudioBSDClient::getOwner() const
{
  /* declared and maintained by the VFS subsystem */
  extern uid_t console_user;

  return owner == -1 ? console_user : owner;
}
