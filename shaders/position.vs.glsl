#version 430

layout(location = 0) in vec2 aPosition;

out vec2 vTexCoords;

void main() {
    vTexCoords = aPosition * 0.5 + 0.5;
    gl_Position = vec4(aPosition, 0, 1);
}