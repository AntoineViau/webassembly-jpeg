#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libjpeg/jpeglib.h"
#include <setjmp.h>
#include <emscripten/emscripten.h>

typedef unsigned char BYTE;    // 1 byte
typedef unsigned int UINT;     // 4 bytes int ?
typedef unsigned long ULONG;   // 4 bytes int ?
typedef unsigned short USHORT; // 2 bytes


#define WIDTH_OFFSET 0
#define HEIGHT_OFFSET 2
#define COMPRESSED_SIZE_OFFSET 4
#define BMP_OFFSET 8

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
readJpeg(BYTE *jpegData, ULONG dataSize)
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
    jpeg_mem_src(&cinfo, (BYTE *)jpegData, dataSize);
    (void)jpeg_read_header(&cinfo, TRUE);
    (void)jpeg_start_decompress(&cinfo);
    USHORT width = cinfo.output_width;
    USHORT height = cinfo.output_height;
    int pixelSize = cinfo.output_components;
    BYTE *dst = (BYTE *)malloc(BMP_OFFSET + width * height * pixelSize);
    *((USHORT*)(&dst[WIDTH_OFFSET])) = width;
    *((USHORT*)(&dst[HEIGHT_OFFSET])) = height;
    *((ULONG*)(&dst[COMPRESSED_SIZE_OFFSET])) = dataSize;
    BYTE *bmp = &dst[BMP_OFFSET];
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

GLOBAL(BYTE *)
writeJpeg(BYTE *bmp, USHORT width, USHORT height, USHORT quality)
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

    BYTE *dst = (BYTE *)calloc(BMP_OFFSET + bufferSize, 1);
    *((ULONG *)(&dst[COMPRESSED_SIZE_OFFSET])) = bufferSize;
    memcpy(&dst[BMP_OFFSET], buffer, bufferSize);
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
    srcImageBmp = &src[BMP_OFFSET];
    return src;
}

BYTE *EMSCRIPTEN_KEEPALIVE compress(USHORT quality)
{
    BYTE *compressed = writeJpeg(srcImageBmp, srcImageWidth, srcImageHeight, quality);
    ULONG compressedSize = ((ULONG *)compressed)[1];
    BYTE *ret = readJpeg(&compressed[BMP_OFFSET], compressedSize);
    free(compressed);
    return ret;
}
