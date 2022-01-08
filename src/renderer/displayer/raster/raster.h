#pragma once
#include "../../../common.h"
#include "../../scene.h"
#include "../../../glad/glad.h"
#include <GLFW/glfw3.h>

namespace Overlay
{
    struct RenderSettings;
}

// Triangle composed objects work out of the box here. 
// However custom hit functions objects need to be vertex generated for realtime manipulation.
namespace Raster
{
    void SetupRaster(GLFWwindow* window, Overlay::RenderSettings* rs);
    void CleanRaster();
    void RenderRaster(GLFWwindow* window, Scene* world, Overlay::RenderSettings* rs);
    void UpdateRasterSceneOnLoad(Scene* world);
}