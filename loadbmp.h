#ifndef LOADBMP_H__
#define LOADBMP_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

struct BitmapFileHeader
{
    uint8_t id[2];
    uint8_t len[4];
    uint8_t reserved1[2];
    uint8_t reserved2[2];
    uint8_t offset[4];
} BFH;

struct BITMAPINFOHEADER
{
    uint8_t size_header[4];
    uint8_t bitmap_width[4];
    uint8_t bitmap_height[4];
    uint8_t n_color_planes[2];
    uint8_t bpp[2];
    uint8_t compression[4];
    uint8_t image_size[4];
    uint8_t horizontal_res[4];
    uint8_t vertical_res[4];
    uint8_t n_color_palette[4];
    uint8_t n_imp_colors[4];
} BIH;

struct COLORTABLE
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} *CT;


struct PIXELARRAY
{
    uint8_t *pixels;
} PA;

unsigned int invert_hex(uint8_t *hex, int len)
{
    unsigned int value = 0x00000000;
    for (int i = 0; i < len; i++)
    {
        value |= (hex[i] << i*8);
    }
    return value;
}

uint8_t get_pixel(int x, int y, int w, int h)
{
    return PA.pixels[(h - y - 1) * w + x];
}

unsigned int *load_image(char *filename, unsigned int *width, unsigned int *height)
{
    FILE *fp = fopen(filename, "rb");
    unsigned int *p;

    if (fp != NULL)
    {
        fread((char *)&BFH, sizeof(uint8_t), sizeof(struct BitmapFileHeader), fp);

        fread((char *)&BIH, sizeof(uint8_t), sizeof(struct BITMAPINFOHEADER), fp);

        uint16_t bpp = invert_hex((uint8_t *)BIH.bpp, 2);

        if (bpp <= 8)
        {
            uint16_t len = invert_hex((uint8_t *)BIH.n_color_palette, 4);
            uint8_t *pixels = (uint8_t *)malloc(len*sizeof(uint8_t)*4);

            fread(pixels, sizeof(uint8_t), len*sizeof(uint8_t)*4, fp);
            CT = (struct COLORTABLE *)malloc(len*sizeof(struct COLORTABLE));

            for (int i = 0; i < len; i++)
            {
                CT[i].r = pixels[0 + i*4]; CT[i].g = pixels[1 + i*4];
                CT[i].b = pixels[2 + i*4]; CT[i].a = pixels[3 + i*4];
            }
            free(pixels);
        }

        uint32_t image_size = invert_hex((uint8_t *)BIH.image_size, 4);
        uint32_t offset = invert_hex((uint8_t *)BFH.offset, 4);
        fseek(fp, offset, SEEK_SET);

        *width = invert_hex((uint8_t *)BIH.bitmap_width, 4);
        *height = invert_hex((uint8_t *)BIH.bitmap_height, 4);

        if (image_size != 0)
        {
            PA.pixels = (uint8_t *)malloc(image_size);
            fread(PA.pixels, sizeof(uint8_t), image_size, fp);
        }
        else
        {
            PA.pixels = (uint8_t *)malloc((*width)*( *height)*3);
            fread(PA.pixels, sizeof(uint8_t), (*width)*( *height)*3, fp);
        }

        p = (unsigned int *)malloc((*width)*( *height)*sizeof(unsigned int)*3);
        if (bpp <= 8)
        {
            if (bpp == 8)
            {
                for (int y = 0; y < *height; y++)
                {
                    for (int x = 0; x < *width; x++)
                    {
                        uint8_t pixel = get_pixel(x,y, *width, *height);
                        p[y*(*width)*3 +(x*3)+0] = CT[pixel].r;
                        p[y*(*width)*3 +(x*3)+1] = CT[pixel].g;
                        p[y*(*width)*3 +(x*3)+2] = CT[pixel].b;
                    }
                }
            }

            if (bpp == 4)
            {
                for (int y = 0; y < *height; y++)
                {
                    for (int x = 0; x < *width/2; x++)
                    {
                        uint8_t pixel = get_pixel(x,y, *width/2, *height);
                        p[y*(*width)*3 +((x*3)*2)+0] = CT[(pixel & 0xF0) >> 4].r;
                        p[y*(*width)*3 +((x*3)*2)+1] = CT[(pixel & 0xF0) >> 4].g;
                        p[y*(*width)*3 +((x*3)*2)+2] = CT[(pixel & 0xF0) >> 4].b;

                        p[y*(*width)*3 +((x*3)*2)+3] = CT[pixel & 0xF].r;
                        p[y*(*width)*3 +((x*3)*2)+4] = CT[pixel & 0xF].g;
                        p[y*(*width)*3 +((x*3)*2)+5] = CT[pixel & 0xF].b;
                    }
                }
            }
        }
        else
        {
            for (int y = 0; y < *height; y++)
            {
                for (int x = 0; x < *width; x++)
                {
                    p[y*(*width)*3 +(x*3)+0] = get_pixel((x*3)+0,y,*width*3, *height);
                    p[y*(*width)*3 +(x*3)+1] = get_pixel((x*3)+1,y,*width*3, *height);
                    p[y*(*width)*3 +(x*3)+2] = get_pixel((x*3)+2,y,*width*3, *height);
                }
            }
        }
    }

    fclose(fp);

    return p;
}

#endif