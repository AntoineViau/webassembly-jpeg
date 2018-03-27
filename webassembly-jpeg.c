#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <emscripten/emscripten.h>
#include "webassembly-jpeg.h"

Image *pSrcImage;

Image *EMSCRIPTEN_KEEPALIVE setSrcImage(BYTE *jpegData, ULONG size)
{
    pSrcImage = readJpeg(jpegData, size);
    EM_ASM({ console.log('setSrcImage done'); });
    return pSrcImage;
}

Image *EMSCRIPTEN_KEEPALIVE compress(ULONG quality)
{

    Image *pCompressedImage = writeJpeg(pSrcImage->data, pSrcImage->width, pSrcImage->height, quality);

    Image *pDecompressedImage = readJpeg(pCompressedImage->data, pCompressedImage->compressedSize);

    free(pCompressedImage->data);
    free(pCompressedImage);

    EM_ASM_({ console.log('compress with quality', $0, ' done'); }, quality);

    return pDecompressedImage;
}
