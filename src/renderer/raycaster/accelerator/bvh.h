#pragma once

#include "../../../math/vector.h"
#include "../../../math/aabb.h"
#include "../hittable/object.h"
#include <vector>

struct BVHNode
{
    AABB box;
    BVHNode* nleft;
    BVHNode* nright;
    Object* left;
    Object* right;

    static BVHNode* NewBVHTree(std::vector<Object*> objects);
    static void FreeBVHTree(BVHNode* parent);
    static void PrintBVHTree(BVHNode* parent);

    bool traverse(const Ray* r, f32 tmin, f32 tmax, HitRecord* rec);
};
