#include "../include/audioio.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <limits.h>

#include <fcntl.h>
#include <sys/ioctl.h>

static int fd = -1;

static bool get_volume(unsigned *lv, unsigned *rv)
{
  int r;
  audio_volume_t v;

  v.id = kIOAudioControlChannelIDDefaultLeft;
  r = ioctl(fd, AUDIOGETVOL, &v);

  if (r) return false;
  else *lv = v.value;

  v.id = kIOAudioControlChannelIDDefaultRight;
  r = ioctl(fd, AUDIOGETVOL, &v);

  if (r) return false;
  else *rv = v.value;

  return true;
}

static bool set_volume(unsigned lv, unsigned rv)
{
  int r;
  audio_volume_t v;

  v.id = kIOAudioControlChannelIDDefaultLeft;
  v.value = lv;
  v.muted = false;
  r = ioctl(fd, AUDIOSETVOL, &v);

  if (r) return false;

  v.id = kIOAudioControlChannelIDDefaultRight;
  v.value = rv;
  r = ioctl(fd, AUDIOSETVOL, &v);

  if (r) return false;

  return true;
}

static void p(unsigned a, unsigned b)
{
  fprintf(stderr, "%u\t%u\n", a, b);
  fprintf(stderr, "%g\t%g\n", (double)a / AUDIO_VOL_MAX, (double)b / AUDIO_VOL_MAX);
}

int main(int argc, char *argv[])
{
  unsigned lv, rv;

  if (argc != 4) {
    fprintf(stderr, "Usage: %s <device> lvolume rvolume\n", argv[0]);
    exit(1);
  };

  fprintf(stderr, "opening %s...\n", argv[1]);

  fd = open(argv[1], O_WRONLY);
  
  if (fd < 0) {
    perror("open failed");
    exit(1);
  };

  fprintf(stderr, "getting volume...\n");

  if (!get_volume(&lv, &rv)) {
    perror("AUDIOGETVOL failed");
    exit(1);
  } else {
    p(lv, rv);
  }

  fprintf(stderr, "setting volume...\n");

  lv = atof(argv[2]) * AUDIO_VOL_MAX;
  rv = atof(argv[3]) * AUDIO_VOL_MAX;

  if (!set_volume(lv, rv)) {
    perror("AUDIOSETVOL failed");
    exit(1);
  } else {
    p(lv, rv);
  }

  fprintf(stderr, "getting volume...\n");

  if (!get_volume(&lv, &rv)) {
    perror("AUDIOGETVOL failed");
    exit(1);
  } else {
    p(lv, rv);
  }

}
