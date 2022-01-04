#include "caster.h"
#include "accelerator/bvh.h"
#include "material.h"
#include "../../math/random.h"
#include "../../thread/threadpool.h"

void calculateChunk(JobContext* ctx, std::mutex* img_mtx)
{
    img_mtx->lock();
    std::cerr << "info: Starting Job " << ctx->id << "\n";
    img_mtx->unlock();

    f32 scale = 1.0f / ctx->spp;
    f32 localLinePct = 100 / (f32)(ctx->jspan * ctx->numJobs);
    for(i32 j = ctx->jstart; j < ctx->jstart + ctx->jspan; j++)
    {
        for(i32 i = ctx->istart; i < ctx->istart + ctx->ispan; i++)
        {
            Vector3 pixel_color(0, 0, 0);
            for(i32 s = 0; s < ctx->spp; s++)
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
        std::lock_guard<std::mutex> lock(*ctx->globalDoneMtx);
        *ctx->globalDonePct += localLinePct;
    }
}

internal bool ClosestIntersect(const Ray* r, const Scene* scene, HitRecord* rec_out)
{
    bool hit = scene->top->traverse(r, 0.001f, std::numeric_limits<f32>::max(), rec_out);

    // TODO: Get background sample here (or the uvs)
    return hit;
}

Vector3 RayCast(const Ray* r, Scene* world, i32 depth)
{
    if(depth <= 0)
    {
        return Vector3(0, 0, 0);
    }
    HitRecord rec;
    if(ClosestIntersect(r, world, &rec))
    {
        Ray scatter;
        Vector3 color;
        Vector3 emit;

        if(rec.m->emit) emit = rec.m->emit(rec.m, 0, 0, Vector3());

        if(rec.m->scatter(rec.m, r, &scatter, &rec, &color))
        {
            // TODO: Don't add emit value when it is (0, 0, 0) - spare the FPU
            return emit + color * RayCast(&scatter, world, depth - 1);
        }
        return emit;
    }

    // Naive texture sky
    if(world->sky)
    {
        Vector3 N = r->direction.normalized();
        Vector2 uv = Vector2(
            (atan2f(-N.z, N.x) + PI) / (2 * PI), 
            acosf(-N.y) / PI
        );
        return world->sky->sample(world->sky, uv, N);
    }

    return Vector3(0, 0, 0);
    
    // Blue gradient sky
    // Vector3 u_dir = r->direction.normalized();
    // f32 t = 0.5f * (u_dir.y + 1.0f);
    // static const Vector3 white  (1.0f, 1.0f, 1.0f);
    // static const Vector3 blueish(0.5f, 0.7f, 1.0f);
    // return white * (1.0f - t) + blueish * t;
}