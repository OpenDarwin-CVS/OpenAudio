/*
 * Small utility for byteswapping data
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

#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

#define BUFSIZE 8192

static volatile bool running = true;

void sigint_handler(int signal)
{
  running = false;
}

int main(int argc, char *argv[])
{
  char *inbuf = malloc(BUFSIZE);
  char *outbuf = malloc(BUFSIZE);
  int in = STDIN_FILENO, out = STDOUT_FILENO;
  int bytes_read, flags;

  if (!inbuf || !outbuf)
    return -1;

  fprintf(stderr, "Usage: %s [infile [outfile]]\n", argv[0]);
  fprintf(stderr, "Swapping from %s to %s\n",
	  argc > 1 ? argv[1] : "stdin", argc > 2 ? argv[2] : "stdout");

  if (argc > 1)
    in = open(argv[1], O_RDONLY);
  if (argc > 2)
    out = open(argv[2], O_WRONLY);

  signal(SIGINT, sigint_handler);

  flags = fcntl(in, F_GETFL, 0);
  fcntl(in, F_SETFL, flags | O_NONBLOCK);

  do {
    bytes_read = read(in, inbuf, BUFSIZE);

    if (bytes_read > 0) {
      if (bytes_read % 2 == 0) {
	swab(inbuf, outbuf, bytes_read);
      } else {
	swab(inbuf, outbuf, bytes_read - 1);
	outbuf[bytes_read - 1] = inbuf[bytes_read - 1];
      }

      write(out, outbuf, bytes_read);
    }
  } while (running && bytes_read != 0);

  return 0;
}
