#include "bvh.h"
#include "../../../math/random.h"
#include "../hittable/object.h"
#include "../hittable/model.h"
#include <algorithm>

bool BVHNode::traverse(const Ray* r, f32 tmin, f32 tmax, HitRecord* rec)
{
    // The ray did not hit anything
    if(!this->box.hit(r, tmin, tmax))
    { 
        return false;
    }

    // The BVH goes deeper
    if(this->nleft != nullptr)
    {
        bool hit_left  = this->nleft->traverse(r, tmin, tmax, rec);
        bool hit_right = this->nright->traverse(r, tmin, hit_left ? rec->t : tmax, rec);
        return hit_left || hit_right;
    }

    // This is a leaf with 2 Objects
    if(this->left != this->right)
    {
        bool hit_left  = this->left->hit(this->left, r, tmin, tmax, rec);
        bool hit_right = this->right->hit(this->right, r, tmin, hit_left ? rec->t : tmax, rec);
        return hit_left || hit_right;
    }
    
    // This is a leaf with 1 Object (left == right)
    return this->left->hit(this->left, r, tmin, tmax, rec);
}

bool BVHNodeTri::traverse(const Ray* r, f32 tmin, f32 tmax, HitRecord* rec)
{
    // The ray did not hit anything
    if(!this->box.hit(r, tmin, tmax))
    { 
        return false;
    }

    // The BVH goes deeper
    if(this->nleft != nullptr)
    {
        bool hit_left  = this->nleft->traverse(r, tmin, tmax, rec);
        bool hit_right = this->nright->traverse(r, tmin, hit_left ? rec->t : tmax, rec);
        return hit_left || hit_right;
    }

    // This is a leaf with 2 Objects
    if(this->left != this->right)
    {
        bool hit_left  = this->left->hit(this->mesh, r, tmin, tmax, rec);
        bool hit_right = this->right->hit(this->mesh, r, tmin, hit_left ? rec->t : tmax, rec);
        return hit_left || hit_right;
    }
    
    // This is a leaf with 1 Object (left == right)
    return this->left->hit(this->mesh, r, tmin, tmax, rec);
}

internal bool BoxCompare(Object* o0, Object* o1, i32 axis)
{
    AABB box0 = o0->getAABB(o0);
    AABB box1 = o1->getAABB(o1);

    return box0.min.data[axis] < box1.min.data[axis];
}

internal bool BoxCompareTriangle(Triangle* o0, Triangle* o1, i32 axis)
{
    return o0->box.min.data[axis] < o1->box.min.data[axis];
}

internal BVHNode* NewBVHNodeIter(const std::vector<Object*>& objects, i32 start, i32 stop)
{
    BVHNode* parent = new BVHNode();
    i32 axis = Random::RandomI32Range(0, 2);
    i32 count = stop - start;

    auto RandomBoxCompare = [axis](Object* a, Object* b) -> bool { return BoxCompare(a, b, axis); };

    if(count == 1)
    {
        parent->nleft = parent->nright = nullptr;
        parent->left = parent->right = objects[start];
    }
    else if(count == 2)
    {
        parent->nleft = parent->nright = nullptr;

        Object* a = objects[start];
        Object* b = objects[start + 1];
        if(RandomBoxCompare(a, b))
        {
            parent->left = a;
            parent->right = b;
        }
        else
        {
            parent->left = b;
            parent->right = a;
        }
    }
    else
    {
        parent->left = parent->right = nullptr;

        std::vector<Object*> sorted = objects;

        // TODO: Use some optimized sort later on
        std::sort(sorted.begin() + start, sorted.begin() + stop, RandomBoxCompare);

        i32 mid = start + count / 2;
        parent->nleft  = NewBVHNodeIter(sorted, start, mid);
        parent->nright = NewBVHNodeIter(sorted, mid, stop);
    }

    if(parent->nleft != nullptr)
    {
        // Box is from bvh child nodes
        parent->box = AABB::SurroundingBox(parent->nleft->box, parent->nright->box);
    }
    else
    {
        // Box is from object childs
        parent->box = AABB::SurroundingBox(parent->left->getAABB(parent->left), parent->right->getAABB(parent->right));
    }
    return parent;
}

BVHNode* BVHNode::NewBVHTree(std::vector<Object*> objects)
{
    return NewBVHNodeIter(objects, 0, (i32)objects.size());
}

