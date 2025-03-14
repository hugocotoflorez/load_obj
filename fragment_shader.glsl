#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D texture1; // Solo un sampler

void main()
{
    FragColor = texture(texture1, TexCoord);
}

