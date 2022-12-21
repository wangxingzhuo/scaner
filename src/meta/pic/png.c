#include "pic_meta.h"

/** RFC 2083 - Portable Network Graphics */

/*
| 颜色类型 | 位深 | |
| --- | --- | ---|
| 0   | 1,2,4,8,16 | 每个像素是一个灰度样本 |
| 2   | 8,16       | 每个像素是一个 R、G、B 三元组 |
| 3   | 1,2,4,8    | 每个像素是一个调色板索引；必须出现 PLTE 块 |
| 4   | 8,16       | 每个像素是一个灰度样本，后跟一个 alpha 样本 |
| 6   | 8,16       | 每个像素是一个 R、G、B 三元组，后跟一个 alpha 样本 |
*/

// struct png_block_t {
//     uint32_t data_length;
//     // 第一个字节的第5位（32）0（大写）=关键，1（小写）=辅助
//     // 第二个字节的第5位（32）0（大写）=公有，1（小写）=私有
//     // 第四个字节的第5位（32）0（大写）=复制不安全，1（小写）=复制安全
//     uint32_t type;
//     char data[];
//     // [ISO-3309] 或 [ITU-V42] 定义
//     // uint32_t crc;
// };

// 有效的 PNG 图片必须包含 IHDR块、一个或多个 IDAT 块和一个 IEND 块。

enum color_t {
    PALETTE_USED = 1,
    COLOR_USED = 2,
    ALPHA_CHANNEL_USED = 4
};

// 调色板，需在第一个IDAT块之前
// struct PLTE {
//     char[3][] rgb;
// };

int load_png_meta(struct pic_meta_t *info, int fd)
{
    struct png_t png;
    const struct IHDR *meta;
    u_int32_t len, block_info[2];
    struct phys_t phys_block;
    off_t off;

    memset(info, 0, sizeof(struct pic_meta_t));

    off = pread(fd, &png, sizeof(struct png_t), 0);
    if (off < 1)
        return -1;

    if (!check_png(&png))
        return -1;

    meta = &(png.ihdr);
    memcpy(info->format, "png", 4);
    info->bit_depth = meta->bit_depth;
    info->color_type = meta->color_type;
    info->width = hton32(meta->width);
    info->height = hton32(meta->height);

    while (0 < pread(fd, block_info, 8, off))
    {
        off += 8;
        len = hton32(block_info[0]);
        // pHYs
        if (0x73594870 == block_info[1])
        {
            pread(fd, &phys_block, 9, off);
            info->unit = 0;
            // meter
            if (1 == phys_block.unit)
            {
                info->unit = phys_block.unit;
                info->x_dpu = hton32(phys_block.x_ppu) * 0.0254;
                info->y_dpu = hton32(phys_block.y_ppu) * 0.0254;
            }
        }
        off += len + 4;
    }
    return 0;
}
