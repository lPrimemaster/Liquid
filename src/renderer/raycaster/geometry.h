#pragma once

#include "../../common.h"
#include "../../math/vector.h"

struct Geometry
{
    static void RegisterGeometry(std::string name, Geometry* geometry);
    static Geometry* GetGeometry(std::string name);
    static void UnloadAll();
};

struct Sphere : Geometry
{
    
};