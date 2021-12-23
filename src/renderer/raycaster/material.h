#pragma once
#include "../../common.h"
#include "../../math/vector.h"
#include "../../math/ray.h"
#include "hit_record.h"

struct Material
{
    // Vector3 color;
    // f32 fuzziness;
    // f32 n_refract;
    bool (*scatter)(const Material* self, const Ray* r, Ray* scattered, HitRecord* rec, Vector3* outColor);
    Vector3 (*emit)(const Material* self, f32 u, f32 v, const Vector3& point) = nullptr;

    static Material* RegisterMaterial(std::string name, Material* material);
    static Material* GetMaterial(std::string name);
    static void UnloadAll();
};

struct Lambertian : Material
{
    Lambertian(Vector3 color);
    Vector3 color;
};

struct DiffuseLight : Material
{
    DiffuseLight(Vector3 color, f32 intensity);
    Vector3 color;
};
