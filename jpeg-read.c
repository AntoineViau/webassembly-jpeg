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

GLOBAL(Image *)
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
    Image *pImage = (Image *)malloc(sizeof(Image));
    pImage->width = width;
    pImage->height = height;
    pImage->compressedSize = dataSize;
    pImage->data = (BYTE *)malloc(width * height * pixelSize);
    row_stride = cinfo.output_width * cinfo.output_components;
    while (cinfo.output_scanline < cinfo.output_height)
    {
        BYTE *buffer_array[1];
        buffer_array[0] = pImage->data + (cinfo.output_scanline) * row_stride;
        jpeg_read_scanlines(&cinfo, buffer_array, 1);
    }
    (void)jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    return pImage;
}
