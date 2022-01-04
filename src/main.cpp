#include "renderer/raycaster/hittable/object.h"
#include "renderer/raycaster/geometry.h"
#include "renderer/raycaster/hittable/object.h"
#include "renderer/raycaster/accelerator/bvh.h"
#include "renderer/scene.h"
#include "image/image.h"
#include "math/random.h"
#include "renderer/camera.h"
#include "renderer/raycaster/caster.h"
#include "renderer/raycaster/material.h"
#include "thread/threadpool.h"

// TODO: These sources are temporary
#include "renderer/displayer/debug_display_win32.h"
#include "renderer/displayer/display.h"

int main()
{
#ifdef _WIN32WINDOW_DEBUG
    run_window(&image, pool.getImage_mtx());
#else
    RasterDisplay::RunGLFWWindow();
#endif _WIN32WINDOW_DEBUG

    Object::DeleteAll();
    Material::UnloadAll();
    Geometry::UnloadAll();
    return 0;
}
