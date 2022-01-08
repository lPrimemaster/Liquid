#include "raster.h"
#include "../../raycaster/geometry.h"
#include "../overlay.h"
#include "../../../utils/shaderloader.h"
#include "../../raycaster/hittable/model.h"
#include <vector>
#include <tuple>
#include <unordered_map>

internal Vector3 rasterCamFront;
internal Vector3 rasterCamPos;
internal Vector3 rasterCamUp;
internal u32 rasterScreenW;
internal u32 rasterScreenH;

struct RasterData
{
    GLuint vao, vbo;
    u64 vtxCount;
};

internal std::vector<RasterData> rasterDataArray[Geometry::TYPESIZE];

internal GLuint program;
internal std::vector<Vector3> rasterRandomColors;

internal void CreateRasterSphere(i32 ico_it)
{
    std::vector<Vector3> vertices;
    std::vector<i32> indices;
    f32 t = (1.0f + sqrtf(5.0f)) / 2.0f;

    f32 len = sqrtf(1 + t*t);

    f32 a = 1 / len;
    f32 b = t / len;

    // Create initial vertices
    vertices.push_back(Vector3(-a,  b,  0));
    vertices.push_back(Vector3( a,  b,  0));
    vertices.push_back(Vector3(-a, -b,  0));
    vertices.push_back(Vector3( a, -b,  0));

    vertices.push_back(Vector3( 0, -a,  b));
    vertices.push_back(Vector3( 0,  a,  b));
    vertices.push_back(Vector3( 0, -a, -b));
    vertices.push_back(Vector3( 0,  a, -b));

    vertices.push_back(Vector3( b,  0, -a));
    vertices.push_back(Vector3( b,  0,  a));
    vertices.push_back(Vector3(-b,  0, -a));
    vertices.push_back(Vector3(-b,  0,  a));

    // Create initial faces / tris
    using FaceVec = std::vector<std::tuple<i32, i32, i32>>;
    FaceVec faces;

    // 5 faces around point 0
    faces.push_back(std::make_tuple(0, 11, 5));
    faces.push_back(std::make_tuple(0, 5, 1));
    faces.push_back(std::make_tuple(0, 1, 7));
    faces.push_back(std::make_tuple(0, 7, 10));
    faces.push_back(std::make_tuple(0, 10, 11));

    // 5 adjacent faces 
    faces.push_back(std::make_tuple(1, 5, 9));
    faces.push_back(std::make_tuple(5, 11, 4));
    faces.push_back(std::make_tuple(11, 10, 2));
    faces.push_back(std::make_tuple(10, 7, 6));
    faces.push_back(std::make_tuple(7, 1, 8));

    // 5 faces around point 3
    faces.push_back(std::make_tuple(3, 9, 4));
    faces.push_back(std::make_tuple(3, 4, 2));
    faces.push_back(std::make_tuple(3, 2, 6));
    faces.push_back(std::make_tuple(3, 6, 8));
    faces.push_back(std::make_tuple(3, 8, 9));

    // 5 adjacent faces 
    faces.push_back(std::make_tuple(4, 9, 5));
    faces.push_back(std::make_tuple(2, 4, 11));
    faces.push_back(std::make_tuple(6, 2, 10));
    faces.push_back(std::make_tuple(8, 6, 7));
    faces.push_back(std::make_tuple(9, 8, 1));

    std::unordered_map<i64, i32> middlePointIndexCache;

    auto getMiddlePoint = [&](i32 p1, i32 p2) -> i32
    {
        // first check if we have it already
        bool firstIsSmaller = p1 < p2;
        i64 smallerIndex = firstIsSmaller ? p1 : p2;
        i64 greaterIndex = firstIsSmaller ? p2 : p1;
        i64 key = (smallerIndex << 32) + greaterIndex;

        if(middlePointIndexCache.find(key) != middlePointIndexCache.end())
        {
            return middlePointIndexCache.at(key);
        }

        // not in cache, calculate it
        Vector3 point1 = vertices[p1];
        Vector3 point2 = vertices[p2];
        Vector3 middle = Vector3(
            (point1.x + point2.x) / 2.0f, 
            (point1.y + point2.y) / 2.0f, 
            (point1.z + point2.z) / 2.0f
        );

        vertices.push_back(middle.normalized());
        i32 i = (i32)vertices.size() - 1;
        middlePointIndexCache.emplace(key, i);
        return i;
    };

    // refine triangles with recursion
    for (i32 i = 0; i < ico_it; i++)
    {
        FaceVec facesR;
        for(auto t : faces)
        {
            i32 v1 = std::get<0>(t);
            i32 v2 = std::get<1>(t);
            i32 v3 = std::get<2>(t);

            // replace triangle by 4 triangles
            i32 a = getMiddlePoint(v1, v2);
            i32 b = getMiddlePoint(v2, v3);
            i32 c = getMiddlePoint(v3, v1);

            facesR.push_back(std::make_tuple(v1, a, c));
            facesR.push_back(std::make_tuple(v2, b, a));
            facesR.push_back(std::make_tuple(v3, c, b));
            facesR.push_back(std::make_tuple(a, b, c));
        }
        faces = facesR;
    }

    std::vector<f32> rawVertexData;
    // done we dont use EBO for this
    for(auto t : faces)
    {
        Vector3 v1 = vertices[std::get<0>(t)];
        Vector3 v2 = vertices[std::get<1>(t)];
        Vector3 v3 = vertices[std::get<2>(t)];

        // flat
        // Vector3 w = v2 - v1;
        // Vector3 u = v3 - v1;
        // Vector3 N = Vector3::Cross(w, v);

        // smooth
        Vector3 n1 = v1;
        Vector3 n2 = v2;
        Vector3 n3 = v3;

        rawVertexData.push_back(v1.x);
        rawVertexData.push_back(v1.y);
        rawVertexData.push_back(v1.z);

        rawVertexData.push_back(n1.x);
        rawVertexData.push_back(n1.y);
        rawVertexData.push_back(n1.z);

        rawVertexData.push_back(v2.x);
        rawVertexData.push_back(v2.y);
        rawVertexData.push_back(v2.z);

        rawVertexData.push_back(n2.x);
        rawVertexData.push_back(n2.y);
        rawVertexData.push_back(n2.z);

        rawVertexData.push_back(v3.x);
        rawVertexData.push_back(v3.y);
        rawVertexData.push_back(v3.z);

        rawVertexData.push_back(n3.x);
        rawVertexData.push_back(n3.y);
        rawVertexData.push_back(n3.z);
    }


    // Now fill the opengl buffer
    RasterData data;
    data.vtxCount = rawVertexData.size() / 2;
    glGenVertexArrays(1, &data.vao);
    glGenBuffers(1, &data.vbo);
    glBindVertexArray(data.vao);

    glBindBuffer(GL_ARRAY_BUFFER, data.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(f32) * rawVertexData.size(), rawVertexData.data(), GL_STATIC_DRAW);

    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(f32), (void*)0);
    glEnableVertexAttribArray(0);

    // Normals
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(f32), (void*)(3 *  sizeof(f32)));
    glEnableVertexAttribArray(1);

    rasterDataArray[Geometry::SPHERE].push_back(data);
}

