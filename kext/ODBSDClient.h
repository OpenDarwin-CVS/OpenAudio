/* -*- c++ -*-
 *
 * OpenDarwin IOKit BSD Client
 * An abstract IOService with an entry in /dev
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

#ifndef _ODBSDCLIENT_H_
#define _ODBSDCLIENT_H_

#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/conf.h>

#include <IOKit/IOService.h>

#define ODBSDClient org_opendarwin_ODBSDClient

class ODBSDClient : public IOService
{
  OSDeclareAbstractStructors(ODBSDClient);

private:
  
  int minor;
  void *node;

protected:

  /* this return the base name of the /dev/<name>N entries */
  virtual const char *getDeviceBase() const = 0;
  virtual uid_t getOwner() const = 0;

public:

  /* the device operations to be implemented by the subclass */
  virtual int open(int flags, int devtype, struct proc *pp);
  virtual int close(int flags, int mode, struct proc *pp);
  virtual int read(struct uio *uio, int ioflag);
  virtual int write(struct uio *uio, int ioflag);
  virtual int ioctl(u_long cmd, caddr_t data, int fflag, struct proc *p);
  virtual int select(int which, void * wql, struct proc *p);
  virtual int mmap();
  virtual int getc();
  virtual int putc(char c);

  virtual bool start(IOService *provider);
  virtual void stop(IOService *provider);

};

#endif /* _ODAUDIOBSDCLIENT_H_ */
