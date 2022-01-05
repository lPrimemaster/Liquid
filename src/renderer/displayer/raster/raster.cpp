#include "raster.h"
#include "../../raycaster/geometry.h"

struct RasterVertex
{
    f32 vertex[3];
    f32 normal[3];
    f32 texcrd[2];
};

struct RasterMesh
{
    RasterVertex* vertices;
    u64 vertexCount;
    u32* indices;
    u64 indexCount;
};

struct RasterData
{
    // Vertex info
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    
    // For loaded data only
    RasterMesh data;

    // Set to true if the data is self owned (for raytrace primitives only, e.g. spheres)
    bool native = false;
};

internal std::vector<RasterData> rasterData;
internal GLuint rasterProgram;

internal RasterMesh rtToRasterMesh(TriangleMesh* m)
{
    RasterVertex* rm = new RasterVertex[m->vertexCount];
    for(i32 i = 0; i < m->vertexCount; i++)
    {
        rm[i].vertex[0] = m->vertices[i].x;
        rm[i].vertex[1] = m->vertices[i].y;
        rm[i].vertex[2] = m->vertices[i].z;

        rm[i].normal[0] = m->normals[i].x;
        rm[i].normal[1] = m->normals[i].y;
        rm[i].normal[2] = m->normals[i].z;

        rm[i].texcrd[0] = m->texCoords[i].u;
        rm[i].texcrd[1] = m->texCoords[i].v;
    }

    u32* indices = new u32[m->triangleCount * 3];
    for(i32 i = 0; i < m->triangleCount; i++)
    {
        indices[i * 3    ] = (u32)m->triangles[i].indicesVertex[0];
        indices[i * 3 + 1] = (u32)m->triangles[i].indicesVertex[1];
        indices[i * 3 + 2] = (u32)m->triangles[i].indicesVertex[2];
    }

    RasterMesh r;
    r.vertices = rm;
    r.indices = indices;
    r.vertexCount = m->vertexCount;
    r.indexCount = m->triangleCount * 3;
    return r;
}

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

internal void CreateRasterSphere()
{
    // Setup sphere RasterData
    RasterData sphere;
    sphere.native = true;

    // We use uvspheres for raster rendering
    // Loading from static obj files for now
    TriangleMesh* rtMesh = TriangleMesh::CreateMeshFromFile("internal_uvsphere.obj");
    sphere.data = rtToRasterMesh(rtMesh);
    delete rtMesh;

    glGenVertexArrays(1, &sphere.vao);
    glBindVertexArray(sphere.vao);
    glGenBuffers(1, &sphere.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, sphere.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(RasterVertex) * sphere.data.vertexCount, sphere.data.vertices, GL_STATIC_DRAW);

    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(RasterVertex), (void*)0);
    glEnableVertexAttribArray(0);

    // Normals
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(RasterVertex), (void*)(3 * sizeof(f32)));
    glEnableVertexAttribArray(1);

    // UV's
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(RasterVertex), (void*)(6 * sizeof(f32)));
    glEnableVertexAttribArray(2);

    glGenBuffers(1, &sphere.ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphere.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphere.data.indexCount * sizeof(u32), sphere.data.indices, GL_STATIC_DRAW);
    rasterData.push_back(sphere);
}

void Raster::SetupRaster()
{
    CreateRasterSphere();
    rasterProgram = CreateRasterProgram();
}

void Raster::CleanRaster()
{
    for(auto d : rasterData)
    {
        glDeleteVertexArrays(1, &d.vao);
        glDeleteBuffers(1, &d.vbo);
        glDeleteBuffers(1, &d.ebo);

        delete[] d.data.indices;
        delete[] d.data.vertices;
    }

    glDeleteProgram(rasterProgram);
}

void Raster::RenderRaster(GLFWwindow* window, Scene* world)
{
    // No scene, skip rendering
    if(!world->top) return;

    // Get objects in the scene and render them
    glUseProgram(rasterProgram);
    glUniformMatrix4fv(
            glGetUniformLocation(rasterProgram, "view"),
            1, GL_FALSE, &world->renderCamera->staticView.data[0][0]
    );

    glUniformMatrix4fv(
            glGetUniformLocation(rasterProgram, "project"),
            1, GL_FALSE, &world->renderCamera->projection.data[0][0]
    );

    glUniform3f(
            glGetUniformLocation(rasterProgram, "lightColor"),
            1, 1, 1
    );

    glUniform3f(
            glGetUniformLocation(rasterProgram, "lightDir"),
            1, 1, 1
    );

    glUniform3fv(
            glGetUniformLocation(rasterProgram, "viewPos"),
            1,
            world->renderCamera->origin.data
    );

    for(auto o : world->objList)
    {
        glUniformMatrix4fv(
            glGetUniformLocation(rasterProgram, "model"),
            1, GL_FALSE, &o->transform.tmatrix.data[0][0]
        );

        glUniform3f(
            glGetUniformLocation(rasterProgram, "objectColor"),
            1, 1, 1
        );

        if(o->rasterData)
        {
            glBindVertexArray(o->rasterData->vao);
            glDrawElements(GL_TRIANGLES, (GLsizei)(o->rasterData->data.indexCount), GL_UNSIGNED_INT, nullptr);
        }
        else
        {
            // TODO: One could display just a pink error cube or smt with the obj's size
            // TODO: Contruct obj raster data elsewhere
            o->rasterData = &rasterData[0]; // Admiting spheres
        }
    }

    // Render all the raster objects to texture

}