#include <stdlib.h>
#include <emscripten/emscripten.h>
#include "webassembly-jpeg.h"

Image *EMSCRIPTEN_KEEPALIVE compress(BYTE *jpegData, ULONG dataSize, ULONG quality)
{
    Image *src = readJpeg(jpegData, dataSize);

    Image *compressed = writeJpeg(src->data, src->width, src->height, quality);

    Image *uncompressed = readJpeg(compressed->data, compressed->size);

    int len = src->width * src->height * 4;
    BYTE *rgba = (BYTE *)malloc(len);
    for (int i = 0, j = 0; i < len; i += 4, j += 3)
    {
        rgba[i + 0] = uncompressed->data[j + 0];
        rgba[i + 1] = uncompressed->data[j + 1];
        rgba[i + 2] = uncompressed->data[j + 2];
        rgba[i + 3] = 255;
    }
    free(src->data);
    free(src);
    free(compressed->data);
    free(compressed);
    free(uncompressed->data);
    uncompressed->data = rgba;
    return uncompressed;
}