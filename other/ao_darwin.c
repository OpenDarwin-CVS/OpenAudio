/*
 * Darwin audio output module for MPlayer. This is the reference
 * implementation of an audio output using OpenAudio directly.
 *
 * Copyright (c) 2004, Dan Villiom Podlaski Christiansen
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 */

#include <OpenAudio/audioio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "config.h"

#include "mp_msg.h"
#include "bswap.h"
#include "afmt.h"
#include "audio_out.h"
#include "audio_out_internal.h"

/* for some reason, we need the sync value returned by the driver to
 * be multiplied by 1.5
 * TODO: More tuning of DELAY_MULTIPLIER */
 #define DELAY_MULTIPLIER 1.5

#define ao_msg(lvl, ...)			\
  mp_msg(MSGT_AO, lvl, "AO: [darwin] " __VA_ARGS__)

#define ao_dbg(...)				\
  ao_msg(MSGL_DBG2, __VA_ARGS__)

#define ao_v(...)				\
  ao_msg(MSGL_V, __VA_ARGS__)

#define ao_err(...)				\
  ao_msg(MSGL_ERR, __VA_ARGS__)

#define ao_warn(...)				\
  ao_msg(MSGL_WARN, __VA_ARGS__)

#define ao_f()				\
  ao_dbg("--> %s:%s()\n", __FILE__, __func__);

static ao_info_t info = 
{
	"OpenAudio output for Darwin",
	"darwin",
	"Dan Villiom P. Christiansen",
	""
};

LIBAO_EXTERN(darwin);

static int ao_fd = -1;
static int delay = 0;

static int control(int cmd, void *arg)
{
  ao_f();

  switch (cmd) {
  case AOCONTROL_SET_VOLUME: {
    ao_control_vol_t *aovol = arg;
    audio_volume_t sysvol;
    
    sysvol.value = aovol->left / 100.0 * AUDIO_VOL_MAX;
    sysvol.id = kIOAudioControlChannelIDDefaultLeft;

    if(ioctl(ao_fd, AUDIOSETVOL, &sysvol)) {
      ao_err("AUDIOSETVOL right failed: %s\n", strerror(errno));
      return CONTROL_ERROR;
    }

    sysvol.value = aovol->right / 100.0 * AUDIO_VOL_MAX;
    sysvol.id = kIOAudioControlChannelIDDefaultRight;

    ao_dbg("sleft=%f,%i\n", aovol->left, sysvol.value);

    if(ioctl(ao_fd, AUDIOSETVOL, &sysvol)) {
      ao_err("AUDIOSETVOL left failed: %s\n", strerror(errno));
      return CONTROL_ERROR;
    }

    ao_dbg("sright=%f,%i\n", aovol->right, sysvol.value);

    return CONTROL_OK;
  }

  case AOCONTROL_GET_VOLUME: {
    ao_control_vol_t *aovol = arg;
    audio_volume_t sysvol;
    
    sysvol.id = kIOAudioControlChannelIDDefaultLeft;

    if(ioctl(ao_fd, AUDIOGETVOL, &sysvol)) {
      ao_err("AUDIOGETVOL right failed: %s\n", strerror(errno));
      return CONTROL_ERROR;
    }

    aovol->left = sysvol.value * 100.0 / AUDIO_VOL_MAX;
    ao_dbg("gleft=%f,%i\n", aovol->left, sysvol.value);

    sysvol.id = kIOAudioControlChannelIDDefaultRight;

    if(ioctl(ao_fd, AUDIOGETVOL, &sysvol)) {
      ao_err("AUDIOGETVOL left failed: %s\n", strerror(errno));
      return CONTROL_ERROR;
    }

    aovol->right = sysvol.value * 100.0 / AUDIO_VOL_MAX;
    ao_dbg("gright=%f,%i\n", aovol->right, sysvol.value);

    return CONTROL_OK;
  }

  case AOCONTROL_SET_DEVICE:
  case AOCONTROL_GET_DEVICE:
  case AOCONTROL_QUERY_FORMAT:
  case AOCONTROL_SET_PLUGIN_DRIVER:
  case AOCONTROL_SET_PLUGIN_LIST:
  default:
    return CONTROL_UNKNOWN;
  }
}

static int init(int rate,int channels,int format,int flags)
{
  IOAudioStreamFormat sfmt;
  int f;

  ao_f();

  if (!ao_subdevice) ao_subdevice = "/dev/dsp";
  if (ao_fd < 0) ao_fd = open(ao_subdevice, O_WRONLY | O_NONBLOCK);

  if (ao_fd < 0) {
    ao_err("Failed to open audio device %s: %s\n",
	   ao_subdevice, strerror(errno));
    uninit();
    return false;
  }

  /* get audio format from driver */
  if(ioctl(ao_fd, AUDIOGETOFMT, &sfmt)) {
    ao_err("AUDIOGETOFMT failed: %s\n", strerror(errno));
    uninit();
    return false;
  }

  /* copy fields */
  sfmt.fSampleFormat = kIOAudioStreamSampleFormatLinearPCM;

  /* get chunk size from driver */
  if(ioctl(ao_fd, AUDIOCHUNKSIZE, &f)) {
    ao_err("AUDIOCHUNKSIZE failed: %s\n", strerror(errno));
    uninit();
    return false;
  }

  ao_data.outburst = f;

  /* hard code 16-bit native endian stereo */
  sfmt.fNumChannels = ao_data.channels = channels = 2;
  ao_data.format = AFMT_S16_NE;
  ao_data.samplerate = 44100;
  sfmt.fBitDepth = sfmt.fBitWidth = 16;
  ao_data.bps = ao_data.samplerate * ao_data.channels * sfmt.fBitDepth / 8;

  /* send audio format to driver */
  if(ioctl(ao_fd, AUDIOSETOFMT, &sfmt)) {
    ao_err("AUDIOSETOFMT failed: %s\n", strerror(errno));
    uninit();
    return false;
  }

  return true;
}

static void uninit()
{
  ao_f();

  if (ao_fd >= 0)
    close(ao_fd);

  ao_fd = -1;
}

static void reset()
{
  ao_f();
}

static void audio_pause()
{
  ao_f();
}

static void audio_resume()
{
  ao_f();
}

static int play(void* data,int len,int flags)
{
  int r = write(ao_fd, data, len);

  ao_f();

  if (r < 0) {
    ao_msg(errno == EAGAIN ? MSGL_DBG2 : MSGL_V,
	   "write failed: %s\n", strerror(errno));
    return 0;
  }

  return r;
}

static int get_space()
{
  int d = 0;

  ao_f();

  if(ioctl(ao_fd, AUDIOGETDELAY, &d))
    ao_warn("AUDIOGETDELAY failed: %s\n", strerror(errno));
  else
    ao_v("delay: %5lli\tspace: %5i\n", d, d ? 0 : ao_data.outburst);

  return d ? 0 : ao_data.outburst;
}

static float get_delay()
{
  int d = 0;

  ao_f();

  if(ioctl(ao_fd, AUDIOLATENCY, &d))
    ao_warn("AUDIOLATENCY failed: %s\n", strerror(errno));
  else
    ao_v("delay=%f\n", d / 1e9);

  return (d / 1e9) * DELAY_MULTIPLIER;
}
