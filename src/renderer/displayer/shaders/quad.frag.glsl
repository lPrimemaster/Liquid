#version 330 core
uniform sampler2D tex;
out vec4 FragColor;
in vec2 TexCoord;

void main()
{
   FragColor = texture(tex, TexCoord);
}
