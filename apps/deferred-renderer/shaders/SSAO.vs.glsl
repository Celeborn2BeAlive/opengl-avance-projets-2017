#version 330 core
layout (location = 0) in vec2 aPosition;

out vec2 TexCoords;

void main()
{
    gl_Position = vec4(aPosition, 0, 1);
}