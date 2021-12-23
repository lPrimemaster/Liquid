#include "material.h"
#include "../../math/random.h"
#include <unordered_map>

internal std::unordered_map<std::string, Material*> MaterialRegistry;


internal bool ScatterLambertian(const Material* self, const Ray* r, Ray* scattered, HitRecord* rec, Vector3* outColor)
{
    scattered->origin = rec->p;
    scattered->direction = rec->n + Random::RandomUnitNorm();

    if (scattered->direction.nearZero())
        scattered->direction = rec->n;
    
    Lambertian* ptr = (Lambertian*)self;
    *outColor = ptr->color;

    return true;
}

Lambertian::Lambertian(Vector3 color) : color(color) { scatter = ScatterLambertian; }

internal bool ScatterDiffuseLight(const Material* self, const Ray* r, Ray* scattered, HitRecord* rec, Vector3* outColor)
{
    return false;
}

internal Vector3 EmitDiffuseLight(const Material* self, f32 u, f32 v, const Vector3& point)
{
    DiffuseLight* ptr = (DiffuseLight*)self;
    return ptr->color;
}

DiffuseLight::DiffuseLight(Vector3 color, f32 intensity) : color(color * intensity) 
{ 
    scatter = ScatterDiffuseLight;
    emit    = EmitDiffuseLight;
}

Material* Material::RegisterMaterial(std::string name, Material* material)
{
    if(MaterialRegistry.find(name) != MaterialRegistry.end())
    {
        std::cerr << "warn: Global material registry already contains name " << name << " - overwriting..." << std::endl;
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
            delete g.second;
    }
}