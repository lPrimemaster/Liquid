#include "raster.h"
#include "../../raycaster/geometry.h"
#include "../overlay.h"

// FIX: This is an exact copy from display.cpp
internal void LoadSource(const char* filename, char* buffer)
{
    FILE* f = fopen(filename, "r");
    char line[512];
    while(fgets(line, 512, f))
    {
        strcat(buffer, line);
    }
    fclose(f);
}

internal GLuint CreateShader(i32 shader_type, const char* source)
{
    GLuint shader = glCreateShader(shader_type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    int  success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if(!success)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        printf("SHADER COMPILATION_FAILED\n%s\n", infoLog);
    }
    return shader;
}

internal GLuint CreateRasterProgram()
{
    char source[4096];
    memset(source, 0, 4096);
    LoadSource("shaders/raster.vert.glsl", source);
    GLuint vertex_shader = CreateShader(GL_VERTEX_SHADER, source);

    memset(source, 0, 4096);
    LoadSource("shaders/raster.frag.glsl", source);
    GLuint fragment_shader = CreateShader(GL_FRAGMENT_SHADER, source);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    int  success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if(!success)
    {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        printf("SHADER LINK_FAILED\n%s\n", infoLog);
    }

    glUseProgram(program);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return program;
}

internal GLuint vao, vbo, program;

internal void CreateRasterCube()
{
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);

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

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(f32), (void*)0);
    glEnableVertexAttribArray(0);

    // Normals
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(f32), (void*)(3 *  sizeof(f32)));
    glEnableVertexAttribArray(1);
}

void Raster::SetupRaster()
{
    program = CreateRasterProgram();
    CreateRasterCube();
}

void Raster::CleanRaster()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(program);
}

void Raster::RenderRaster(GLFWwindow* window, Scene* world, Overlay::RenderSettings* rs)
{
    if(world->top == nullptr || !rs->rasterRender.load()) return;

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glClearDepth(1.0);

    Matrix4 P0 = Matrix4::Projection(45.0f, 16.0f / 9.0f, 0.1f, 100.0f);
    Matrix4 V0 = Matrix4::LookAt(Vector3(-2, 1.0f, 5), Vector3(0, 0, 0), Vector3(0, 1, 0));
    
    Matrix4 P = world->renderCamera->projection;
    Matrix4 V = world->renderCamera->staticView;

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
    for(auto o : world->objList)
    {
        // FIX: Redo the bvh's for transform coordinates ....
        Matrix4 M = o->transform.tmatrix;
        glUniformMatrix4fv(
            glGetUniformLocation(program, "model"), 1, GL_FALSE,
            &M.data[0][0]
        );

        glUniform3f(
            glGetUniformLocation(program, "objectColor"),
                1, 0, 0
        );

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    glDisable(GL_DEPTH_TEST);
}