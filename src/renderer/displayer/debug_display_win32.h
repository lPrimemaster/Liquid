#pragma once
#include <mutex>
struct Image;

int run_window(Image* img, std::mutex* mtx);