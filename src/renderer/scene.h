#pragma once
#include "raycaster/geometry.h"
#include "../math/ray.h"
#include "raycaster/accelerator/bvh.h"
#include "raycaster/material.h"
#include "camera.h"

struct Scene
{
    // Contains all of the scene objects
    BVHNode* top;
    Texture* sky;
    Camera* renderCamera;
    std::string name = "Unnamed";
    
    static void FreeScene(Scene* s)
    {
        BVHNode::FreeBVHTree(s->top);
        delete s->sky;
        delete s->renderCamera;
    }
};
