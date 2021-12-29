cmake_minimum_required(VERSION 3.8)

project(Liquid CXX)

list(APPEND CMAKE_PREFIX_PATH "C:/Program Files/GLFW/lib/cmake/glfw3")

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

find_package(glfw3 3.3 REQUIRED)

include_directories(Liquid ${GLFW3_INCLUDE_DIRS})

add_executable(Liquid
    src/glad/khrplatform.h
    src/glad/glad.c
    src/glad/glad.h

    src/stb/stb_image.h

    src/main.cpp
    src/common.h

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
)

target_link_libraries(Liquid glfw)

# add_custom_command(TARGET Liquid POST_BUILD
#                    COMMAND ${CMAKE_COMMAND} -E copy_directory 
#                    ${CMAKE_SOURCE_DIR}/shaders/ $<TARGET_FILE_DIR:Raytracer>/shaders)

# add_custom_command(TARGET Liquid POST_BUILD
#                    COMMAND ${CMAKE_COMMAND} -E copy_directory 
#                    ${CMAKE_SOURCE_DIR}/shaders/ $<TARGET_FILE_DIR:Raytracer>/../shaders)
