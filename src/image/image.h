#pragma once
#include "../common.h"
#include "../math/vector.h"
#include "../math/math.h"

#define IMAGE_FORMAT_BPP 4

struct Image
{
    u32 w;
    u32 h;
    u8 bpp;
    u8* data;

    Image(u32 w, u32 h, u8 bpp) : w(w), h(h), bpp(bpp) { data = new u8[w * h * bpp]; }
    ~Image() { delete[] data; }

    POSSIBLE_INLINE void setPixel(u32 x, u32 y, const Vector3& value)
    {
        // Skip data check for speed - FORMAT = BGRA
        data[IMAGE_FORMAT_BPP * y * w + IMAGE_FORMAT_BPP * x    ] = (u8)(clampf32(value.z, 0.0f, 0.999f) * 256);
        data[IMAGE_FORMAT_BPP * y * w + IMAGE_FORMAT_BPP * x + 1] = (u8)(clampf32(value.y, 0.0f, 0.999f) * 256);
        data[IMAGE_FORMAT_BPP * y * w + IMAGE_FORMAT_BPP * x + 2] = (u8)(clampf32(value.x, 0.0f, 0.999f) * 256);
        data[IMAGE_FORMAT_BPP * y * w + IMAGE_FORMAT_BPP * x + 3] = 0x00;
    }

    POSSIBLE_INLINE void setAll(const Vector3& value)
    {
        for(u32 i = 0; i < w * h * IMAGE_FORMAT_BPP; i += IMAGE_FORMAT_BPP)
        {
            *(data + i    ) = (u8)(clampf32(value.z, 0.0f, 0.999f) * 256);
            *(data + i + 1) = (u8)(clampf32(value.y, 0.0f, 0.999f) * 256);
            *(data + i + 2) = (u8)(clampf32(value.x, 0.0f, 0.999f) * 256);
            *(data + i + 3) = 0x00;
        }
    }

    void saveToBMP(std::string filename) const;
};