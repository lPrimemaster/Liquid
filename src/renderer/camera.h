#pragma once
#include "../math/vector.h"
#include "../math/ray.h"
#include "../math/random.h"
#include "../math/matrix.h"

struct Camera
{
    Camera(Vector3 pos, Vector3 lookAt, Vector3 up, f32 fov, f32 aspect, f32 aperture, f32 focus)
    {
        f32 theta = fov * PI / 180.0f;
        f32 h = tanf(theta / 2.0f);
        f32 viewport_height = 2.0f * h;
        f32 viewport_width = aspect * viewport_height;

        Vector3 w = (pos - lookAt).normalized();
        u0 = Vector3::Cross(up, w).normalized();
        v0 = Vector3::Cross(w, u0);

        origin = pos;
        horizontal = u0 * (focus * viewport_width);
        vertical   = v0 * (focus * viewport_height);
        
        lower_left = origin - horizontal / 2 - vertical / 2 - (w * focus);

        lr = aperture / 2.0f;

        staticView = Matrix4::LookAt(pos, lookAt, up);
        projection = Matrix4::Projection(fov, aspect, 0.1f, 1000.0f);

        direction = -w;
        this->up = up;

        ifov = fov;
        iaspect = aspect;
        iaperture = aperture;
        ifocus = focus;
    }

    POSSIBLE_INLINE void updateView(Vector3 pos, Vector3 lookAt)
    {
        f32 theta = ifov * PI / 180.0f;
        f32 h = tanf(theta / 2.0f);
        f32 viewport_height = 2.0f * h;
        f32 viewport_width = iaspect * viewport_height;

        Vector3 w = (pos - lookAt).normalized();
        u0 = Vector3::Cross(up, w).normalized();
        v0 = Vector3::Cross(w, u0);

        origin = pos;
        horizontal = u0 * (ifocus * viewport_width);
        vertical   = v0 * (ifocus * viewport_height);
        
        lower_left = origin - horizontal / 2 - vertical / 2 - (w * ifocus);

        lr = iaperture / 2.0f;

        staticView = Matrix4::LookAt(pos, lookAt, up);
        projection = Matrix4::Projection(ifov, iaspect, 0.1f, 1000.0f);

        direction = -w;
        this->up = up;
    }

    FORCE_INLINE Ray shootRay(f32 u, f32 v)
    {
        Ray r;
        Vector3 rd = Random::RandomUnitDisk() * lr;
        Vector3 offset = u0 * rd.x + v0 * rd.y;

        r.origin = origin + offset;
        r.direction = lower_left + horizontal * u + vertical * v - origin - offset;
        return r;
    }

    Matrix4 staticView;
    Matrix4 projection;
    Vector3 origin;
    Vector3 direction;
    Vector3 up;
    Vector3 lower_left;
    Vector3 horizontal, vertical;
    Vector3 u0, v0;
    f32 lr;
    f32 ifov;
    f32 iaspect;
    f32 iaperture;
    f32 ifocus;
};
