#include "pic_meta.h"

static void load_app0(struct pic_meta_t *info, const struct app0_t *app)
{
    printf("%X\n", info);
    memcpy(info->format, app->JFIF, 5);
    info->x_dpu = hton16(app->x_dpu);
    info->y_dpu = hton16(app->y_dpu);
    // cm
    if (2 == app->unit)
    {
        info->x_dpu *= 2.54;
        info->y_dpu *= 2.54;
    }
    info->unit = !!app->unit;
}

static void load_sof0(struct pic_meta_t *info, const struct SOF0_t *p)
{
    info->bit_depth = p->bit_depth;
    info->color_type = p->color_type;
    info->width = hton16(p->width);
    info->height = hton16(p->height);
}

int load_jpg_meta(struct pic_meta_t *info, int fd)
{
    register off_t off;
    register int flag;
    register size_t seg_len;
    struct jpg_seg_t jpg_seg;
    struct app0_t app;
    struct SOF0_t sof;
    unsigned char buf[6];

    // load header
    if (pread(fd, buf, 6, 0) < 1)
        return -1;

    if (!check_jpg(buf))
        return -1;

    // copy first segment
    *(int32_t *)&jpg_seg = *(int32_t *)(buf + 2);
    off = 2;
    do
    {
        off += 4;
        seg_len = hton16(jpg_seg.len);

        switch (jpg_seg.type)
        {
            // app0
            case 0xE0:
                if (pread(fd, &app, seg_len, off) < 1) return -1;
                load_app0(info, &app);
                flag += 1;
                break;
            // sof0
            case 0xC0:
                if (pread(fd, &sof, seg_len, off) < 1) return -1;
                load_sof0(info, &sof);
                flag += 1;
                break;
            default:
                break;
        }
        off += seg_len < 3 ? 0 : (seg_len - 2);
    }
    while (flag < 2 && 4 == pread(fd, &jpg_seg, 4, off));
    return 0;
}
