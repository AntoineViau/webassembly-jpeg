#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jpeglib.h"
#include <setjmp.h>
#include <emscripten/emscripten.h>

typedef unsigned char BYTE;  // 1 byte
typedef unsigned int UINT;   // 4 bytes int ?
typedef unsigned long ULONG; // 4 bytes int ?
typedef unsigned short USHORT; // 2 bytes

struct my_error_mgr
{
    struct jpeg_error_mgr pub; /* "public" fields */
    jmp_buf setjmp_buffer;     /* for return to caller */
};

typedef struct my_error_mgr *my_error_ptr;

METHODDEF(void)
my_error_exit(j_common_ptr cinfo)
{
    my_error_ptr myerr = (my_error_ptr)cinfo->err;
    (*cinfo->err->output_message)(cinfo);
    longjmp(myerr->setjmp_buffer, 1);
}

GLOBAL(BYTE *)
readJpeg(BYTE *data, ULONG dataSize)
{
    struct jpeg_decompress_struct cinfo;
    struct my_error_mgr jerr;
    JSAMPARRAY buffer;
    int row_stride;

    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;
    if (setjmp(jerr.setjmp_buffer))
    {
        jpeg_destroy_decompress(&cinfo);
        return 0;
    }
    jpeg_create_decompress(&cinfo);
    jpeg_mem_src(&cinfo, (BYTE *)data, dataSize);
    (void)jpeg_read_header(&cinfo, TRUE);
    (void)jpeg_start_decompress(&cinfo);
    USHORT width = cinfo.output_width;
    USHORT height = cinfo.output_height;
    int pixelSize = cinfo.output_components;
    BYTE *dst = (BYTE *)malloc(width * height * pixelSize + 2 * 2);
    ((USHORT *)dst)[0] = width;
    ((USHORT *)dst)[1] = height;
    BYTE *bmp = &dst[2 * 2];
    row_stride = cinfo.output_width * cinfo.output_components;
    while (cinfo.output_scanline < cinfo.output_height)
    {
        BYTE *buffer_array[1];
        buffer_array[0] = bmp + (cinfo.output_scanline) * row_stride;
        jpeg_read_scanlines(&cinfo, buffer_array, 1);
    }
    (void)jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    return dst;
}

// ---------------------------------------------------------------------------

GLOBAL(BYTE *)
writeJpeg(BYTE *bmp, UINT width, int height, int quality)
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
    ULONG bufferSize;
    BYTE *buffer;
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

    BYTE *dst = (BYTE *)calloc(width * height * 3 + 1 * 4, 1);
    ((ULONG *)dst)[0] = bufferSize;
    memcpy(&dst[4], buffer, bufferSize);
    free(buffer);
    return dst;
}

// ---------------------------------------------------------------------------

BYTE *srcImageBmp;
USHORT srcImageWidth;
USHORT srcImageHeight;

BYTE *EMSCRIPTEN_KEEPALIVE setSrcImage(BYTE *jpegData, ULONG size)
{
    BYTE *src = readJpeg(jpegData, size);
    srcImageWidth = ((USHORT *)src)[0];
    srcImageHeight = ((USHORT *)src)[1];
    srcImageBmp = &src[4];
    return src;
}

BYTE *EMSCRIPTEN_KEEPALIVE compress(USHORT quality)
{
    BYTE *compressed = writeJpeg(srcImageBmp, srcImageWidth, srcImageHeight, quality);
    ULONG compressedSize = ((ULONG *)compressed)[0];
    BYTE *ret = readJpeg(&compressed[4], compressedSize);
    free(compressed);
    return ret;
}

