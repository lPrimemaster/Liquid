#pragma once
#include "../../common.h"

#include <atomic>

struct Scene;

namespace Samples
{
    Scene BasicSphere(std::atomic<i32>* progress);
    Scene SingleSphere(std::atomic<i32>* progress);
    Scene ColoredSpheres(std::atomic<i32>* progress);
}
