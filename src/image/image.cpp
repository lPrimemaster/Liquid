#include "image.h"

#include <Windows.h>

const int BYTES_PER_PIXEL  =  3;
const int FILE_HEADER_SIZE = 14;
const int INFO_HEADER_SIZE = 40;

void generateBitmapImage(unsigned char* image, int height, int width, const char* imageFileName);
unsigned char* createBitmapFileHeader(int height, int stride);
unsigned char* createBitmapInfoHeader(int height, int width);

// NOTE: Not used yet
internal Image* read_bitmap_image(const char* filename)
{
    TCHAR full_path[128];
    GetModuleFileName(NULL, full_path, 128);

    char path_only[128];
    _splitpath(full_path, path_only, &path_only[2], NULL, NULL);
    sprintf(path_only, "%s%s", path_only, filename);

    HBITMAP hBMP =(HBITMAP)LoadImage(NULL, path_only, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_DEFAULTSIZE | LR_LOADFROMFILE);

    BITMAP bitmap;
    GetObject(hBMP, sizeof(bitmap), (LPVOID)&bitmap);

    u8 bpp = (u8)(bitmap.bmWidthBytes / bitmap.bmWidth);

    Image* img = new Image(bitmap.bmWidth, bitmap.bmHeight, bpp);

    memcpy(img->data, bitmap.bmBits, bitmap.bmHeight * bitmap.bmWidthBytes);
    
    DeleteObject(hBMP);

    return img;
}

internal void generateBitmapImage (unsigned char* image, int height, int width, const char* imageFileName)
{
    int widthInBytes = width * BYTES_PER_PIXEL;

    unsigned char padding[3] = {0, 0, 0};
    int paddingSize = (4 - (widthInBytes) % 4) % 4;

    int stride = (widthInBytes) + paddingSize;

    FILE* imageFile = fopen(imageFileName, "wb");

    unsigned char* fileHeader = createBitmapFileHeader(height, stride);
    fwrite(fileHeader, 1, FILE_HEADER_SIZE, imageFile);

    unsigned char* infoHeader = createBitmapInfoHeader(height, width);
    fwrite(infoHeader, 1, INFO_HEADER_SIZE, imageFile);

    u8* ndata = (u8*)malloc(sizeof(u8) * 3 * width * height);

    for(i32 y = 0; y < height; y++)
    {
        for(i32 x = 0; x < width; x++)
        {
            ndata[3 * y * width + 3 * x    ] = image[4 * y * width + 4 * x    ];
            ndata[3 * y * width + 3 * x + 1] = image[4 * y * width + 4 * x + 1];
            ndata[3 * y * width + 3 * x + 2] = image[4 * y * width + 4 * x + 2];
        }
    }

    int i;
    for (i = height - 1; i >= 0; i--) {
        fwrite(ndata + (i*widthInBytes), BYTES_PER_PIXEL, width, imageFile);
        fwrite(padding, 1, paddingSize, imageFile);
    }

    free(ndata);
    fclose(imageFile);
}

internal unsigned char* createBitmapFileHeader(int height, int stride)
{
    int fileSize = FILE_HEADER_SIZE + INFO_HEADER_SIZE + (stride * height);

    static unsigned char fileHeader[] = {
        0,0,     /// signature
        0,0,0,0, /// image file size in bytes
        0,0,0,0, /// reserved
        0,0,0,0, /// start of pixel array
    };

    fileHeader[ 0] = (unsigned char)('B');
    fileHeader[ 1] = (unsigned char)('M');
    fileHeader[ 2] = (unsigned char)(fileSize      );
    fileHeader[ 3] = (unsigned char)(fileSize >>  8);
    fileHeader[ 4] = (unsigned char)(fileSize >> 16);
    fileHeader[ 5] = (unsigned char)(fileSize >> 24);
    fileHeader[10] = (unsigned char)(FILE_HEADER_SIZE + INFO_HEADER_SIZE);

    return fileHeader;
}

internal unsigned char* createBitmapInfoHeader(int height, int width)
{
    static unsigned char infoHeader[] = {
        0,0,0,0, /// header size
        0,0,0,0, /// image width
        0,0,0,0, /// image height
        0,0,     /// number of color planes
        0,0,     /// bits per pixel
        0,0,0,0, /// compression
        0,0,0,0, /// image size
        0,0,0,0, /// horizontal resolution
        0,0,0,0, /// vertical resolution
        0,0,0,0, /// colors in color table
        0,0,0,0, /// important color count
    };

    infoHeader[ 0] = (unsigned char)(INFO_HEADER_SIZE);
    infoHeader[ 4] = (unsigned char)(width      );
    infoHeader[ 5] = (unsigned char)(width >>  8);
    infoHeader[ 6] = (unsigned char)(width >> 16);
    infoHeader[ 7] = (unsigned char)(width >> 24);
    infoHeader[ 8] = (unsigned char)(height      );
    infoHeader[ 9] = (unsigned char)(height >>  8);
    infoHeader[10] = (unsigned char)(height >> 16);
    infoHeader[11] = (unsigned char)(height >> 24);
    infoHeader[12] = (unsigned char)(1);
    infoHeader[14] = (unsigned char)(BYTES_PER_PIXEL*8);

    return infoHeader;
}

void Image::saveToBMP(std::string filename) const
{
    generateBitmapImage(data, h, w, filename.c_str());
}