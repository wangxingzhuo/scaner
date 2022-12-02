#include "pic_meta.h"

int check_jpg(const void *buf)
{
    return !memcmp((const char *)buf, "\xFF\xD8\xFF", 3);
}

int check_png(const void *buf)
{
    return !memcmp((const char *)buf, "\x89\x50\x4e\x47\x0d\x0a\x1a\x0a", 8);
}

int check_pic_type(const void *buf)
{
    if (check_jpg(buf)) return PIC_JPG;
    if (check_png(buf)) return PIC_PNG;
    return 0;
}

u_int32_t hton32(const u_int32_t n)
{
    char *chp = (char *)&n;
    chp[0] ^= chp[3];
    chp[3] ^= chp[0];
    chp[0] ^= chp[3];

    chp[1] ^= chp[2];
    chp[2] ^= chp[1];
    chp[1] ^= chp[2];
    return n;
}

u_int16_t hton16(u_int16_t n)
{
    char *p = (char *)&n;
    p[0] ^= p[1];
    p[1] ^= p[0];
    p[0] ^= p[1];
    return n;
}
