#pragma once
#include "vector.h"
#include "ray.h"

struct AABB
{
    Vector3 min;
    Vector3 max;

    bool hit(const Ray* r, f32 tmin, f32 tmax) const;

    static AABB SurroundingBox(AABB a, AABB b);
};