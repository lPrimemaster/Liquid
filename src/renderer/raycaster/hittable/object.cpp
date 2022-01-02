#include "object.h"
#include "../geometry.h"
#include "model.h"
#include "../../../math/matrix.h"
#include "../../../math/aabb.h"
#include "../accelerator/bvh.h"
#include <vector>

internal std::vector<Object*> internal_refs;

void Object::Delete(Object* obj)
{
    if(obj != nullptr)
    {
        delete obj->model;
        delete obj;
    }
}

void Object::DeleteAll()
{
    for(auto o : internal_refs)
    {
        Object::Delete(o);
    }
    internal_refs.clear();
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
    internal_refs.push_back(o);
    return o;
}

internal bool HitMesh(const Object* self, const Ray* r, f32 tmin, f32 tmax, HitRecord* rec)
{
    TriangleMesh* mesh = (TriangleMesh*)self->model->mesh;
    // rec->m = self->model->material;
    // return true;

    if(mesh->bvh->traverse(r, tmin, tmax, rec))
    {
        rec->m = self->model->material;
        return true;
    }
}

#define CHECK_ASSIGN_S(lhs, rhs) if(lhs < rhs) rhs = lhs
#define CHECK_ASSIGN_G(lhs, rhs) if(lhs > rhs) rhs = lhs

internal AABB AABBMesh(const Object* self)
{
    TriangleMesh* mesh = (TriangleMesh*)self->model->mesh;
    return mesh->bvh->box;
}

#undef CHECK_ASSIGN_S
#undef CHECK_ASSIGN_G

Object* Object::CreateMesh(const std::string& geometryName, Material* material)
{
    Object* o = new Object();
    o->model = new Model(); // NOTE: This a pointer for later instancing maybe
    o->model->material = material;
    o->model->mesh = Geometry::GetGeometry(geometryName);
    // o->transform.tmatrix = Matrix4::Translation(center) * Matrix4::Scale(radius, radius, radius);
    // o->transform.position = center;
    o->transform.scaleValue = Vector3(1, 1, 1);
    o->hit = HitMesh;
    o->getAABB = AABBMesh;
    internal_refs.push_back(o);
    return o;
}
