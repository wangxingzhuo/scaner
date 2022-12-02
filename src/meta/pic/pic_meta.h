#ifndef _PIC_META_H_
#define _PIC_META_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#define PIC_JPG 1
#define PIC_PNG 2

// 第一大块，元数据
struct IHDR {
    u_int32_t data_length;
    // 第一个字节的第5位（32）0（大写）=关键，1（小写）=辅助
    // 第二个字节的第5位（32）0（大写）=公有，1（小写）=私有
    // 第四个字节的第5位（32）0（大写）=复制不安全，1（小写）=复制安全
    u_int32_t type;
    u_int32_t width;
    u_int32_t height;
    // 位深
    unsigned char bit_depth;
    // color_t
    unsigned char color_type;
    // 0 (deflate/inflate 32K滑动窗口压缩)
    unsigned char compression_method;
    // 5种过滤
    unsigned char filter_method;
    // 0（无隔行）或 1（Adam7 隔行）
    unsigned char interlace_method;
    // [ISO-3309] 或 [ITU-V42] 定义
    u_int32_t crc;
} __attribute__((packed));

struct png_t {
    u_int64_t sign;
    struct IHDR ihdr;
    // struct png_block [] blocks;
} __attribute__((packed));

struct phys_t {
    u_int32_t x_ppu;
    u_int32_t y_ppu;
    unsigned char unit;
} __attribute__((packed));

// 多个 IDAT 块必须连续出现
struct IDAT {};

// 最后一个大块，没有内容
struct IEND {};

struct jpg_seg_t
{
    unsigned char ff;
    unsigned char type;
    unsigned short len;
    // unsigned char data[];
} __attribute__((packed));

struct app0_t
{
    char JFIF[5];
    unsigned char m_version;
    unsigned char s_version;
    unsigned char unit; // 0＝无单位；1＝点数/英 寸；2＝点数/厘米
    unsigned short x_dpu; // 水平方向的密度
    unsigned short y_dpu; // 水平方向的密度
    unsigned short thum_x_dpu; // 水平方向的密度
    unsigned short thum_y_dpu; // 水平方向的密度
    char thum_rgb[];
} __attribute__((packed));

struct SOF0_t
{
    unsigned char bit_depth;
    unsigned short height;
    unsigned short width;
    unsigned char color_type; // 1＝灰度图，3＝YCbCr/YIQ 彩色图，4＝CMYK 彩色图
} __attribute__((packed));

struct pic_meta_t
{
    char format[5];
    unsigned char color_type; // 1＝灰度图，3＝YCbCr/YIQ 彩色图，4＝CMYK 彩色图
    unsigned char bit_depth;
    unsigned char unit; // 0＝无单位；1＝点数/英 寸；2＝点数/厘米
    float x_dpu; // 水平方向的密度
    float y_dpu; // 水平方向的密度
    u_int32_t width;
    u_int32_t height;
} __attribute__((packed));


int check_jpg(const void *buf);

int check_png(const void *buf);

int check_pic_type(const void *buf);

u_int32_t hton32(const u_int32_t n);

u_int16_t hton16(u_int16_t n);

int load_jpg_meta(struct pic_meta_t *info, int fd);

int load_png_meta(struct pic_meta_t *info, int fd);

#endif
