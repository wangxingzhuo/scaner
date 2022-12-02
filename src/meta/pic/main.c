#include "pic_meta.h"

typedef union string_6
{
    char str[6];
} string6_t;

static void out_hex(int fd, char *buf, size_t len)
{
    read(fd, buf, len);
    len -= 4;
    for (int i = 0; i < len; i++)
    {
        printf("%2X ", (unsigned char)buf[i]);
    }
    printf("\n");
}

static void print_pic_meta(struct pic_meta_t *app)
{
    char point_unit[][4] = { "px", "dpi", "ppc" };
    char jpg_colors[][6] = { "-", "grey", "2", "YCbCr", "CMYK" };
    char png_colors[][6] = { "grey", "1", "RGB", "PLTE", "greya", "5", "RGBA" };
    register string6_t *colors;

    colors = (string6_t *)(memcmp(app->format, "png", 3) ? jpg_colors : png_colors);

    printf(
        "{format: \"%s\", dpp: \"%.0f * %.0f%s\", depth %d, color_type: \"%s\", weight: %d, height: %d}\n",
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

static int print_meta(const char *filename)
{
    unsigned char buf[BUFSIZ];
    struct pic_meta_t info;

    int fd = open(filename, O_RDONLY);
    if (-1 == fd) return 1;
    size_t len = read(fd, buf, BUFSIZ);

    switch (check_pic_type(buf))
    {
    case PIC_JPG:
        load_jpg_meta(&info, fd);
        break;
    case PIC_PNG:
        load_png_meta(&info, fd);
        break;
    default:
        printf("%s is not a jpg picture!\n", filename);
        return 1;
    }
    close(fd);
    print_pic_meta(&info);
    return 0;
}

static int print_png(const char* filename)
{
    struct png_t png;
    struct IHDR *meta;
    // struct pic_meta_t info;
    int fd, len;
    u_int32_t block_info[2];
    char type_name[5];
    char buf[BUFSIZ];

    fd = open(filename, O_RDONLY);
    len = read(fd, &png, sizeof(struct png_t));

    if (!check_png(&png))
    {
        close(fd);
        printf("not png");
        return 1;
    }

    // load_png_meta(&info, (void *)&png, len);
    // print_pic_meta(&info);

    len = 0;
    type_name[4] = 0;

    while (1)
    {
        if (read(fd, block_info, 8) < 1) break;
        memcpy(type_name, (char *)(block_info + 1), 4);
        len = hton32(block_info[0]);
        printf("type: %s, len: %d\n", type_name, len);
        len += 4;
        if (len < BUFSIZ) out_hex(fd, buf, len);
        else lseek(fd, len, SEEK_CUR);
    }

    close(fd);
    return 0;
}

int main(const int argc, const char *argv[])
{
    if (3 == argc)
        return print_png(argv[2]);
    return print_meta(argv[1]);
}
