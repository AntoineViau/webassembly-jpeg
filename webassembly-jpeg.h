typedef unsigned char BYTE;  // 1 byte
typedef unsigned long ULONG; // 4 bytes

#define WIDTH_OFFSET 0
#define HEIGHT_OFFSET 4
#define COMPRESSED_SIZE_OFFSET 8
#define BMP_OFFSET 12

extern BYTE *readJpeg(BYTE *jpegData, ULONG dataSize);
extern BYTE *writeJpeg(BYTE *bmp, ULONG width, ULONG height, ULONG quality);