#pragma once
#include "hittable/object.h"
#include "../scene.h"

#include <mutex>

struct JobContext;

Vector3 RayCast(const Ray* r, Scene* world, i32 depth);

void calculateChunk(JobContext* ctx, std::mutex* img_mtx);