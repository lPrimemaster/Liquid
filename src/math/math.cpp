#include "math.h"

f32 clampf32(f32 x, f32 min, f32 max)
{
    if (x < min) return min;
    if (x > max) return max;
    return x;
}