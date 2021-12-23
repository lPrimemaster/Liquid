#pragma once
#include "../common.h"
#include "vector.h"

namespace Random
{
    i32 RandomI32Range(i32 b, i32 e);

    f32 RandomF32Range(f32 b, f32 e);
    f32 RandomF32();

    Vector3 RandomUnitSphere();
    Vector3 RandomUnitDisk();
    Vector3 RandomUnitNorm();
}