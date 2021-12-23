#include "object.h"
#include "../geometry.h"
#include "model.h"
#include "../../../math/matrix.h"
#include "../../../math/aabb.h"

void Object::Delete(Object* obj)
{
    if(obj != nullptr)
    {
        delete obj->model;
        delete obj;
    }
}

internal bool HitSphere(const Object* self, const Ray* r, f32 tmin, f32 tmax, HitRecord* rec)
{
    Sphere* s = (Sphere*)self->model->mesh;
    Vector3 center = self->transform.position;
    f32 radius = self->transform.scaleValue.x;

    Vector3 oc = r->origin - center;
    f32 a  = Vector3::Dot(r->direction, r->direction);
    f32 hb = Vector3::Dot(oc, r->direction);
    f32 c  = Vector3::Dot(oc, oc) - radius * radius;
    f32 d  = hb * hb - a * c;

    if(d < 0)
    {
        return false;
    }
    
    f32 sqrt_d = sqrtf(d);
    f32 root = (-hb - sqrt_d) / a;
    if(root < tmin || root > tmax)
    {
        root = (-hb + sqrt_d) / a;
        if(root < tmin || root > tmax)
        {
            return false;
        }
    }

    rec->t = root;
    rec->p = r->at(root);
    Vector3 N = (rec->p - center) / radius;
    rec->SetFace(r, N);
    rec->uv = Vector2(
        (atan2f(-N.z, N.x) + PI) / (2 * PI), 
        acosf(-N.y) / PI
    );
    rec->m = self->model->material;
    return true;
}

internal AABB AABBSphere(const Object* self)
{
    AABB r;
    Vector3 center = self->transform.position;
    Vector3 radius = Vector3(self->transform.scaleValue.x, self->transform.scaleValue.x, self->transform.scaleValue.x);
    r.min = center - radius;
    r.max = center + radius;
    return r;
}

Object* Object::CreateSphere(Vector3 center, f32 radius, Material* material)
{
    Object* o = new Object();
    o->model = new Model(); // NOTE: This a pointer for later instancing maybe
    o->model->material = material;
    o->model->mesh = Geometry::GetGeometry("Sphere");
    o->transform.tmatrix = Matrix4::Translation(center) * Matrix4::Scale(radius, radius, radius);
    o->transform.position = center;
    o->transform.scaleValue.x = radius; // Use scaleValue.x for the radius
    o->hit = HitSphere;
    o->getAABB = AABBSphere;
    return o;
}

