#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#include <fcntl.h>
#include <sys/ioctl.h>

#include "../include/audioio.h"

int main(int argc, char *argv[])
{
  audio_volume_t v;
  int fd;

  if (argc != 2) {
    fprintf(stderr, "Usage: %s <device>\n", argv[0]);
    return 1;
  };

  fd = open(argv[1], O_WRONLY);
  
  if (fd < 0) {
    perror("open failed");
    return 1;
  };

  v.id = kIOAudioControlChannelIDAll;
  v.value = 0;
  v.muted = true;

  if (ioctl(fd, AUDIOSETVOL, &v)) {
    perror("AUDIOSETVOL faled");
    return 1;
  }

  return 0;
}
