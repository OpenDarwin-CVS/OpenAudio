/*
 * Small utility for mixing two PCM streams
 * Implements a home grown, low quality algorithm
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

#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <math.h>
#include <limits.h>
#include <architecture/byte_order.h>
#include <c.h>

#define BUFSIZE 8192

static inline int imin(int a, int b)
{
  return a < b ? a : b;
}

int main(int argc, char *argv[])
{
  short *inbuf1 = malloc(BUFSIZE), *inbuf2 = malloc(BUFSIZE);
  short *outbuf = malloc(BUFSIZE);
  int in1 = -1, in2 = -1, out = STDOUT_FILENO, bytes_read;

  if (!inbuf1 || !inbuf2 || !outbuf)
    return -1;

  if (argc < 3) {
    fprintf(stderr, "Usage: %s infile infile [outfile]\n", argv[0]);
    return -1;
  }

  in1 = open(argv[1], O_RDONLY);
  in2 = open(argv[2], O_RDONLY);
  if (argc > 3)
    out = open(argv[3], O_WRONLY | O_CREAT, 666);

  if (in1 < 0 || in2 < 0 || out < 0)
    return -1;

  fprintf(stderr, "Mixing %s and %s to %s\n",
	  argv[1], argv[2], argc > 3 ? argv[3] : "stdout");

  double scale[2] = {2.0, 2.0};
  const int nch = 2;

  do {
    bytes_read =
      imin(read(in1, inbuf1, BUFSIZE), read(in2, inbuf2, BUFSIZE));
    
    if (bytes_read > 0) {
      for (int i = 0; i < bytes_read / 2; i++) {
#if 1
	float a = (inbuf1[i] - (float)SHRT_MIN) / USHRT_MAX;
	float b = (inbuf2[i] - (float)SHRT_MIN) / USHRT_MAX;
	const float factor = 1.000001;

	if (a * b * scale[i%nch] > 1.0) {
	  scale[i%nch] = 1 / (a * b);
	} else if (scale[i%nch] > 2.0) {
	  scale[i%nch] = 2.0;
	} else {
	  scale[i%nch] *= factor;
	}

	outbuf[i] = a * b * scale[i%nch] * USHRT_MAX + SHRT_MIN;
#else
	double a = inbuf1[i], b = inbuf2[i], r;
	const double factor = 1.0001;
	
	r = (a + b) / scale[i%nch];

	if (r > SHRT_MAX || r < SHRT_MIN) {
	  fprintf(stderr, "%5i: r=%7.0f s=%.3f\t", i, r, scale[i%nch]);
	  scale[i%nch] = 1.0 + SHRT_MAX / fabs(r);
	  r = (a + b) / scale[i%nch];
	  fprintf(stderr, "r=%7.0f s=%1.3f\t%s\n", r, scale[i%nch],
		  r > SHRT_MAX || r < SHRT_MIN ? "CLIP" : "");
	} else if (scale[i%nch] > 1.0) {
	  scale[i%nch] /= factor;
	  r = (a + b) / scale[i%nch];
	} else {
	  r = a + b;
	}

	outbuf[i] = r;
#endif
      }

      write(out, outbuf, bytes_read);
    }
  } while (bytes_read != 0);

  return 0;
}
