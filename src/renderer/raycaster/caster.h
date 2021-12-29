#pragma once
#include "hittable/object.h"
#include "../scene.h"

Vector3 RayCast(const Ray* r, Scene* world, i32 depth);
