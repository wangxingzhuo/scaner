#ifndef _STDAFX_H_
#define _STDAFX_H_
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/pixdesc.h>
#include <libavutil/hwcontext.h>
#include <libavutil/opt.h>
#include <libavutil/avassert.h>
#include <libavutil/imgutils.h>

#define AUDIO_INBUF_SIZE 20480
#define AUDIO_REFILL_THRESH 4096

struct duration_t
{
    int minute;
    int second;
};

struct meta_t
{
    char fmt[8];
    double sample_rate;
    int bit_depth;
    int bit_rate;
    struct duration_t duration;
    int nb_channels;
    char channel_layout;
};

struct sound_t
{
    AVFormatContext *fmt_ctx;
    const struct AVCodec *codec;
    AVCodecContext *codec_ctx;
    const AVCodecHWConfig *hw_cfg;
};

/* open the input file */
int open_media(struct sound_t *media, const char *filename);

int find_audio_decoder(struct sound_t *media);

int find_hw_device(struct sound_t *media);

void free_media(struct sound_t *media);

void print_meta(struct sound_t *media);
#endif
