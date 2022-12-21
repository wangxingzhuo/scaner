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

int load_audio_meta(const char *inputFileName)
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

    // printf("path=%s\n", inputFileName);
    printf("#EXTINF:%ld,\n", fmt_ctx->duration / AV_TIME_BASE);
    printf("sample_rate:%dHz,\n", coder_ctx->sample_rate);
    printf("channels:%d,\n", coder_ctx->ch_layout.nb_channels);
    printf("bit_rate:%ldkbps,\n", fmt_ctx->bit_rate / 1000);
    while ((tag = av_dict_get(fmt_ctx->metadata, "", tag, AV_DICT_IGNORE_SUFFIX)))
        printf("%s:%s,\n", tag->key, tag->value);
    printf("\n");
    ret = 0;

End:
    if (NULL != coder_ctx)
        avcodec_free_context(&coder_ctx);
    avformat_close_input(&fmt_ctx);
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