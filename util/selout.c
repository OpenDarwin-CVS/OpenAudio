#include "../include/audioio.h"

#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

int main(int argc, char *argv[])
{
  int32_t fd, r, i;

  if (argc != 3) {
    fprintf(stderr, "Usage: %s <device> {hdpn,ispk,espk}\n", argv[0]);
    exit(1);
  };

  fd = open(argv[1], O_WRONLY);
  
  if (fd < 0) {
    perror("open failed");
    exit(1);
  };

  fprintf(stderr, "%s opened...\n", argv[1]);

  r = ioctl(fd, AUDIOGETOPORT, &i);

  if (r) {
    perror("ioctl failed");
    exit(1);
  };

  fprintf(stderr, "port is %.4s...\n", &i);

  if (!strcmp("hdpn", argv[2]))
    i = kIOAudioSelectorControlSelectionValueHeadphones;
  else if (!strcmp("espk", argv[2]))
    i = kIOAudioSelectorControlSelectionValueInternalSpeaker;
  else if (!strcmp("ispk", argv[2]))
    i = kIOAudioSelectorControlSelectionValueExternalSpeaker;
  else
    fprintf(stderr, "%s is not one of hdpn, ispk or espk!\n", argv[2]);

  fprintf(stderr, "trying to set port to %.4s...\n", &i);

  r = ioctl(fd, AUDIOSETOPORT, &i);

  if (r) {
    perror("ioctl failed");
    exit(1);
  };

  fprintf(stderr, "port is %.4s...\n", &i);

}