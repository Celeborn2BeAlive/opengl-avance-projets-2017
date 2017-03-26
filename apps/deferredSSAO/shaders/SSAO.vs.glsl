#version 330

layout(location = 0) in vec2 aPosition;
layout(location = 2) in vec2 aTexCoord;

out vec2 TexCoords;

void main()
{
    gl_Position =  vec4(aPosition, 0, 1);
    TexCoords = aTexCoord;
}