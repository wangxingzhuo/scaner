#include "pic_meta.h"

static int _print_pic_meta(void *dst, const size_t len, struct pic_meta_t *app)
{
    char point_unit[][4] = { "px", "dpi", "ppc" };
    char jpg_colors[][6] = { "-", "grey", "2", "YCbCr", "CMYK" };
    char png_colors[][6] = { "grey", "1", "RGB", "PLTE", "greya", "5", "RGBA" };
    register string6_t *colors;

    colors = (string6_t *)(memcmp(app->format, "png", 3) ? jpg_colors : png_colors);

    return snprintf(
        dst,
        len,
        "{format: \"%s\", dpp: \"%.0f * %.0f%s\", depth: %d, color_type: \"%s\", width: %d, height: %d}\n",
        app->format,
        app->x_dpu,
        app->y_dpu,
        point_unit[app->unit],
        app->bit_depth,
        colors[app->color_type].str,
        app->width,
        app->height
    );
}

int print_pic_meta(const char *filename)
{
    unsigned char buf[BUFSIZ];
    struct pic_meta_t info;
    memset(&info, 0, sizeof(struct pic_meta_t));

    int fd = open(filename, O_RDONLY);
    if (-1 == fd) return 1;

    if (8 == pread(fd, buf, 8, 0))
        switch (check_pic_type(buf))
        {
        case PIC_JPG:
            load_jpg_meta(&info, fd);
            break;
        case PIC_PNG:
            load_png_meta(&info, fd);
            break;
        default:
        }

    close(fd);

    int len = _print_pic_meta(buf, BUFSIZ, &info);
    write(STDOUT_FILENO, buf, len);
    return 0;
}
