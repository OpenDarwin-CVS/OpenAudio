/*
 * Utility definitions for the OpenDarwin audio subsystem
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

#define CAST(type, instance)			\
  (type *)(instance)->metaCast(#type)

/* HACK: workaround a typo in a kernel header */
#define devfs_link devfs_make_link

#define UMASK 0666

#if 0
#define VERBOSE_DEBUG(...) DEBUG(__VA_ARGS__)
#else
#define VERBOSE_DEBUG(...) (void)0
#endif

#ifndef NDEBUG
#define MACH_ASSERT
#define DEBUG(...) \
  IOLog(__VA_ARGS__)
#if 0
#define DEBUG_FUNCTION()			\
  DEBUG("-> %s\n", __PRETTY_FUNCTION__);
#else
#define DEBUG_FUNCTION() (void)0
#endif
#define DEBUG_WAIT(...)				\
  do {						\
    IOLog(__VA_ARGS__);				\
    IOSleep(1000);				\
  } while (0);
#else
#define DEBUG(...) (void)0
#define DEBUG_FUNCTION() (void)0
#define DEBUG_WAIT() (void)0
#endif

#define EXPAND4(...) __VA_ARGS__ __VA_ARGS__ __VA_ARGS__ __VA_ARGS__

#define EXPAND16(...) EXPAND4(__VA_ARGS__) EXPAND4(__VA_ARGS__) \
       EXPAND4(__VA_ARGS__) EXPAND4(__VA_ARGS__)

#define EXPAND256(...) EXPAND16(__VA_ARGS__) EXPAND16(__VA_ARGS__) \
       EXPAND16(__VA_ARGS__) EXPAND16(__VA_ARGS__)

#define ARRAY4(...) { EXPAND4(__VA_ARGS__,) }
#define ARRAY16(...) { EXPAND16(__VA_ARGS__,) }
#define ARRAY256(...) { EXPAND256(__VA_ARGS__,) }
