#pragma once
#include "../../../common.h"
#include "../../../math/transform.h"
#include "../../../math/aabb.h"
#include "../hit_record.h"

struct Model;

struct Object
{
    // This eliminates the need for virtual functions and their overhead
    bool (*hit)(const Object* self, const Ray* r, f32 tmin, f32 tmax, HitRecord* rec);
    AABB (*getAABB)(const Object* self);
    Model* model;
    Transform transform;

    static Object* CreateSphere(Vector3 center, f32 radius, Material* material);
    static void Delete(Object* obj);
    static void DeleteAll();
};