#include "shaderloader.h"
#include "../glad/glad.h"

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
        fprintf(stderr, "SHADER COMPILATION FAILED:\n%s\n", infoLog);
    }
    return shader;
}

u32 ShaderLoader::LoadSimpleShader(const std::string& name)
{
    GLuint shaderId[2];
    i32 i = 0;
    for(auto st : {".vert.glsl", ".frag.glsl"})
    {
        std::string src;
        std::ifstream sf;

        sf.open(name + st);

        if(sf)
        {
            std::stringstream srcStream;
            srcStream << sf.rdbuf();
            sf.close();

            src = srcStream.str();
            shaderId[i++] = CreateShader(GL_VERTEX_SHADER - i, src.c_str()); //VS:0x8B31 and //FS:0x8B30
        }
        else
        {
            std::cerr << "Failed to open file: " << name + st << "\n";
        }
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, shaderId[0]);
    glAttachShader(program, shaderId[1]);
    glLinkProgram(program);

    int  success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if(!success)
    {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        fprintf(stderr, "SHADER LINK FAILED:\n%s\n", infoLog);
    }

    glUseProgram(program);

    // Mark for delete
    glDeleteShader(shaderId[0]);
    glDeleteShader(shaderId[1]);
    return program;
}
