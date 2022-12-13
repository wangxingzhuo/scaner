#include "stdafx.h"

int put_meta(const char *inputFileName)
{
    int ret;
    struct sound_t media;

    ret = open_media(&media, inputFileName);
    if (ret)
        return ret;

    ret = find_audio_decoder(&media);
    if (ret)
        return ret;

    print_meta(&media);
    free_media(&media);
    return 0;
}
