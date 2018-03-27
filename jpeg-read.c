#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libjpeg/jpeglib.h"
#include <setjmp.h>
#include "webassembly-jpeg.h"

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
    ULONG width = cinfo.output_width;
    ULONG height = cinfo.output_height;
    int pixelSize = cinfo.output_components;
    BYTE *dst = (BYTE *)malloc(BMP_OFFSET + width * height * pixelSize);
    ULONG *infos = (ULONG *)dst;
    infos[0] = width;
    infos[1] = height;
    infos[2] = dataSize;
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
