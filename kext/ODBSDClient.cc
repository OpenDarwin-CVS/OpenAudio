/*
 * OpenDarwin IOKit BSD Client
 * An abstract IOService with an entry in /dev
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

#include "ODBSDClient.h"

#include "audio_util.h"

#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/systm.h>
#include <sys/conf.h>
#include <sys/uio.h>
#include <sys/errno.h>

#include <miscfs/devfs/devfs.h>

#include <IOKit/IOLib.h>

#include "../include/defines.h"

OSDefineMetaClassAndAbstractStructors(ODBSDClient, IOService);

/* major device number shared across all clients */
static int major = -1;

/* these map a minor device number to a client and it's node */
static ODBSDClient *clients[256] = ARRAY256(NULL);

static int bsd_open(dev_t dev, int flags, int devtype, struct proc *pp);
static int bsd_close(dev_t dev, int flags, int mode, struct proc *pp);
static int bsd_read(dev_t dev, struct uio *uio, int ioflag);
static int bsd_write(dev_t dev, struct uio *uio, int ioflag);
static int bsd_ioctl(dev_t dev, u_long cmd, caddr_t data,
		     int fflag, struct proc *p);
static int bsd_select(dev_t dev, int which, void * wql, struct proc *p);
static int bsd_getc(dev_t dev);
static int bsd_putc(dev_t dev, char c);

static struct cdevsw chardev = {
  bsd_open,        /* open */
  bsd_close,       /* close */
  bsd_read,        /* read */
  bsd_write,       /* write */
  bsd_ioctl,       /* ioctl */
  eno_stop,        /* stop */
  eno_reset,       /* reset */
  NULL,            /* tty's */
  bsd_select,      /* select */
  eno_mmap,        /* mmap */
  eno_strat,       /* strategy */
  bsd_getc,        /* getc */
  bsd_putc,        /* putc */
  0                /* type */
};

bool ODBSDClient::start(IOService *provider)
{
  DEBUG_FUNCTION();

  const char *device_base = this->getDeviceBase();

  if (!device_base || strlen(device_base) > 255 || !provider) {
    IOLog("%s: Bad device base or bad provider.", this->getName());
    return false;
  }

  if (major == -1)
    major = cdevsw_add(major, &chardev);

  this->minor = -1;

  for (unsigned i = 0; i <= 256; i++) if (!clients[i]) {
    this->minor = i;
    break;
  }
      
  if (this->minor == -1) {
    IOLog("%s: Cannot allocate instance, maximum reached.", this->getName());
    return false;
  }

  char name[256] = ARRAY256(0);

  if (this->minor == 0)
    strncpy(name, device_base, sizeof(name) - 1);
  else
    snprintf(name, sizeof(name) - 1, "%s%x", device_base, this->minor + 1);

  clients[this->minor] = this;
  clients[this->minor]->node = devfs_make_node(makedev(major, this->minor),
					       DEVFS_CHAR,
					       this->getOwner(), GID_WHEEL, 
					       UMASK, name, this->minor);
  if (!clients[this->minor]->node) return false;
  
#if 0
  /* try to create a link in /dev/<base> */
  devfs_link(clients[this->minor]->node, (char *)device_base);
#endif

  /* set properties */
  this->setProperty(kODBSDClientMajorNumberKey, major, sizeof(major)*8);
  this->setProperty(kODBSDClientMinorNumberKey, minor, sizeof(minor)*8);
  this->setProperty(kODBSDClientDeviceKey, name);
  this->setProperty(kODBSDClientBaseNameKey, device_base);

  return true;
}

void ODBSDClient::stop(IOService *provider)
{
  DEBUG_FUNCTION();

  unsigned nused = 0;

  /* count the amount of used device entries */
  for (unsigned i = 0; i < 256; i++) if (clients[i]) nused++;

  if (nused == 1) {
    cdevsw_remove(major, &chardev);
    major = -1;
  }

  devfs_remove(clients[this->minor]->node);
  clients[this->minor] = NULL;
  nused--;
}

static int bsd_open(dev_t dev, int flags, int devtype, struct proc *pp)
{
  DEBUG_FUNCTION();

  if (clients[minor(dev)])
    return clients[minor(dev)]->open(flags, devtype, pp);
  else
    return EBADF;
}

int ODBSDClient::open(int flags, int devtype, struct proc *pp)
{
  DEBUG_FUNCTION();

  return ENODEV;
}

int ODBSDClient::close(int flags, int mode, struct proc *pp)
{
  DEBUG_FUNCTION();

  return ENODEV;
}

int ODBSDClient::read(struct uio *uio, int ioflag)
{
  DEBUG_FUNCTION();

  return ENODEV;
}

int ODBSDClient::write(struct uio *uio, int ioflag)
{
  DEBUG_FUNCTION();

  return ENODEV;
}

int ODBSDClient::ioctl(u_long cmd, caddr_t data, int fflag, struct proc *p)
{
  DEBUG_FUNCTION();

  return ENODEV;
}

int ODBSDClient::select(int which, void * wql, struct proc *p)
{
  DEBUG_FUNCTION();

  return ENODEV;
}

int ODBSDClient::mmap()
{
  DEBUG_FUNCTION();

  return ENODEV;
}

int ODBSDClient::getc()
{
  DEBUG_FUNCTION();

  return ENODEV;
}

int ODBSDClient::putc(char c)
{
  DEBUG_FUNCTION();

  return ENODEV;
}

static int bsd_close(dev_t dev, int flags, int mode, struct proc *pp)
{
  DEBUG_FUNCTION();

  if (clients[minor(dev)])
    return clients[minor(dev)]->close(flags, mode, pp);
  else
    return EBADF;
}

static int bsd_read(dev_t dev, struct uio *uio, int ioflag)
{
  DEBUG_FUNCTION();

  if (clients[minor(dev)])
    return clients[minor(dev)]->read(uio, ioflag);
  else
    return EBADF;
}

static int bsd_write(dev_t dev, struct uio *uio, int ioflag)
{
  DEBUG_FUNCTION();

  if (clients[minor(dev)])
    return clients[minor(dev)]->write(uio, ioflag);
  else
    return EBADF;
}

static int bsd_ioctl(dev_t dev, u_long cmd, caddr_t data,
		     int fflag, struct proc *p)
{
  DEBUG_FUNCTION();

  if (clients[minor(dev)])
    return clients[minor(dev)]->ioctl(cmd, data, fflag, p);
  else
    return EBADF;
}

static int bsd_select(dev_t dev, int which, void * wql, struct proc *p) {
  DEBUG_FUNCTION();

  if (clients[minor(dev)])
    return clients[minor(dev)]->select(which, wql, p);
  else
    return EBADF;
}

static int bsd_getc(dev_t dev) {
  DEBUG_FUNCTION();

  if (clients[minor(dev)])
    return clients[minor(dev)]->getc();
  else
    return EBADF;
}

static int bsd_putc(dev_t dev, char c) {
  DEBUG_FUNCTION();

  if (clients[minor(dev)])
    return clients[minor(dev)]->putc(c);
  else
    return EBADF;
}
