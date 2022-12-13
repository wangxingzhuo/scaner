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

static enum AVPixelFormat hw_pix_fmt;
static enum AVPixelFormat get_hw_format(AVCodecContext *ctx, const enum AVPixelFormat *pix_fmts)
{
    const enum AVPixelFormat *p;

    for (p = pix_fmts; *p != -1; p++)
    {
        if (*p == hw_pix_fmt)
            return *p;
    }

    fprintf(stderr, "Failed to get HW surface format.\n");
    return AV_PIX_FMT_NONE;
}

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

static void printHW()
{
    enum AVHWDeviceType type;
    printf("hw device type:");
    while ((type = av_hwdevice_iterate_types(type)) != AV_HWDEVICE_TYPE_NONE)
        printf(" %s", av_hwdevice_get_type_name(type));
    printf("\n");
}

static void printMeta(AVFormatContext *input_ctx)
{
    const AVDictionaryEntry *tag = NULL;
    while ((tag = av_dict_get(input_ctx->metadata, "", tag, AV_DICT_IGNORE_SUFFIX)))
        printf("%s=%s\n", tag->key, tag->value);
}

struct sound
{
    AVFormatContext *fmt_ctx;
    AVCodec *codec;
    AVCodecContext *codec_ctx;
    AVCodecHWConfig *hw_cfg;
};

