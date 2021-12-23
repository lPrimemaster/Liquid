#pragma once
#include "../../../common.h"
#include "../geometry.h"

struct Material;

struct Model
{
    Geometry* mesh;
    Material* material;
};
