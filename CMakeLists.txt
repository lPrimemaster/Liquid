cmake_minimum_required(VERSION 3.8)

project(Liquid LANGUAGES CXX)

list(APPEND CMAKE_PREFIX_PATH "C:/Program Files/GLFW/lib/cmake/glfw3")

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

find_package(glfw3 3.3 REQUIRED)

set_source_files_properties(src/glad/glad.c PROPERTIES LANGUAGE CXX)

set(IMGUI_SRC_DIR src/imgui)

add_executable(Liquid
    ${IMGUI_SRC_DIR}/imgui.cpp
    ${IMGUI_SRC_DIR}/imgui_demo.cpp
    ${IMGUI_SRC_DIR}/imgui_draw.cpp
    ${IMGUI_SRC_DIR}/imgui_tables.cpp
    ${IMGUI_SRC_DIR}/imgui_widgets.cpp
    ${IMGUI_SRC_DIR}/imgui_widgets.cpp
    ${IMGUI_SRC_DIR}/imgui_impl_glfw.cpp
    ${IMGUI_SRC_DIR}/imgui_impl_opengl3.cpp
    
    ${IMGUI_SRC_DIR}/imgui_impl_glfw.h
    ${IMGUI_SRC_DIR}/imgui_impl_opengl3.h
    ${IMGUI_SRC_DIR}/imgui.h

    src/glad/khrplatform.h
    src/glad/glad.h
    src/glad/glad.c

    src/stb/stb_image.h

    src/main.cpp
    src/common.h

    src/utils/fileloader.h

    src/image/image.h
    src/image/image.cpp

    src/math/vector.h
    src/math/transform.h
    src/math/matrix.h
    
    src/math/ray.h

    src/math/aabb.h
    src/math/aabb.cpp

    src/math/random.h
    src/math/random.cpp

    src/math/math.h
    src/math/math.cpp

    src/thread/threadpool.h
    src/thread/threadpool.cpp

    src/renderer/displayer/debug_display_win32.h
    src/renderer/displayer/debug_display_win32.cpp

    src/renderer/displayer/display.h
    src/renderer/displayer/display.cpp
    src/renderer/displayer/overlay.h
    src/renderer/displayer/overlay.cpp

    src/renderer/displayer/raster/raster.h
    src/renderer/displayer/raster/raster.cpp

    src/renderer/scene.h

    src/renderer/raycaster/geometry.h
    src/renderer/raycaster/geometry.cpp
    src/renderer/raycaster/hit_record.h

    src/renderer/raycaster/caster.h
    src/renderer/raycaster/caster.cpp

    src/renderer/raycaster/material.h
    src/renderer/raycaster/material.cpp

    src/renderer/raycaster/accelerator/bvh.h
    src/renderer/raycaster/accelerator/bvh.cpp

    src/renderer/raycaster/hittable/model.h
    src/renderer/raycaster/hittable/object.h
    src/renderer/raycaster/hittable/object.cpp

    src/renderer/samples/samples.h
    src/renderer/samples/samples.cpp
)

target_include_directories(Liquid PRIVATE ${GLFW3_INCLUDE_DIRS})
target_link_libraries(Liquid glfw)

# add_custom_command(TARGET Liquid POST_BUILD
#                    COMMAND ${CMAKE_COMMAND} -E copy_directory 
#                    ${CMAKE_SOURCE_DIR}/renderer/displayer/shaders $<TARGET_FILE_DIR:Liquid>)

# add_custom_command(TARGET Liquid POST_BUILD
#                    COMMAND ${CMAKE_COMMAND} -E copy_directory 
#                    ${CMAKE_SOURCE_DIR}/renderer/displayer/shaders/ $<TARGET_FILE_DIR:Liquid>)
