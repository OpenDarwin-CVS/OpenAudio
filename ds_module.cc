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

#include "ds_module.h"

/* HACK: workaround a typo in a kernel header */
#if DARWIN_MAJOR < 7
#define devfs_link devfs_make_link
#endif

extern "C" {
#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/conf.h>
#include <sys/uio.h>

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <miscfs/devfs/devfs.h>
}

#include <libkern/c++/OSDictionary.h>
#include <IOKit/IOLib.h>

#include <IOKit/audio/IOAudioDefines.h>
#include <IOKit/audio/IOAudioEngine.h>
#include <IOKit/audio/IOAudioStream.h>

#define CAST(type, instance)			\
  (type *)(instance)->metaCast(#type)

/* A struct describing which functions will get invoked for certain
 * actions.
 */
static struct cdevsw ds_cdevsw = {
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

/* Used to detect whether we've already been initialized */
static int ds_installed = 0;
static int opened = 0;

static IOAudioEngine **engines;
static void **device_nodes;
static int nengines;

static const char *status_to_string(const IOAudioEngineState s)
{
  switch (s) {
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

extern "C" kern_return_t ds_start(kmod_info_t * ki, void * d)
{
  if( ds_installed ) {
    /* already registered, so don't register again */
    printf("%s: already loaded!\n", __FUNCTION__);
    return KERN_FAILURE;
  }

  ds_installed = 1;

  OSDictionary *dict = IOService::serviceMatching(kIOAudioEngineClassName);
  OSIterator *i = IOService::getMatchingServices(dict);
	 
  if (!i) {
    /* no audio engines available, don't start */
    dict->release();
    printf("%s: no audio engines preset!\n", __FUNCTION__);
    return KERN_FAILURE;
  } else {
    /* create the character device */
    int ret = cdevsw_add(DS_MAJOR, &ds_cdevsw);

    if(ret < 0) {
      dict->release();
      i->release();
      printf("%s: failed to allocate a major number!\n", __FUNCTION__);
      return KERN_FAILURE;
    }

    while (i->getNextObject()) nengines++;
    i->reset();

    engines = IONew(IOAudioEngine *, nengines);
    device_nodes = IONew(void *, nengines);

    for (int n = 0; n < nengines; n++) {
      engines[n] = CAST(IOAudioEngine, i->getNextObject());
      device_nodes[n] = devfs_make_node(makedev(ret, n), DEVFS_CHAR,
					UID_ROOT, GID_WHEEL, 
					0600, "dsp%x", n);
    
      printf("Found a %s IOAudioEngine %s!\n",
	     status_to_string(engines[n]->getState()), engines[n]->getName());

      if (engines[n]->outputStreams) {
	printf("\tEngine has %u output streams!\n",
	       engines[n]->outputStreams->getCount());
	IOAudioStream *outputstream =
	  CAST(IOAudioStream, engines[n]->outputStreams->getCount() > 0 ?
	       engines[n]->outputStreams->getObject(0) : NULL);

	if (outputstream)
	  printf("\tOutput stream has %u IO functions\n",
		 (unsigned)outputstream->numIOFunctions);
      } else {
	printf("\tEngine has no output streams!\n");
      }

      if (engines[n]->inputStreams)
	printf("\tEngine has %u input streams!\n", engines[n]->inputStreams->getCount());
      else
	printf("\tEngine has no input streams!\n");

    }

    devfs_make_link(device_nodes[0], "dsp");
  }
       
  if (dict) dict->release();
  if (i) i->release();
  
  return KERN_SUCCESS;
}

kern_return_t ds_stop(kmod_info_t * ki, void * d)
{
  int ret;
  ret = cdevsw_remove(DS_MAJOR, &ds_cdevsw);
  if( ret < 0 ) {
    printf("%s: failed to remove character device!\n", __FUNCTION__);
    return KERN_FAILURE;
  }

  for (int n = 0; n < nengines; n++)
    devfs_remove(device_nodes[n]);

  IODelete(engines, IOAudioEngine *, nengines);
  IODelete(device_nodes, void *, nengines);

  ds_installed = 0;
  nengines = 0;

  return KERN_SUCCESS;
}

int ds_open(dev_t dev, int flags, int devtype, struct proc *pp)
{
  if (!opened) {
    opened = 1;
  } else {
    printf("%s entered: device already opened\n", __FUNCTION__);
    return EACCES;
  }

  engines[minor(dev)]->startAudioEngine();

  printf("%s entered: %s is now %s\n", __FUNCTION__, engines[minor(dev)]->getName(),
	 status_to_string(engines[minor(dev)]->getState()));
  
  return 0;
}

int ds_close(dev_t dev, int flags, int mode, struct proc *pp)
{
  printf("%s entered\n", __FUNCTION__);

  engines[minor(dev)]->stopAudioEngine();

  opened = 0;

  return 0;
}

int ds_write(dev_t dev, struct uio *uio, int ioflag)
{
  int n = minor(dev);
  IOAudioEngine *engine = engines[n];

  /* debug output */
  switch (uio->uio_segflg) {
  case UIO_USERSPACE:
  case UIO_USERISPACE:
    break;
  case UIO_SYSSPACE:
  case UIO_PHYS_USERSPACE:
    return EFAULT;
  }

  if (engine->outputStreams && engine->outputStreams->getCount() > 0) {
    IOAudioStream *outputstream = 
      CAST(IOAudioStream, engine->outputStreams->getObject(0));
    int bytes = MIN((long)outputstream->getSampleBufferSize(), (long)uio->uio_resid);
    char buf[bytes];
    int r = uiomove((caddr_t)buf, bytes, uio);

    IOReturn ior = engine->mixOutputSamples((char *)buf,
					    outputstream->getMixBuffer(),
					    0, 1,
					    &outputstream->format,
					    outputstream);

    if (ior != kIOReturnSuccess) {
      printf("%s: MIX FAILED\n", __FUNCTION__);
    } else {
      printf("%u written to a %s %s: %u\n", bytes, 
	     status_to_string(engine->getState()),
	     engine->getName(), r);
    }

    return r;
  } else {
    return EIO;
  }
}
