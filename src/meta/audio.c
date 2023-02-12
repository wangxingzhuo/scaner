#include <stdio.h>
#include <libavformat/avformat.h>
#include <libavutil/dict.h>
#include <libavcodec/avcodec.h>

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


/**`
 * @return {
 *   cover: string; // https://img.webp
 *   lrc: string; // lrc txt
 *   url: string; // /sha1.flac
 *
 *   title: string; // My all
 *   artist: string; // 浜崎あゆみ
 *   album: string; // GUILTY
 *
 *   channles: number; // 2
 *   smple_rate: number; // 44100(Hz)
 *   bit_rate: number; // 1077(kbps)
 * }
*/
int load_audio_meta(const char *inputFileName, char *buf, size_t size)
{
    AVFormatContext *fmt_ctx = NULL;
    AVDictionaryEntry *tag = NULL;
    const AVCodec *codec = NULL;
    AVCodecContext *coder_ctx = NULL;
    int ret = avformat_open_input(&fmt_ctx, inputFileName, NULL, NULL);
    if (ret) return ret;
    ret = -1;

    if (avformat_find_stream_info(fmt_ctx, NULL) < 0) goto End;

    const int stream_nb = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, &codec, 0);
    if (stream_nb < 0) goto End;

    AVStream *av_stream = fmt_ctx->streams[stream_nb];

    if (!(coder_ctx = new_codec_cxt(codec)))
    {
        ret = AVERROR(ENOMEM);
        goto End;
    }

    if (avcodec_parameters_to_context(coder_ctx, av_stream->codecpar) < 0) goto End;

    size_t len = 0;
    // printf("path=%s\n", inputFileName);
    len += snprintf(buf, size - len, "#EXTINF:%ld,\n", fmt_ctx->duration / AV_TIME_BASE);
    len += snprintf(buf + len, size - len, "sample_rate:%dHz,\n", coder_ctx->sample_rate);
    len += snprintf(buf + len, size - len, "channels:%d,\n", coder_ctx->ch_layout.nb_channels);
    len += snprintf(buf + len, size - len, "bit_rate:%ldkbps,\n", fmt_ctx->bit_rate / 1000);

    while ((tag = av_dict_get(fmt_ctx->metadata, "", tag, AV_DICT_IGNORE_SUFFIX)))
    {
        len += snprintf(buf + len, size - len, "%s:%s,\n", tag->key, tag->value);
    }

    len += snprintf(buf + len, size - len, "\n");
    ret = len;
    // printf("%s", buf);

End:
    if (NULL != coder_ctx)
        avcodec_free_context(&coder_ctx);
    // printf("%ld\n", &fmt_ctx);
    avformat_close_input(&fmt_ctx);
    // printf("%d\n", 321321);
    return ret;
}

/*
int main(int argc, char **argv)
{
    AVFormatContext *fmt_ctx = NULL;
    AVDictionaryEntry *tag = NULL;
    int ret;
    char *inputFileName = argv[1];

    if (argc != 2) {
        printf("usage: %s <input_file>\n"
               "example program to demonstrate the use of the libavformat metadata API.\n"
               "\n", argv[0]);
        return 1;
    }

    return load_audio_meta(argv[1]);
}
//*/