#include "aabb.h"

internal POSSIBLE_INLINE bool boxAxis(const AABB* aabb, const Ray* r, f32& tmin, f32& tmax, i32 axis)
{
    f32 tx_min = (aabb->min.data[axis] - r->origin.data[axis]) / r->direction.data[axis];
    f32 tx_max = (aabb->max.data[axis] - r->origin.data[axis]) / r->direction.data[axis];
    f32 tx_0 = fminf(tx_min, tx_max);
    f32 tx_1 = fmaxf(tx_min, tx_max);

    tmin = fmaxf(tx_0, tmin);
    tmax = fminf(tx_1, tmax);
    if(tmax <= tmin) return false;
    return true;
}

bool AABB::hit(const Ray* r, f32 tmin, f32 tmax) const
{
    if(!boxAxis(this, r, tmin, tmax, 0)) return 0;
    if(!boxAxis(this, r, tmin, tmax, 1)) return 0;
    if(!boxAxis(this, r, tmin, tmax, 2)) return 0;
    return 1;
}

AABB AABB::SurroundingBox(AABB a, AABB b)
{
    AABB r;
    r.min = Vector3(
        fminf(a.min.x, b.min.x),
        fminf(a.min.y, b.min.y),
        fminf(a.min.z, b.min.z)
    );

    r.max = Vector3(
        fmaxf(a.max.x, b.max.x),
        fmaxf(a.max.y, b.max.y),
        fmaxf(a.max.z, b.max.z)
    );

    return r;
}