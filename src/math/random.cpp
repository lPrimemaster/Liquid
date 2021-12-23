#include "random.h"
#include <random>

internal std::random_device GlobalRandDevice;
internal std::mt19937 GlobalGenerator(GlobalRandDevice());

i32 Random::RandomI32Range(i32 b, i32 e)
{
    std::uniform_int_distribution<i32> dist(b, e);
    return dist(GlobalGenerator);
}

f32 Random::RandomF32Range(f32 b, f32 e)
{
    std::uniform_real_distribution<f32> dist(b, e);
    return dist(GlobalGenerator);
}

f32 Random::RandomF32()
{
    std::uniform_real_distribution<f32> dist(0.0f, 1.0f);
    return dist(GlobalGenerator);
}

Vector3 Random::RandomUnitSphere()
{
    while(1)
    {
        Vector3 point(RandomF32Range(-1, 1), RandomF32Range(-1, 1), RandomF32Range(-1, 1));
        if(Vector3::Dot(point, point) < 1) return point;
    }
}

Vector3 Random::RandomUnitDisk()
{
    while(1)
    {
        Vector3 point(RandomF32Range(-1, 1), RandomF32Range(-1, 1), 0);
        if(Vector3::Dot(point, point) < 1) return point;
    }
}

Vector3 Random::RandomUnitNorm()
{
    return RandomUnitSphere().normalized();
}
