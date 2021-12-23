#pragma once
#include "../../math/vector.h"

struct Material;

enum class HitFace
{
    FRONT, BACK
};

struct HitRecord
{
    Vector3 p;   // Hit point
    Vector3 n;   // Hit normal
    f32 t;       // Hit distance
    HitFace f;   // Hit face (front/back)
    Material* m; // Hit material
    Vector2 uv;  // Hit Texcoord

    inline void SetFace(const Ray* r, const Vector3 N)
    {
        f = Vector3::Dot(r->direction, N) < 0.0f ? HitFace::FRONT : HitFace::BACK;
        n = (f == HitFace::FRONT) ? N : -N;
    }
};
