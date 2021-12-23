#pragma once
#include <thread>
#include <atomic>
#include <mutex>
#include <vector>
#include "../common.h"
#include "../image/image.h"
#include "../renderer/camera.h"
#include "../renderer/scene.h"

struct JobContext
{
    u32 id;
    u32 numJobs;
    i32 istart, ispan;
    i32 jstart, jspan;
    Camera* cam;
    Image* img;
    Scene* world;
};

class ThreadPool
{
public:
    ThreadPool(u32 chunkSizeX, u32 chunkSizeY, Scene* world, Image* img, Camera* cam, void (*jobFunc)(JobContext* ctx, std::mutex* img_mtx));


    void run();
    void fence() const;

    inline std::mutex* getImage_mtx()
    {
        return &image_mtx;
    }

private:
    u32 chunkSizeX = 1;
    u32 chunkSizeY = 1;
    u32 istep;
    u32 jstep;
    u32 numJobs;
    std::vector<JobContext> jobs;
    std::vector<std::thread*> threads;
    std::mutex mtx;
    std::mutex image_mtx;
    std::atomic<bool> running = true;
    void (*jobFunc)(JobContext* ctx, std::mutex* img_mtx);
};