internal void CreateRasterCube()
{
    RasterData& data = rasterDataArray[0].at(0);
    glGenVertexArrays(1, &data.vao);
    glGenBuffers(1, &data.vbo);
    glBindVertexArray(data.vao);

    f32 vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, // 0
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, // 1
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, // 2
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, // 0
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, // 1
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, // 2

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, // 0
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, // 1
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, // 2
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, // 0
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, // 1
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, // 2

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, // 0
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f, // 1
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, // 2
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, // 0
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f, // 1
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, // 2

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, // 0
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f, // 1
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f, // 2
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f, // 0
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f, // 1
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, // 2

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, // 0
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, // 1
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, // 2
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, // 0
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, // 1
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, // 2

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, // 0
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, // 1
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, // 2
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, // 0
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, // 1
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f  // 2
    };

    glBindBuffer(GL_ARRAY_BUFFER, data.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(f32), (void*)0);
    glEnableVertexAttribArray(0);

    // Normals
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(f32), (void*)(3 *  sizeof(f32)));
    glEnableVertexAttribArray(1);
}

// TODO : Refactor this for general callbacks for both raster and rt with switch based on options
internal void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if(glfwGetMouseButton(window, 1) != GLFW_PRESS) return;

    static const f32 D2R = PI / 180.0f;
    static f32 pitch = 0;
    static f32 yaw = 0;
    f32 xoffset = (f32)xpos - rasterScreenW / 2;
    f32 yoffset = rasterScreenH / 2 - (f32)ypos;

    glfwSetCursorPos(window, rasterScreenW / 2, rasterScreenH / 2);

    static const f32 sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw   += xoffset;
    pitch += yoffset;

    if(pitch > 89.0f)
        pitch = 89.0f;
    if(pitch < -89.0f)
        pitch = -89.0f;

    Vector3 direction;
    direction.x = cosf(D2R * yaw) * cosf(D2R * pitch);
    direction.y = sinf(D2R * pitch);
    direction.z = sinf(D2R * yaw) * cosf(D2R * pitch);
    rasterCamFront = direction.normalized();
}

