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

int main(int argv, char *argc[])
{
  char *inbuf = malloc(BUFSIZE);
  char *outbuf = malloc(BUFSIZE);
  int bytes_read, flags;

  if (!inbuf || !outbuf)
    return -1;

  signal(SIGINT, sigint_handler);

  flags = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

  do {
    bytes_read = read(STDIN_FILENO, inbuf, BUFSIZE);

    if (bytes_read > 0) {
      if (bytes_read % 2 == 0) {
	swab(inbuf, outbuf, bytes_read);
      } else {
	swab(inbuf, outbuf, bytes_read - 1);
	outbuf[bytes_read - 1] = inbuf[bytes_read - 1];
      }

      write(STDOUT_FILENO, outbuf, bytes_read);
    }
  } while (running && bytes_read != 0);

  return 0;
}
