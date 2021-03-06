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

Scene Samples::SingleSphere(std::atomic<i32>* progress)
{
    Geometry::RegisterGeometry("Sphere", new Sphere());

    Material* earthMat = Material::RegisterMaterial("EarthLambertian", new Lambertian("earthmap.bmp"));
    Object* earth  = Object::CreateSphere(Vector3(0, 0, 0), 1, earthMat);

    progress->store(50);

    BVHNode* tree = BVHNode::NewBVHTree({ earth });

    Scene world;
    world.name = "SingleEarth";
    world.top = tree;
    world.objList = { earth };
    world.sky = new ColorTexture(Vector3(0.1f, 0.1f, 0.1f));
    world.renderCamera = new Camera(Vector3(15, 15, 15), Vector3(0, 0, 0), Vector3(0, 1, 0), 45.0f, 16.0f / 9.0f, .01f, 250);
    
    progress->store(100);

    return world;
}

Scene Samples::ColoredSpheres(std::atomic<i32>* progress)
{
    Geometry::RegisterGeometry("Sphere", new Sphere());

    Material* r = Material::RegisterMaterial("Red", new Lambertian(Vector3(1, 0, 0)));
    Material* g = Material::RegisterMaterial("Green", new Lambertian(Vector3(0, 1, 0)));
    Material* b = Material::RegisterMaterial("Blue", new Lambertian(Vector3(0, 1, 1)));
    Material* c = Material::RegisterMaterial("Check", new Lambertian(Vector3(0, 0, 0), Vector3(1, 1, 1)));
    Material* l = Material::RegisterMaterial("Light", new DiffuseLight(Vector3(1, 1, 1), 2.0f));

    Object* s0  = Object::CreateSphere(Vector3(0, 0, 0), 1, r);
    Object* s1  = Object::CreateSphere(Vector3(2, 0, 0), 1, g);
    Object* s2  = Object::CreateSphere(Vector3(-2, 0, 0), 1, b);
    Object* ground  = Object::CreateSphere(Vector3(0, -1001, 0), 1000, c);
    Object* light  = Object::CreateSphere(Vector3(-5, 5, -5), 5, l);

    progress->store(50);

    BVHNode* tree = BVHNode::NewBVHTree({ s0, s1, s2, ground, light });

    Scene world;
    world.name = "ColoredSpheres";
    world.top = tree;
    world.objList = { s0, s1, s2, ground, light };
    world.sky = nullptr; // Black
    world.renderCamera = new Camera(Vector3(10, 5, 10), Vector3(0, 0, 0), Vector3(0, 1, 0), 45.0f, 16.0f / 9.0f, .01f, sqrtf(22));
    
    progress->store(100);

    return world;
}
