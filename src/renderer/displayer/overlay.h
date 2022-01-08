#pragma once
#include "../../common.h"
#include "../scene.h"

struct GLFWwindow;

class ThreadPool;

namespace Overlay
{
    struct OverlaySettings
    {
        bool displaySettings = false;
        bool displayRenderSave = false;
    };

    struct RenderSettings
    {
        std::atomic<i32> rtSamples = 8;
        std::atomic<i32> rtThreads = 1;

        std::atomic<i32> rtBlocksX = 8;
        std::atomic<i32> rtBlocksY = 8;

        std::atomic<u32> rtImageW = 1280;
        std::atomic<u32> rtImageH = 720;

        std::atomic<bool> rtRender = false;

        std::atomic<bool> rasterRender = false;


        ThreadPool* workingPool = nullptr;
        Scene world;
    };

    void Display(GLFWwindow* window);

    RenderSettings& GetRenderSettings();
    Image* GetRenderTarget();
    void DeleteRenderTarget();
    
}