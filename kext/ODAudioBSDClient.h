/* -*- c++ -*-
 *
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

#ifndef _ODAUDIOBSDCLIENT_H_
#define _ODAUDIOBSDCLIENT_H_

#include "ODBSDClient.h"

#include <IOKit/audio/IOAudioTypes.h>
#include <IOKit/audio/IOAudioEngine.h>
#include <IOKit/audio/IOAudioStream.h>

#define kODAudioBSDClientTypeKey "ODAudioBSDClientType"

class ODAudioBSDClient : public ODBSDClient
{
  OSDeclareAbstractStructors(ODAudioBSDClient);

 private:
  static int ninitialised;
  virtual const char *statusString();

  virtual uint64_t bytesToNanos(uint64_t bytes);
  virtual uint64_t bytesToFrames(uint64_t bytes);
  virtual uint64_t nanosToBytes(uint64_t nanos);
  virtual uint64_t framesToBytes(uint64_t frames);

  virtual AbsoluteTime getTime();

 protected:

  /* STATE-RELATED FIELDS */

  bool is_open;
  UInt32 loopcount;
  AbsoluteTime next_call;

  /* IOAUDIOFAMILY-RELATED FIELDS */

  IOAudioEngine *engine;
  IOAudioStream *outputstream;

  virtual const char *getDeviceBase() const;

 public:

  virtual int open(int flags, int devtype, struct proc *pp);
  virtual int close(int flags, int mode, struct proc *pp);
  virtual int write(struct uio *uio, int ioflag);
  virtual int ioctl(u_long cmd, caddr_t data, int fflag, struct proc *p);

  virtual bool start(IOService *provider);

};

#endif
