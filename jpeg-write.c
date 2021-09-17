#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jpeglib.h>
#include <setjmp.h>
#include "webassembly-jpeg.h"

typedef struct
{
    struct jpeg_destination_mgr pub; /* public fields */

    unsigned char **outbuffer; /* target buffer */
    unsigned long *outsize;
    unsigned char *newbuffer; /* newly allocated buffer */
    JOCTET *buffer;           /* start of buffer */
    size_t bufsize;
} my_mem_destination_mgr;

typedef my_mem_destination_mgr *my_mem_dest_ptr;

GLOBAL(Image *)
writeJpeg(BYTE *bmp, ULONG width, ULONG height, ULONG quality)
{
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    JSAMPROW row_pointer[1];
    int row_stride;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;
    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, TRUE);
    ULONG bufferSize = 0;
    BYTE *buffer = NULL;
    jpeg_mem_dest(&cinfo, &buffer, &bufferSize);
    jpeg_start_compress(&cinfo, TRUE);
    row_stride = width * 3;
    while (cinfo.next_scanline < cinfo.image_height)
    {
        row_pointer[0] = &bmp[cinfo.next_scanline * row_stride];
        (void)jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }
    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);

    Image *pImage = (Image *)malloc(sizeof(Image));
    pImage->width = width;
    pImage->height = height;
    pImage->compressedSize = bufferSize;
    pImage->data = (BYTE *)malloc(bufferSize);
    memcpy(pImage->data, buffer, bufferSize);
    free(buffer);

    // BYTE *dst = (BYTE *)calloc(BMP_OFFSET + bufferSize, 1);
    // ULONG *infos = (ULONG *)dst;
    // infos[0] = width;
    // infos[1] = width;
    // infos[2] = bufferSize;
    // memcpy(&dst[BMP_OFFSET], buffer, bufferSize);
    // free(buffer);
    return pImage;
}
