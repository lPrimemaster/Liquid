#pragma once

#include "vector.h"

struct Ray
{
    Vector3 origin;
    Vector3 direction;

    POSSIBLE_INLINE Vector3 at(f32 t) const
    {
        return origin + t * direction;
    }
};
