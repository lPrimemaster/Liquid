#include "bvh.h"
#include "../../../math/random.h"
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

internal bool BoxCompare(Object* o0, Object* o1, i32 axis)
{
    AABB box0 = o0->getAABB(o0);
    AABB box1 = o1->getAABB(o1);

    return box0.min.data[axis] < box1.min.data[axis];
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
        parent->left = parent->right = objects[0];
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
        std::sort(sorted.begin(), sorted.end(), RandomBoxCompare);

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

void BVHNode::FreeBVHTree(BVHNode* parent)
{
    if(parent->nleft != nullptr)
        FreeBVHTree(parent->nleft);
    if(parent->nright != nullptr)
        FreeBVHTree(parent->nright);

    delete parent;
}