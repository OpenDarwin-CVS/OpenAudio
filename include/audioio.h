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

#include <sys/ioccom.h>
#include <IOKit/audio/IOAudioTypes.h>

/* general audio ioctl's. */

#ifdef UNIMPLEMENTED
/* perform an audible beep */
#define AUDIOBEEP _IO('A', 1)

/* flush audio device */
#define AUDIOFLUSH _IO('A', 2)

/* get latency in nanoseconds */
#define AUDIOLATENCY _IOR('A', 3, long long)

/* audio format ioctl's. */

/* get output format of the audio device */
#define AUDIOGETOFMT _IOR('A', 11, IOAudioStreamFormat)

/* set output format of the audio device */
#define AUDIOSETOFMT _IOW('A', 12, IOAudioStreamFormat)

/* get input format of the audio device */
#define AUDIOGETIFMT _IOR('A', 13, IOAudioStreamFormat)

/* set input format of the audio device */
#define AUDIOSETIFMT _IOW('A', 14, IOAudioStreamFormat)

#endif /* UNIMPLEMENTED */

#endif /* !_AUDIO_IOCTL_H_ */
