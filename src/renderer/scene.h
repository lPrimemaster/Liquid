#pragma once
#include "raycaster/geometry.h"
#include "../math/ray.h"
#include "raycaster/accelerator/bvh.h"
#include "raycaster/material.h"

struct Scene
{
    // Contains all of the scene objects
    BVHNode* top;
    ImageTexture* sky;
    // TODO: Add more settings, like scene background
};
