#include "renderer/raycaster/hittable/object.h"
#include "renderer/raycaster/geometry.h"
#include "renderer/raycaster/hittable/object.h"
#include "renderer/raycaster/accelerator/bvh.h"
#include "renderer/scene.h"
#include "image/image.h"
#include "math/random.h"
#include "renderer/camera.h"
#include "renderer/raycaster/caster.h"
#include "renderer/raycaster/material.h"
#include "thread/threadpool.h"

// TODO: These sources are temporary
#include "renderer/displayer/debug_display_win32.h"

internal void calculateChunk(JobContext* ctx, std::mutex* img_mtx)
{
    img_mtx->lock();
    std::cerr << "info: Starting Job " << ctx->id << "\n";
    img_mtx->unlock();

    f32 scale = 1.0f / 1024;
    for(i32 j = ctx->jstart; j < ctx->jstart + ctx->jspan; j++)
    {
        for(i32 i = ctx->istart; i < ctx->istart + ctx->ispan; i++)
        {
            Vector3 pixel_color(0, 0, 0);
            for(i32 s = 0; s < 1024; s++)
            {
                f32 u = (f32)(i + Random::RandomF32()) / ctx->img->w;
                f32 v = (f32)(j + Random::RandomF32()) / ctx->img->h;

                Ray r = ctx->cam->shootRay(u, v);
                
                pixel_color = pixel_color + RayCast(&r, ctx->world, 8);
            }

            pixel_color = pixel_color * scale;
            pixel_color = pixel_color.sqrtComponents(); // For a gamma of 2.0

            { // TODO: Better minimize the locks, place line by line instead in j loop (?)
                std::lock_guard<std::mutex> lock(*img_mtx);
                ctx->img->setPixel(i, ctx->img->h - j - 1, pixel_color);
            }
        }
    }
}

int main()
{
    Geometry::RegisterGeometry("Sphere", new Sphere());

    Material* blue  = Material::RegisterMaterial( "BlueLambertian", new Lambertian(Vector3(0.4f, 0.3f, 0.7f)));
    Material* red   = Material::RegisterMaterial(  "RedLambertian", new Lambertian(Vector3(0.5f, 0.2f, 0.1f)));
    Material* green = Material::RegisterMaterial("GreenLambertian", new Lambertian(Vector3(0.1f, 0.7f, 0.2f)));

    Material* lmat  = Material::RegisterMaterial("Light", new DiffuseLight(Vector3(1, 1, 1), 2.0f));

    Object* sphere  = Object::CreateSphere(Vector3(0, 0, -1), 0.5f, blue);
    Object* ground  = Object::CreateSphere(Vector3(0, -100.5, 0), 100, red);
    Object* sphere2 = Object::CreateSphere(Vector3(1.1f, 0, -1), 0.5f, green);
    Object* light   = Object::CreateSphere(Vector3(-3, 1, -1), 2.0f, lmat);

    BVHNode* tree = BVHNode::NewBVHTree({sphere, ground, sphere2, light});

    Scene world;

    world.top = tree;

    HitRecord test;
    Ray r;
    r.origin = Vector3(0, 1, 0);
    r.direction = Vector3(0, 0, 1);
    // tree->traverse(&r, 0, 0xFFFF, &test);

    const u32 w = 1280;
    const u32 h = w * 9.0f / 16.0f;

    Image image(w, h, 4);

    Camera cam(Vector3(2, 2, -7), Vector3(0, 0, -1), Vector3(0, 1, 0), 20.0f, 16.0f / 9.0f, 0.1f, 9.0f);
    
    ThreadPool pool(8, 8, &world, &image, &cam, calculateChunk);

    pool.run();

    run_window(&image, pool.getImage_mtx());

    pool.fence();

    BVHNode::FreeBVHTree(tree);

    // NOTE: Maybe Transfer object ownership to their BVHNode
    Object::Delete(sphere);
    Object::Delete(ground);
    Object::Delete(sphere2);

    image.saveToBMP("output.bmp");

    Material::UnloadAll();
    Geometry::UnloadAll();
    return 0;
}
