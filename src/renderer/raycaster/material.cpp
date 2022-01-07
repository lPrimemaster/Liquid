#include "material.h"
#include "../../math/random.h"
#include <unordered_map>

internal std::unordered_map<std::string, Material*> MaterialRegistry;

// TODO: Consider virtual functions instead of this. Maybe not a visible performance impact / might be even faster!
internal Vector3 SampleColorTexture(const Texture* self, const Vector2& uv, const Vector3& p)
{
    return ((ColorTexture*)self)->color;
}

ColorTexture::ColorTexture(const Vector3& color) : color(color)
{
    sample = SampleColorTexture;
}

internal Vector3 SampleCheckerTexture(const Texture* self, const Vector2& uv, const Vector3& p)
{
    CheckerTexture* ptr = (CheckerTexture*)self;
    f32 sines = sinf(10 * p.x) * sinf(10 * p.y) * sinf(10 * p.z);
    if (sines < 0)
        return ptr->colorA;
    else
        return ptr->colorB;
}

CheckerTexture::CheckerTexture(const Vector3& a, const Vector3& b) : colorA(a), colorB(b)
{
    sample = SampleCheckerTexture;
}

internal Vector3 SampleImageTexture(const Texture* self, const Vector2& uv, const Vector3& p)
{
    ImageTexture* tex = (ImageTexture*)self;
    f32 u = clampf32(uv.u, 0.0f, 1.0f);
    f32 v = clampf32(uv.v, 0.0f, 1.0f);
    u32 i = (u32)(u * tex->img->w);
    u32 j = (u32)(v * tex->img->h);

    if(i >= tex->img->w) i = tex->img->w - 1;
    if(j >= tex->img->h) j = tex->img->h - 1;

    static f32 color_scale = 1.0f / 255.0f;
    // TODO: Create a new var for bpp * w
    u8* pix = tex->img->data + j * tex->img->w * tex->img->bpp + i * tex->img->bpp;

    return Vector3(
        color_scale * pix[0],
        color_scale * pix[1],
        color_scale * pix[2]
    );
}

ImageTexture::ImageTexture(const std::string& bitmap)
{
    img = new Image(bitmap);
    sample = SampleImageTexture;
}

ImageTexture::ImageTexture(const std::string& bitmap, f32 factor)
{
    img = new Image(bitmap, factor);
    sample = SampleImageTexture;
}

internal bool ScatterLambertian(const Material* self, const Ray* r, Ray* scattered, HitRecord* rec, Vector3* outColor)
{
    scattered->origin = rec->p;
    scattered->direction = rec->n + Random::RandomUnitNorm();

    if (scattered->direction.nearZero())
        scattered->direction = rec->n;
    
    Lambertian* ptr = (Lambertian*)self;
    *outColor = ptr->albedo->sample(ptr->albedo, rec->uv, rec->p);

    return true;
}

Lambertian::Lambertian(Vector3 color) 
{
    albedo = new ColorTexture(color);
    scatter = ScatterLambertian; 
}

Lambertian::Lambertian(Vector3 checkerA, Vector3 checkerB)
{
    albedo = new CheckerTexture(checkerA, checkerB);
    scatter = ScatterLambertian; 
}

Lambertian::Lambertian(const std::string& image)
{
    albedo = new ImageTexture(image);
    scatter = ScatterLambertian;
}

internal bool ScatterDiffuseLight(const Material* self, const Ray* r, Ray* scattered, HitRecord* rec, Vector3* outColor)
{
    return false;
}

internal Vector3 EmitDiffuseLight(const Material* self, f32 u, f32 v, const Vector3& point)
{
    DiffuseLight* ptr = (DiffuseLight*)self;
    return ptr->albedo->sample(ptr->albedo, Vector2(u, v), point);
}

DiffuseLight::DiffuseLight(Vector3 color, f32 intensity)
{
    albedo = new ColorTexture(color * intensity);
    scatter = ScatterDiffuseLight;
    emit    = EmitDiffuseLight;
}

internal bool ScatterMetal(const Material* self, const Ray* r, Ray* scattered, HitRecord* rec, Vector3* outColor)
{
    Metal* ptr = (Metal*)self;
    scattered->origin = rec->p;
    scattered->direction = Vector3::Reflect(r->direction.normalized(), rec->n) + (Random::RandomUnitSphere() * ptr->fuzz);
    *outColor = ptr->albedo->sample(ptr->albedo, rec->uv, rec->p);
    return (Vector3::Dot(scattered->direction, rec->n) > 0);
}

Metal::Metal(Vector3 color, f32 fuzziness) : fuzz(fuzziness)
{
    albedo = new ColorTexture(color);
    scatter = ScatterMetal;
}

internal f32 reflectance(f32 cos, f32 refract_material_ratio)
{
    f32 r0 = (1 - refract_material_ratio) / (1 + refract_material_ratio);
    r0 = r0 * r0;
    return r0 + (1 - r0) * powf(1 - cos, 5);
}

internal bool ScatterGlass(const Material* self, const Ray* r, Ray* scattered, HitRecord* rec, Vector3* outColor)
{
    Glass* ptr = (Glass*)self;
    
    f32 rratio = rec->f == HitFace::FRONT ? (1.0f / ptr->ior) : ptr->ior;
    scattered->origin = rec->p;

    Vector3  udir = r->direction.normalized();
    Vector3 iudir = -udir;

    f32 cos_theta = fminf(Vector3::Dot(iudir, rec->n), 1.0f);
    f32 sin_theta = sqrtf(1.0f - cos_theta * cos_theta);

    i8 not_refract = rratio * sin_theta > 1.0f;

    if(not_refract || reflectance(cos_theta, rratio) > Random::RandomF32())
    {
        scattered->direction = Vector3::Reflect(udir, rec->n);
    }
    else
    {
        scattered->direction = Vector3::Refract(udir, rec->n, rratio, cos_theta);
    }
    *outColor = ptr->albedo->sample(ptr->albedo, rec->uv, rec->p);
    return true;
}

Glass::Glass(Vector3 color, f32 ior) : ior(ior)
{
    albedo = new ColorTexture(color);
    scatter = ScatterGlass;
}

Material* Material::RegisterMaterial(std::string name, Material* material)
{
    auto matit = MaterialRegistry.find(name);
    if(matit != MaterialRegistry.end())
    {
        std::cerr << "warn: Global material registry already contains name " << name << " - aborting..." << std::endl;
        delete material;
        return (*matit).second;
    }
    MaterialRegistry.emplace(name, material);
    return material;
}

Material* Material::GetMaterial(std::string name)
{
    if(MaterialRegistry.find(name) != MaterialRegistry.end())
        return MaterialRegistry.at(name);
    else
        return nullptr;
}

void Material::UnloadAll()
{
    for(auto g : MaterialRegistry)
    {
        if(g.second != nullptr)
        {
            if(g.second->albedo != nullptr)
            {
                delete g.second->albedo; // TODO: Use destructors instead
            }
            delete g.second;
        }
    }
}