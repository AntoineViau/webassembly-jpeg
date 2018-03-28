typedef unsigned char BYTE;
typedef unsigned long ULONG;

typedef struct Image
{
    ULONG width;
    ULONG height;
    ULONG size;
    BYTE *data;
} Image;

extern Image *readJpeg(BYTE *jpegData, ULONG dataSize);
extern Image *writeJpeg(BYTE *bitmapData, ULONG width, ULONG height, ULONG quality);
