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

#include "ds_util.h"
#include "ODAudioBSDClient.h"

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
#include <IOKit/IOWorkLoop.h>

#include <IOKit/audio/IOAudioDefines.h>
#include <IOKit/audio/IOAudioEngine.h>
#include <IOKit/audio/IOAudioStream.h>

/* Used to detect whether we've already been initialized */
static int ds_installed = 0;
static ODAudioBSDClient **clients;
static int nclients;

kern_return_t ds_start(kmod_info_t * ki, void * d)
{
  DEBUG_FUNCTION();

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
    /* count IOAudioEngines */
    while (i->getNextObject()) nclients++;
    i->reset();

    clients = IONew(ODAudioBSDClient *, nclients);

    for (int n = 0; n < nclients; n++) {
      IOAudioEngine *e = CAST(IOAudioEngine, i->getNextObject());
      clients[n] = ODAudioBSDClient::withAudioEngine(e, n);

      if (!clients[n]) {
	IOLog("Failed to initialise client #%u!\n", n);
	nclients--;
      }
    }
  }

  if (dict) dict->release();
  if (i) i->release();
  
  return KERN_SUCCESS;
}

kern_return_t ds_stop(kmod_info_t * ki, void * d)
{
  DEBUG_FUNCTION();

  for (int n = 0; n < nclients; n++)
    if (clients[n]) clients[n]->release();

  IODelete(clients, ODAudioBSDClient *, nclients);

  ds_installed = 0;
  nclients = 0;

  return KERN_SUCCESS;
}

int ds_open(dev_t dev, int flags, int devtype, struct proc *pp)
{
  DEBUG_FUNCTION();

  return clients[minor(dev)]->open() ? 0 : EBUSY;
}

int ds_close(dev_t dev, int flags, int mode, struct proc *pp)
{
  DEBUG_FUNCTION();

  clients[minor(dev)]->close();

  return 0;
}

int ds_write(dev_t dev, struct uio *uio, int ioflag)
{
  DEBUG_FUNCTION();

  if (uio->uio_resid < 256) return EINVAL;

  return clients[minor(dev)]->write(uio);
}
