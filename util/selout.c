#include "../include/audioio.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <fcntl.h>
#include <sys/ioctl.h>

int main(int argc, char *argv[])
{
  int32_t fd, r, i;

  if (argc != 3) {
    fprintf(stderr, "Usage: %s <device> {hdpn,ispk,espk}\n", argv[0]);
    exit(1);
  };

  fd = open(argv[1], O_RDONLY);
  
  if (fd < 0) {
    perror("open failed");
    exit(1);
  };

  fprintf(stderr, "%s opened...\n", argv[1]);

  r = ioctl(fd, AUDIOGETOPORT, &i);

  if (r) {
    perror("AUDIOGETOPORT failed");
    exit(1);
  };

  fprintf(stderr, "port is %.4s...\n", (const char *)&i);

  if (!strcmp("hdpn", argv[2]))
    i = kIOAudioSelectorControlSelectionValueHeadphones;
  else if (!strcmp("ispk", argv[2]))
    i = kIOAudioSelectorControlSelectionValueInternalSpeaker;
  else if (!strcmp("espk", argv[2]))
    i = kIOAudioSelectorControlSelectionValueExternalSpeaker;
  else
    fprintf(stderr, "%s is not one of hdpn, ispk or espk!\n", argv[2]);

  fprintf(stderr, "trying to set port to %.4s...\n", (const char *)&i);

  r = ioctl(fd, AUDIOSETOPORT, &i);

  if (r) {
    perror("AUDIOSETOPORT failed");
    exit(1);
  };

  fprintf(stderr, "port is %.4s...\n", (const char *)&i);

}