internal AABB GetTriangleAABB(TriangleMesh* mesh, Triangle* t)
{
    AABB r;

    f32 maxX = fmaxf(mesh->vertices[t->indicesVertex[0]].x,
                    fmaxf(mesh->vertices[t->indicesVertex[1]].x,
                        mesh->vertices[t->indicesVertex[2]].x
                    )
                );
    f32 maxY = fmaxf(mesh->vertices[t->indicesVertex[0]].y,
                    fmaxf(mesh->vertices[t->indicesVertex[1]].y,
                        mesh->vertices[t->indicesVertex[2]].y
                    )
                );
    f32 maxZ = fmaxf(mesh->vertices[t->indicesVertex[0]].z,
                    fmaxf(mesh->vertices[t->indicesVertex[1]].z,
                        mesh->vertices[t->indicesVertex[2]].z
                    )
                );
    f32 minX = fminf(mesh->vertices[t->indicesVertex[0]].x,
                    fminf(mesh->vertices[t->indicesVertex[1]].x,
                        mesh->vertices[t->indicesVertex[2]].x
                    )
                );
    f32 minY = fminf(mesh->vertices[t->indicesVertex[0]].y,
                    fminf(mesh->vertices[t->indicesVertex[1]].y,
                        mesh->vertices[t->indicesVertex[2]].y
                    )
                );
    f32 minZ = fminf(mesh->vertices[t->indicesVertex[0]].z,
                    fminf(mesh->vertices[t->indicesVertex[1]].z,
                        mesh->vertices[t->indicesVertex[2]].z
                    )
                );

    r.max = Vector3(maxX, maxY, maxZ);
    r.min = Vector3(minX, minY, minZ);

    auto v1 = mesh->vertices[t->indicesVertex[0]];
    auto v2 = mesh->vertices[t->indicesVertex[1]];
    auto v3 = mesh->vertices[t->indicesVertex[2]];

    // std::cout << "Triangle\n(" << v1.x << " " << v1.y << " " << v1.z << ")\n"
    //           << "(" << v2.x << " " << v2.y << " " << v2.z << ")\n"
    //           << "(" << v3.x << " " << v3.y << " " << v3.z << ")\n";

    // std::cout << "Box\n(" << r.min.x << " " << r.min.y << " " << r.min.z << ")\n"
    //           << "(" << r.max.x << " " << r.max.y << " " << r.max.z << ")\n\n";

    return r;
}

internal BVHNodeTri* NewBVHNodeIterTriangle(TriangleMesh* mesh, i32 start, i32 stop)
{
    BVHNodeTri* parent = new BVHNodeTri();
    i32 axis = Random::RandomI32Range(0, 2);
    i32 count = stop - start;
    if(count == mesh->triangleCount) mesh->bvh = parent;

    auto RandomBoxCompare = [axis](Triangle* a, Triangle* b) -> bool { return BoxCompareTriangle(a, b, axis); };
    auto RandomBoxCompareRef = [axis](Triangle& a, Triangle& b) -> bool { return BoxCompareTriangle(&a, &b, axis); };

    if(count == 1)
    {
        parent->nleft = parent->nright = nullptr;
        parent->left = parent->right = &mesh->triangles[start];
        parent->mesh = mesh;
    }
    else if(count == 2)
    {
        parent->nleft = parent->nright = nullptr;
        parent->mesh = mesh;

        Triangle* a = &mesh->triangles[start];
        Triangle* b = &mesh->triangles[start + 1];
        if(RandomBoxCompare(a, b))
        {
            parent->left = a;
            parent->right = b;
        }
        else
        {
            parent->left = b;
            parent->right = a;
        }
    }
    else
    {
        parent->left = parent->right = nullptr;

        // std::vector<Object*> sorted = mesh->triangles;
        // Triangle* sorted = new Triangle[mesh->triangleCount];
        // memcpy(sorted, mesh->triangles, mesh->triangleCount * sizeof(Triangle));

        // TODO: Use some optimized sort later on
        std::sort(mesh->triangles + start, mesh->triangles + stop, RandomBoxCompareRef);

        i32 mid = start + count / 2;
        parent->nleft  = NewBVHNodeIterTriangle(mesh, start, mid);
        parent->nright = NewBVHNodeIterTriangle(mesh, mid, stop);
    }

    if(parent->nleft != nullptr)
    {
        // Box is from bvh child nodes
        parent->box = AABB::SurroundingBox(parent->nleft->box, parent->nright->box);
    }
    else
    {
        // Box is from object childs
        parent->left->box  = GetTriangleAABB(mesh, parent->left);
        parent->right->box = GetTriangleAABB(mesh, parent->right);
        parent->box = AABB::SurroundingBox(parent->left->box, parent->right->box);
    }
    return parent;
}

BVHNodeTri* BVHNodeTri::NewBVHTriTree(TriangleMesh* mesh)
{
    return NewBVHNodeIterTriangle(mesh, 0, (i32)mesh->triangleCount);
}

void BVHNodeTri::FreeBVHTriTree(BVHNodeTri* parent)
{
    if(parent->nleft != nullptr)
        FreeBVHTriTree(parent->nleft);
    if(parent->nright != nullptr)
        FreeBVHTriTree(parent->nright);

    delete parent;
}

void BVHNodeTri::PrintBVHTriTree(BVHNodeTri* parent)
{
    std::cout << "Node " << std::hex << parent << " --> Leaf? " << (parent->nleft ? "No" : "Yes") << "\n";

    if(!parent->nleft)
    {
        std::cout << "Child left : " << std::hex << parent->left << "\n";
        std::cout << "Child right: " << std::hex << parent->right << "\n";
    }
    else
    {
        PrintBVHTriTree(parent->nleft);
        if(parent->nleft != parent->nright)
            PrintBVHTriTree(parent->nright);
    }
}

void BVHNode::FreeBVHTree(BVHNode* parent)
{
    if(parent->nleft != nullptr)
        FreeBVHTree(parent->nleft);
    if(parent->nright != nullptr)
        FreeBVHTree(parent->nright);

    delete parent;
    parent = nullptr;
}

void BVHNode::PrintBVHTree(BVHNode* parent)
{
    std::cout << "Node " << std::hex << parent << " --> Leaf? " << (parent->nleft ? "No" : "Yes") << "\n";

    if(!parent->nleft)
    {
        std::cout << "Child left : " << std::hex << parent->left << "\n";
        std::cout << "Child right: " << std::hex << parent->right << "\n";
    }
    else
    {
        PrintBVHTree(parent->nleft);
        if(parent->nleft != parent->nright)
            PrintBVHTree(parent->nright);
    }
}
