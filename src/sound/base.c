#include "stdafx.h"

static AVCodecContext *new_codec_cxt(const AVCodec *codec)
{
    AVCodecContext *coder_ctx;

    if (!(coder_ctx = avcodec_alloc_context3(codec)))
        return NULL;

    if (avcodec_open2(coder_ctx, codec, NULL) < 0)
    {
        avcodec_free_context(&coder_ctx);
        return NULL;
    }

    return coder_ctx;
}

static struct duration_t format_time(int64_t nano)
{
    struct duration_t ret;
    int64_t duration = nano * 10 / AV_TIME_BASE;
    ret.minute = duration / 600;
    duration %= 600;
    ret.second = duration / 10 + (4 < duration % 10);
    return ret;
}

static void load_meta(struct meta_t *meta, struct sound_t *media)
{
    AVFormatContext *fmt_ctx = media->fmt_ctx;
    AVCodecContext *codec_ctx = media->codec_ctx;

    static const char channel_layout_dict[] = "UNCA";
    meta->channel_layout = channel_layout_dict[codec_ctx->ch_layout.order];
    meta->nb_channels = codec_ctx->ch_layout.nb_channels;
    meta->sample_rate = codec_ctx->sample_rate / 1000.0;
    meta->duration = format_time(fmt_ctx->duration);
    meta->bit_depth = codec_ctx->bits_per_raw_sample;
    meta->bit_rate = fmt_ctx->bit_rate / 1000;
    strncpy(meta->fmt, fmt_ctx->iformat->name, 7);
}

void print_meta(struct sound_t *media)
{
    AVFormatContext *fmt_ctx = media->fmt_ctx;
    const AVDictionaryEntry *tag = NULL;
    struct meta_t meta;
    load_meta(&meta, media);

    printf(
        "fmt=%s channel_layout=%c_%d sample_rate=%gKHz bit_depth=%d br=%dKbps duration=%d:%02d\n",
        meta.fmt,
        meta.channel_layout,
        meta.nb_channels,
        meta.sample_rate,
        meta.bit_depth,
        meta.bit_rate,
        meta.duration.minute,
        meta.duration.second);

    while ((tag = av_dict_get(fmt_ctx->metadata, "", tag, AV_DICT_IGNORE_SUFFIX)))
        printf("%s=%s\n", tag->key, tag->value);
}

/* open the input file */
int open_media(struct sound_t *media, const char *filename)
{
    memset(media, 0, sizeof(struct sound_t));
    AVFormatContext *ctx = NULL;

    if (avformat_open_input(&ctx, filename, NULL, NULL) != 0)
        return -1;

    if (avformat_find_stream_info(ctx, NULL) < 0)
    {
        avformat_close_input(&ctx);
        return -1;
    }

    media->fmt_ctx = ctx;
    return 0;
}

int find_audio_decoder(struct sound_t *media)
{
    AVFormatContext *input_ctx = media->fmt_ctx;
    AVStream *av_stream = NULL;
    AVCodecContext *coder_ctx = NULL;
    // AVPacket *pkt;

    /* find the audio stream information */
    const int stream_nb = av_find_best_stream(input_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, &(media->codec), 0);
    if (stream_nb < 0)
    {
        fprintf(stderr, "Cannot find a audio stream in the input file\n");
        return -1;
    }
    av_stream = input_ctx->streams[stream_nb];

    if (!(coder_ctx = new_codec_cxt(media->codec)))
        return AVERROR(ENOMEM);

    if (avcodec_parameters_to_context(coder_ctx, av_stream->codecpar) < 0)
        return -1;

    media->codec_ctx = coder_ctx;
    return 0;
}

int find_hw_device(struct sound_t *media)
{
    const AVCodecHWConfig *cfg = NULL;
    for (int i = 0;; i++)
    {
        const AVCodecHWConfig *config = avcodec_get_hw_config(media->codec, i);
        if (!config)
            break;

        if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX)
        {
            cfg = config;
            break;
        }
    }
    media->hw_cfg = cfg;
    return !!cfg;
}

void free_media(struct sound_t *media)
{
    if (media->codec_ctx)
        avcodec_free_context(&media->codec_ctx);

    avformat_close_input(&media->fmt_ctx);
    memset(media, 0, sizeof(struct sound_t));
}
