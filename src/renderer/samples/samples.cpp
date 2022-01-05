#include "../raycaster/hittable/object.h"
#include "../raycaster/geometry.h"
#include "../raycaster/hittable/object.h"
#include "../raycaster/accelerator/bvh.h"
#include "../scene.h"
#include "../../image/image.h"
#include "../../math/random.h"
#include "../camera.h"
#include "../raycaster/caster.h"
#include "../raycaster/material.h"
#include "../../thread/threadpool.h"
#include "samples.h"

Scene Samples::BasicSphere(std::atomic<i32>* progress)
{
    Geometry::RegisterGeometry("Sphere", new Sphere());

    Material* earthMat = Material::RegisterMaterial("EarthLambertian", new Lambertian("earthmap.bmp"));
    Material* sunMat = Material::RegisterMaterial("SunDiffuse", new DiffuseLight(Vector3(1.0f, 0.83f, 0.25f), 1.0f));
    
    Object* earth  = Object::CreateSphere(Vector3(0, 0, 0), 1, earthMat);
    Object* sun  = Object::CreateSphere(Vector3(200, 1, 0), 100, sunMat);

    progress->store(50);

    BVHNode* tree = BVHNode::NewBVHTree({ sun, earth });

    Scene world;
    world.name = "SimpleSpaceEarth";
    world.top = tree;
    world.objList = { sun, earth };
    world.sky = nullptr; // = Black
    world.renderCamera = new Camera(Vector3(0, 0, 250), Vector3(50, 0, 0), Vector3(0, 1, 0), 15.0f, 16.0f / 9.0f, .01f, 250);

    progress->store(100);

    return world;
} 