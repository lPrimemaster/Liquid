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

    f32 scale = 1.0f / 128;
    for(i32 j = ctx->jstart; j < ctx->jstart + ctx->jspan; j++)
    {
        for(i32 i = ctx->istart; i < ctx->istart + ctx->ispan; i++)
        {
            Vector3 pixel_color(0, 0, 0);
            for(i32 s = 0; s < 128; s++)
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
    Geometry::RegisterGeometry("Test", TriangleMesh::CreateMeshFromFile("SanMiguel.obj"));
    Geometry::RegisterGeometry("LightP", TriangleMesh::CreateMeshFromFile("plane.obj"));

    Material* blue   = Material::RegisterMaterial(     "ClearGlass", new Glass(Vector3(1.0f, 1.0f, 1.0f), 1.5f));
    Material* red    = Material::RegisterMaterial("BlaWhiteCheckLa", new Lambertian(Vector3(0, 0, 0), Vector3(1.0f, 1.0f, 1.0f)));
    // Material* red    = Material::RegisterMaterial("BlaWhiteCheckLa", new Lambertian("uvtex.png"));
    Material* green  = Material::RegisterMaterial("EarthLambertian", new Lambertian("earthmap.bmp"));
    Material* metal  = Material::RegisterMaterial(          "Metal", new Metal(Vector3(1.0f, 1.0f, 1.0f), 0.01f));
    Material* glass  = Material::RegisterMaterial(      "GlassLowN", new Glass(Vector3(1.0f, 1.0f, 1.0f), 1.5f));
    Material* diff   = Material::RegisterMaterial(     "DiffuseRed", new Lambertian(Vector3(0.7f, 0.9f, 0.8f)));
    Material* lmat   = Material::RegisterMaterial(          "Light", new DiffuseLight(Vector3(1, 1, 1), 2.0f));

    Object* sphere  = Object::CreateSphere(Vector3(0, 0, 0), 0.5f, metal);
    Object* ground  = Object::CreateSphere(Vector3(0, -1000, 0), 1000, red);
    Object* sphere2 = Object::CreateSphere(Vector3(2.1f, 0, -1), 0.5f, green);
    Object* light0  = Object::CreateSphere(Vector3(0, 10, 0), 4.5f, lmat);
    Object* test    = Object::CreateMesh("Test", diff);
    
    BVHNode* tree = BVHNode::NewBVHTree({ ground, test });

    Scene world;
    world.top = tree;
    world.sky = new ImageTexture("HDR_040_Field.hdr");

    const u32 w = 1280;
    const u32 h = w * 9.0f / 16.0f;

    Image image(w, h, 4);

    Camera cam(Vector3(2, 2, 2), Vector3(0, 1, 0), Vector3(0, 1, 0), 35.0f, 16.0f / 9.0f, .01f, 6.5f);
    
    ThreadPool pool(8, 8, &world, &image, &cam, calculateChunk);

    pool.run();

    run_window(&image, pool.getImage_mtx());

    pool.fence();

    BVHNode::FreeBVHTree(tree);


    image.saveToBMP("output.bmp");

    delete world.sky;
    Object::DeleteAll();
    Material::UnloadAll();
    Geometry::UnloadAll();
    return 0;
}
