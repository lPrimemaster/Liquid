#pragma once
#include "raycaster/geometry.h"
#include "../math/ray.h"
#include "raycaster/accelerator/bvh.h"

struct Scene
{
    // Contains all of the scene objects
    BVHNode* top;

    // TODO: Add more settings, like scene background
};
