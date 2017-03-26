#version 430

layout(location = 0) in vec3 aPosition;
uniform mat4 uLightViewProjMatrix;

void main() {
    gl_Position = uLightViewProjMatrix * vec4(aPosition, 1); 
}