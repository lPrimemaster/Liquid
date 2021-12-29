#pragma once
#include "../../common.h"
#include "../../math/vector.h"
#include "../../math/ray.h"
#include "hit_record.h"
#include "../../image/image.h"

struct Texture
{
    virtual ~Texture() {  };
    Vector3 (*sample)(const Texture* self, const Vector2& uv, const Vector3& p);
};

struct ColorTexture : Texture
{
    ColorTexture(const Vector3& color);
    Vector3 color;
};

struct CheckerTexture : Texture
{
    CheckerTexture(const Vector3& a, const Vector3& b);
    Vector3 colorA;
    Vector3 colorB;
};


struct ImageTexture : Texture
{
    ImageTexture(const std::string& bitmap);
    ~ImageTexture() { if(img != nullptr) delete img; }
    Image* img;
};

struct Material
{
    Texture* albedo; 
    
    bool (*scatter)(const Material* self, const Ray* r, Ray* scattered, HitRecord* rec, Vector3* outColor);
    Vector3 (*emit)(const Material* self, f32 u, f32 v, const Vector3& point) = nullptr;

    static Material* RegisterMaterial(std::string name, Material* material);
    static Material* GetMaterial(std::string name);
    static void UnloadAll();
};

struct Lambertian : Material
{
    Lambertian(Vector3 color);
    Lambertian(Vector3 checkerA, Vector3 checkerB);
    Lambertian(const std::string& image);
};

struct DiffuseLight : Material
{
    DiffuseLight(Vector3 color, f32 intensity);
};

struct Metal : Material
{
    Metal(Vector3 color, f32 fuzziness);
    f32 fuzz;
};

struct Glass : Material
{
    Glass(Vector3 color, f32 ior);
    f32 ior;
};
