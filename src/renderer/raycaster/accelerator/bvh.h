#pragma once

#include "../../../math/vector.h"
#include "../../../math/aabb.h"
#include "../hittable/object.h"
#include "../geometry.h"
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

struct BVHNodeTri
{
    AABB box;
    BVHNodeTri* nleft;
    BVHNodeTri* nright;
    Triangle* left;
    Triangle* right;
    TriangleMesh* mesh;

    static BVHNodeTri* NewBVHTriTree(TriangleMesh* mesh);
    static void FreeBVHTriTree(BVHNodeTri* parent);
    static void PrintBVHTriTree(BVHNodeTri* parent);

    bool traverse(const Ray* r, f32 tmin, f32 tmax, HitRecord* rec);
};
