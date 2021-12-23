#include "geometry.h"

#include <unordered_map>

internal std::unordered_map<std::string, Geometry*> GeometryRegistry;

void Geometry::RegisterGeometry(std::string name, Geometry* geometry)
{
    if(GeometryRegistry.find(name) != GeometryRegistry.end())
    {
        std::cerr << "warn: Global geometry registry already contains name " << name << " - overwriting..." << std::endl;
    }
    GeometryRegistry.emplace(name, geometry);
}

Geometry* Geometry::GetGeometry(std::string name)
{
    if(GeometryRegistry.find(name) != GeometryRegistry.end())
        return GeometryRegistry.at(name);
    else
        return nullptr;
}

void Geometry::UnloadAll()
{
    for(auto g : GeometryRegistry)
    {
        if(g.second != nullptr)
            delete g.second;
    }
}