/* open the input file */
int open_media(struct sound *media, const char *filename)
{
    memset(media, 0, sizeof(struct sound));
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

int find_audio_decoder(struct sound *media)
{
    AVFormatContext *input_ctx = media->fmt_ctx;
    AVStream *av_stream = NULL;
    AVCodecContext *coder_ctx = NULL;
    AVPacket *pkt;

    /* find the video stream information */
    const int stream_nb = av_find_best_stream(input_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, &media->codec, 0);
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

int find_hw_device(struct sound *media)
{

    AVCodecHWConfig *cfg = NULL;
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

void free_media(struct sound *media)
{
    if (media->codec_ctx)
        avcodec_free_context(&media->codec_ctx);

    avformat_close_input(&media->fmt_ctx);
    memset(media, 0, sizeof(struct sound));
}

static int get_format_from_sample_fmt(const char **fmt,
                                      enum AVSampleFormat sample_fmt)
{
    int i;
    struct sample_fmt_entry
    {
        enum AVSampleFormat sample_fmt;
        const char *fmt_be, *fmt_le;
    } sample_fmt_entries[] = {
        {AV_SAMPLE_FMT_U8, "u8", "u8"},
        {AV_SAMPLE_FMT_S16, "s16be", "s16le"},
        {AV_SAMPLE_FMT_S32, "s32be", "s32le"},
        {AV_SAMPLE_FMT_FLT, "f32be", "f32le"},
        {AV_SAMPLE_FMT_DBL, "f64be", "f64le"},
    };
    *fmt = NULL;

    for (i = 0; i < FF_ARRAY_ELEMS(sample_fmt_entries); i++)
    {
        struct sample_fmt_entry *entry = &sample_fmt_entries[i];
        if (sample_fmt == entry->sample_fmt)
        {
            *fmt = AV_NE(entry->fmt_be, entry->fmt_le);
            return 0;
        }
    }

    fprintf(stderr,
            "sample format %s is not supported as output format\n",
            av_get_sample_fmt_name(sample_fmt));
    return -1;
}

static void decode(AVCodecContext *dec_ctx, AVPacket *pkt, AVFrame *frame, FILE *outfile)
{
    int i, ch;
    int ret, data_size;

    /* send the packet with the compressed data to the decoder */
    ret = avcodec_send_packet(dec_ctx, pkt);
    if (ret < 0)
    {
        fprintf(stderr, "Error submitting the packet to the decoder\n");
        exit(1);
    }

    /* read all the output frames (in general there may be any number of them */
    while (ret >= 0)
    {
        ret = avcodec_receive_frame(dec_ctx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0)
        {
            fprintf(stderr, "Error during decoding\n");
            exit(1);
        }
        data_size = av_get_bytes_per_sample(dec_ctx->sample_fmt);
        if (data_size < 0)
        {
            /* This should not occur, checking just for paranoia */
            fprintf(stderr, "Failed to calculate data size\n");
            exit(1);
        }
        for (i = 0; i < frame->nb_samples; i++)
            for (ch = 0; ch < dec_ctx->ch_layout.nb_channels; ch++)
                fwrite(frame->data[ch] + data_size * i, 1, data_size, outfile);
    }
}

// int main(int argc, char *argv[])
// {
//     AVFormatContext *input_ctx = NULL;
//     int stream_nb;
//     AVStream *av_stream = NULL;
//     AVCodec *decoder = NULL;
//     AVCodecContext *decoder_ctx = NULL;

//     enum AVHWDeviceType type;
//     int i, ret;
//     struct sound media;
//     uint8_t inbuf[AUDIO_INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];

//     if (argc < 2)
//     {
//         printHW();
//         return 0;
//     }

//     /* OPEN A SOUND FILE */
//     if (open_media(&media, argv[1]))
//     {
//         fprintf(stderr, "Cannot open sound file\n");
//         return 1;
//     }

//     printMeta(input_ctx);

//     if (find_audio_decoder(&media))
//     {
//         fprintf(stderr, "find decoder failed\n");
//         return 1;
//     }

//     if (!find_hw_device(&media))
//         fprintf(stderr, "Decoder %s does not support device type.\n", media.codec->name);
//     else
//     {
//         type = media.hw_cfg->device_type;
//         hw_pix_fmt = media.hw_cfg->pix_fmt;
//         media.codec_ctx->get_format = get_hw_format;
//         fprintf(stderr, "hw: %s\n", av_hwdevice_get_type_name(type));
//     }

//     /* decode until eof */
//     AVPacket *pkt = av_packet_alloc();
//     AVFrame *decoded_frame = av_frame_alloc();
//     AVCodecParserContext *parser = av_parser_init(media.codec->id);
//     int fd = open(argv[1], O_RDONLY);

//     if (!parser || !decoded_frame || !pkt || -1 == fd)
//     {
//         fprintf(stderr, "Could not allocate audio frame\n");
//         exit(1);
//     }

//     uint8_t *data = inbuf;
//     off_t off = 0;
//     int64_t written = 0;
//     ssize_t data_size;
//     data_size = pread(fd, inbuf, AUDIO_INBUF_SIZE, off);
//     off += data_size;

//     while (data_size > 0)
//     {
//         ret = av_parser_parse2(parser, media.codec_ctx, &pkt->data, &pkt->size, data, data_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
//         if (ret < 0)
//         {
//             fprintf(stderr, "Error while parsing\n");
//             exit(1);
//         }
//         data += ret;
//         data_size -= ret;

//         if (pkt->size)
//             decode(media.codec_ctx, pkt, decoded_frame, outfile);

//         if (data_size < AUDIO_REFILL_THRESH)
//         {
//             memmove(inbuf, data, data_size);
//             data = inbuf;
//             ssize_t len = pread(fd, data + data_size, AUDIO_INBUF_SIZE - data_size, off);
//             if (len > 0)
//             {
//                 off += len;
//                 data_size += len;
//             }
//         }
//     }

//     /* flush the decoder */
//     pkt->data = NULL;
//     pkt->size = 0;
//     decode(media.codec_ctx, pkt, decoded_frame, outfile);

//     /* print output pcm infomations, because there have no metadata of pcm */
//     enum AVSampleFormat sfmt = media.codec_ctx->sample_fmt;

//     if (av_sample_fmt_is_planar(sfmt))
//     {
//         const char *packed = av_get_sample_fmt_name(sfmt);
//         printf("Warning: the sample format the decoder produced is planar "
//                "(%s). This example will output the first channel only.\n",
//                packed ? packed : "?");
//         sfmt = av_get_packed_sample_fmt(sfmt);
//     }

//     n_channels = c->ch_layout.nb_channels;
//     if ((ret = get_format_from_sample_fmt(&fmt, sfmt)) < 0)
//         goto end;

//     printf("Play the output audio file with the command:\n"
//            "ffplay -f %s -ac %d -ar %d %s\n",
//            fmt, n_channels, c->sample_rate,
//            outfilename);

//     QAudioFormat format;
//     format.setFrequency(codecContext->sample_rate);
//     format.setChannels(2);
//     format.setSampleSize(16);
//     format.setCodec("audio/pcm");
//     format.setByteOrder(QAudioFormat::LittleEndian);
//     format.setSampleType(QAudioFormat::SignedInt);

//     QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());

//     if (!info.isFormatSupported(format))
//     {
//         qWarning() << "raw audio format not supported by backend, cannot play audio.";
//         format = info.nearestFormat(format);
//     }

//     audio = new QAudioOutput(format, this);

//     connect(audio, SIGNAL(stateChanged(QAudio::State)), SLOT(stateChanged(QAudio::State)));

//     if (!buffer.open(QBuffer::ReadWrite))
//         qWarning() << "Couldnt open Buffer";

//     qDebug() << "buffer.size()=" << buffer.size();

//     audio->start(&buffer);

//     free_media(&media);
//     return 0;
// }
