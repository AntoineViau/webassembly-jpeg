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
    for (int i = 0, j = 0; i < len; i += 3, j += 4)
    {
        rgba[j + 0] = uncompressed->data[i + 0];
        rgba[j + 1] = uncompressed->data[i + 1];
        rgba[j + 2] = uncompressed->data[i + 2];
        rgba[j + 3] = 255;
    }
    free(src->data);
    free(src);
    free(compressed->data);
    free(compressed);
    uncompressed->data = rgba;
    return uncompressed;
}
