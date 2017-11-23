#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jpeglib.h"
#include <setjmp.h>
#include <emscripten/emscripten.h>

extern JSAMPLE *image_buffer; /* Points to large array of R,G,B-order data */

typedef unsigned char BYTE; // 8-bit byte

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
readJpeg(BYTE *data, unsigned long dataSize)
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
    int width = cinfo.output_width;
    int height = cinfo.output_height;
    int pixelSize = cinfo.output_components;
    BYTE *dst = (BYTE *)malloc(width * height * pixelSize + 2 * 2);
    ((short *)dst)[0] = width;
    ((short *)dst)[1] = height;
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

//std::vector<JOCTET> my_buffer;
// BYTE* my_buffer;
// #define BLOCK_SIZE 16384

// void my_init_destination(j_compress_ptr cinfo)
// {
//     //my_buffer.resize(BLOCK_SIZE);
//     my_buffer = (BYTE*)malloc(BLOCK_SIZE);
//     cinfo->dest->next_output_byte = &my_buffer[0];
//     cinfo->dest->free_in_buffer = my_buffer.size();
// }

// boolean my_empty_output_buffer(j_compress_ptr cinfo)
// {
//     size_t oldsize = my_buffer.size();
//     my_buffer.resize(oldsize + BLOCK_SIZE);
//     cinfo->dest->next_output_byte = &my_buffer[oldsize];
//     cinfo->dest->free_in_buffer = my_buffer.size() - oldsize;
//     return true;
// }

// void my_term_destination(j_compress_ptr cinfo)
// {
//     my_buffer.resize(my_buffer.size() - cinfo->dest->free_in_buffer);
// }

// cinfo.dest->init_destination = &my_init_destination;
// cinfo.dest->empty_output_buffer = &my_empty_output_buffer;
// cinfo.dest->term_destination = &my_term_destination;

GLOBAL(BYTE *)
writeJpeg(BYTE *bmp, int width, int height, int quality)
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
    unsigned long bufferSize = width * height * 3;
    BYTE *buffer = (BYTE *)malloc(bufferSize);
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

    // printf("%08X, %d", buffer, bufferSize);

    BYTE *dst = (BYTE *)calloc(width * height * 3 + 1 * 4, 1);
    ((int *)dst)[0] = bufferSize;
    memcpy(&dst[4], buffer, bufferSize);
    free(buffer);
    return dst;
}

// ---------------------------------------------------------------------------

char out[32 * 1024];
void dec2hex(BYTE dec, char *hex)
{
    BYTE high, low;
    high = dec / 16;
    hex[0] = high + (high < 10 ? '0' : 'a' - 10);
    low = dec % 16;
    hex[1] = low + (low < 10 ? '0' : 'a' - 10);
}
BYTE *EMSCRIPTEN_KEEPALIVE hexString(BYTE *str, int size)
{
    memset(out, 0, sizeof(out));
    int i, j;
    for (i = 0, j = 0; i < size; i++, j += 2)
    {
        dec2hex((BYTE)str[i], &out[j]);
    }
    return (BYTE *)out;
}

// ---------------------------------------------------------------------------
// TODO : 
// - setSrcImage(jpegData)
// - compresser en WASM, retourner le jpeg brut et l'afficher par JS
BYTE *EMSCRIPTEN_KEEPALIVE doData(BYTE *jpegData, int size, int quality)
{
    BYTE *src = readJpeg(jpegData, size);
    short width = ((short *)src)[0];
    short height = ((short *)src)[1];

    BYTE *bmp = &src[2 * 2];
    BYTE *compressed = writeJpeg(bmp, width, height, quality);
    int compressedSize = ((int*)compressed)[0];

    return readJpeg(&compressed[4], compressedSize);
}
