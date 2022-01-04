#include "display.h"

#include "../../imgui/imgui.h"
#include "../../imgui/imgui_impl_glfw.h"
#include "../../imgui/imgui_impl_opengl3.h"

#include "../../glad/glad.h"
#include <GLFW/glfw3.h>

#include <iostream>

#include "overlay.h"
#include "../../thread/threadpool.h"
 
internal void error_callback(int error, const char* description)
{
    std::cerr << "Error: " << description << "\n";
}
 
internal void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

struct OpenGLInternalData
{
    GLuint rtTexture; // The raytraced image data
    GLuint w, h;
    GLuint rtProgram;
    GLuint rtVao;
    GLuint rtVbo;
};

// TODO: Refactor this to modern C++
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

GLuint CreateRTProgram()
{
    char source[4096];
    memset(source, 0, 4096);
    LoadSource("shaders/quad.vert.glsl", source);
    GLuint vertex_shader = CreateShader(GL_VERTEX_SHADER, source);

    memset(source, 0, 4096);
    LoadSource("shaders/quad.frag.glsl", source);
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

internal OpenGLInternalData InitInternal(u32 w, u32 h)
{
    OpenGLInternalData r;
    r.w = w;
    r.h = h;

    glGenTextures(1, &r.rtTexture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, r.rtTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

    const float vertices[] = {
        -1.0f, -1.0f, 0.0f, 1.0f,
        -1.0f,  1.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 1.0f, 1.0f,
         1.0f,  1.0f, 1.0f, 0.0f
    };

    glGenVertexArrays(1, &r.rtVao);
    glBindVertexArray(r.rtVao);
    glGenBuffers(1, &r.rtVbo);
    glBindBuffer(GL_ARRAY_BUFFER, r.rtVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Position
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // UV's
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    r.rtProgram = CreateRTProgram();

    return r;
}

internal void ResizeImageTex(OpenGLInternalData* r, u32 w, u32 h)
{
    // glDeleteTextures(1, &r->rtTexture);
    // glGenTextures(1, &r->rtTexture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, r->rtTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
}

internal void DestroyInternal(OpenGLInternalData data)
{
    glDeleteTextures(1, &data.rtTexture);
    glDeleteVertexArrays(1, &data.rtVao);
    glDeleteBuffers(1, &data.rtVbo);
    glDeleteProgram(data.rtProgram);
}

internal void rtTextureUpdate(OpenGLInternalData data, Image* img, std::mutex* mtx)
{
    std::lock_guard<std::mutex> lock(*mtx);
    // TODO: Maybe consider using a PBO later on for performance
    glBindTexture(GL_TEXTURE_2D, data.rtTexture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, data.w, data.h, GL_BGRA, GL_UNSIGNED_BYTE, img->data);
}

void RasterDisplay::RunGLFWWindow()
{
    GLFWwindow* window;
 
    glfwSetErrorCallback(error_callback);
 
    if (!glfwInit())
        std::cerr << "err: failed to init glfw.\n";
 
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    
    window = glfwCreateWindow(1280, 720, "Liquid", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        std::cerr << "err: failed to create glfw window.\n";
    }

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
 
    glfwSetKeyCallback(window, key_callback);
 
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glfwSwapInterval(1);

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    Overlay::RenderSettings& rs = Overlay::GetRenderSettings();
    OpenGLInternalData data = InitInternal(rs.rtImageW.load(), rs.rtImageH.load());
    u32 last_h = rs.rtImageH.load();

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        Overlay::Display();

        ImGui::Render();

        float ratio;
        int width, height;
 
        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float) height;
 
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        Image* image = Overlay::GetRenderTarget();
        if(image)
        {
            // Update, resize and draw the rt'ed image
            if(last_h != image->h)
            {
                printf("Resizing: %up -> %up\n", last_h, image->h);
                ResizeImageTex(&data, image->w, image->h);
                last_h = image->h;
            }
            rtTextureUpdate(data, image, rs.workingPool->getImage_mtx());
        }
        glUseProgram(data.rtProgram);
        glBindVertexArray(data.rtVao);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
 
        glfwSwapBuffers(window);
    }

    // TODO: Convert this from thread joins into a work cancel
    if(rs.workingPool)
        rs.workingPool->fence();

    if(rs.world.top)
        Scene::FreeScene(&rs.world);

    DestroyInternal(data);

    Overlay::DeleteRenderTarget();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
 
    glfwDestroyWindow(window);
 
    glfwTerminate();
}