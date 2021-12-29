#include "caster.h"
#include "accelerator/bvh.h"
#include "material.h"
#include "../../math/random.h"


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

    // Black sky
    // return Vector3(0, 0, 0);

    // Naive texture sky
    Vector3 N = r->direction.normalized();
    Vector2 uv = Vector2(
        (atan2f(-N.z, N.x) + PI) / (2 * PI), 
        acosf(-N.y) / PI
    );
    return world->sky->sample(world->sky, uv, N);


    // Blue gradient sky
    // Vector3 u_dir = r->direction.normalized();
    // f32 t = 0.5f * (u_dir.y + 1.0f);
    // static const Vector3 white  (1.0f, 1.0f, 1.0f);
    // static const Vector3 blueish(0.5f, 0.7f, 1.0f);
    // return white * (1.0f - t) + blueish * t;
}