/*
 * IOCTL keys for the OpenDarwin Audio BSD Client
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

#ifndef	_AUDIO_IOCTL_H_
#define	_AUDIO_IOCTL_H_

#ifndef KERNEL
#include <inttypes.h>
#include <stdbool.h>
#endif

#include <sys/types.h>
#include <sys/ioccom.h>
#include <IOKit/audio/IOAudioTypes.h>

/* TYPES */

typedef struct audio_volume {
  /* kIOAudioControlChannelID */
  uint32_t value;
  uint8_t id;
  bool muted : 1;
  int padding : 31;
} audio_volume_t;

/* GENERAL AUDIO */

/* perform an audible beep */
#ifdef UNIMPLEMENTED
#define AUDIOBEEP _IO('A', 1)
#endif

/* flush audio device */
#ifdef UNIMPLEMENTED
#define AUDIOFLUSH _IO('A', 2)
#endif

/* get latency in nanoseconds */
#define AUDIOLATENCY _IOR('A', 3, int32_t)

/* AUDIO FORMAT */

/* get output format of the audio device */
#define AUDIOGETOFMT _IOR('A', 11, IOAudioStreamFormat)

/* set output format of the audio device */
#define AUDIOSETOFMT _IOW('A', 11, IOAudioStreamFormat)

/* get input format of the audio device */
#ifdef UNIMPLEMENTED
#define AUDIOGETIFMT _IOR('A', 12, IOAudioStreamFormat)
#endif

/* set input format of the audio device */
#ifdef UNIMPLEMENTED
#define AUDIOSETIFMT _IOW('A', 12, IOAudioStreamFormat)
#endif

/* VOLUME CONTROL */

/* send channelid - get volume */
#define AUDIOGETVOL _IOWR('A', 22, audio_volume_t)

/* set volume */
#define AUDIOSETVOL _IOW('A', 22, audio_volume_t)

/* get output port
   one of IOAudioTypes::kIOAudioSelectorControlSelectionValue* */
#define AUDIOGETOPORT _IOR('A', 24, int32_t)

/* select output port
   one of IOAudioTypes::kIOAudioSelectorControlSelectionValue* */
#define AUDIOSETOPORT _IOWR('A', 24, int32_t)

#endif /* !_AUDIO_IOCTL_H_ */