internal void mousebtn_callback(GLFWwindow* window, int button, int action, int mods)
{
    if(button == 1 && action == GLFW_RELEASE)
    {
        // Update rt camera for current view
        // TODO: Make this an option instead in the future
        Scene* iworld = (Scene*)glfwGetWindowUserPointer(window);
        iworld->renderCamera->updateView(rasterCamPos, rasterCamPos + rasterCamFront);
    }
}

void Raster::SetupRaster(GLFWwindow* window, Overlay::RenderSettings* rs)
{
    // TODO: Remove hardcode - process viewport dim change
    rasterScreenW = 1280;
    rasterScreenH = 720;
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mousebtn_callback);
    program = ShaderLoader::LoadSimpleShader("shaders/raster");

    // Create default internal rendering models (cube and sphere)
    // CreateRasterCube();
    CreateRasterSphere(5);
}

void Raster::CleanRaster()
{
    for(i32 i = 0; i < Geometry::TYPESIZE; i++)
    {
        for(auto& v : rasterDataArray[i])
        {
            glDeleteVertexArrays(1, &v.vao);
            glDeleteBuffers(1, &v.vbo);
        }
        rasterDataArray[i].clear();
    }
    glDeleteProgram(program);
}


internal Vector3 ProcessInputIncrementFPS(GLFWwindow *window, Vector3& camFront, Vector3& camUp)
{
    Vector3 R;
    if(glfwGetMouseButton(window, 1) != GLFW_PRESS) return R; // Return zero vec if no right click
    static const float cameraSpeed = 0.05f; // adjust accordingly
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        R = R + cameraSpeed * camFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        R = R - cameraSpeed * camFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        R = R - Vector3::Cross(camFront, camUp).normalized() * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        R = R + Vector3::Cross(camFront, camUp).normalized() * cameraSpeed;
    return R;
}

internal Matrix4 UpdateCamera(GLFWwindow* window, Scene* world, Overlay::RenderSettings* rs)
{
    rasterCamPos = rasterCamPos + ProcessInputIncrementFPS(window, rasterCamFront, rasterCamUp);
    return Matrix4::LookAt(rasterCamPos, rasterCamPos + rasterCamFront, rasterCamUp);
}

void Raster::UpdateRasterSceneOnLoad(Scene* world)
{
    rasterCamPos = world->renderCamera->origin;
    rasterCamFront = world->renderCamera->direction;
    rasterCamUp = world->renderCamera->up;
}

void Raster::RenderRaster(GLFWwindow* window, Scene* world, Overlay::RenderSettings* rs)
{
    if(world->top == nullptr || !rs->rasterRender.load()) return;

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glClearDepth(1.0);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    Matrix4 P = world->renderCamera->projection;

    Matrix4 V = UpdateCamera(window, world, rs);

    glUseProgram(program);
    glUniformMatrix4fv(
        glGetUniformLocation(program, "projection"), 1, GL_FALSE,
        &P.data[0][0]
    );

    glUniformMatrix4fv(
        glGetUniformLocation(program, "view"), 1, GL_FALSE,
        &V.data[0][0]
    );

    // Display a cube for each obj in scene
    i32 i = 0;
    for(auto o : world->objList)
    {
        if(i >= rasterRandomColors.size())
        {
            rasterRandomColors.push_back(Vector3(
                Random::RandomF32Range(0, 1),
                Random::RandomF32Range(0, 1),
                Random::RandomF32Range(0, 1)
            ));
        }
        // FIX: Redo the bvh's for transform coordinates ....
        Matrix4 M = o->transform.tmatrix;
        glUniformMatrix4fv(
            glGetUniformLocation(program, "model"), 1, GL_FALSE,
            &M.data[0][0]
        );

        glUniform3fv(
            glGetUniformLocation(program, "objectColor"), 1,
                rasterRandomColors[i++].data
        );

        RasterData& data = rasterDataArray[o->model->mesh->type].at(0);
        glBindVertexArray(data.vao);
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)data.vtxCount);
    }

    // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_DEPTH_TEST);
}
