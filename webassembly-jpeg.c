#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libjpeg/jpeglib.h"
#include <setjmp.h>
#include <emscripten/emscripten.h>
#include "webassembly-jpeg.h"

BYTE *srcImageBmp;
ULONG srcImageWidth;
ULONG srcImageHeight;

BYTE *EMSCRIPTEN_KEEPALIVE setSrcImage(BYTE *jpegData, ULONG size)
{
    BYTE *src = readJpeg(jpegData, size);
    ULONG *infos = (ULONG *)src;
    srcImageWidth = infos[0];
    srcImageHeight = infos[1];
    srcImageBmp = &src[BMP_OFFSET];
    EM_ASM({ console.log('setSrcImage done'); });
    return src;
}

BYTE *EMSCRIPTEN_KEEPALIVE compress(ULONG quality)
{
    BYTE *compressed = writeJpeg(srcImageBmp, srcImageWidth, srcImageHeight, quality);
    ULONG *infos = (ULONG *)compressed;
    ULONG compressedSize = infos[2];
    BYTE *ret = readJpeg(&compressed[BMP_OFFSET], compressedSize);
    free(compressed);
    EM_ASM_({ console.log('compress with quality', $0, ' done'); }, quality);
    return ret;
}
