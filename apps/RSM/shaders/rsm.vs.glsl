#version 330

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aColor;

out vec3 vposition;
out vec3 vnormal;
out vec4 vcolor;

void main(void) {
    vposition = aPosition;
    vnormal = aNormal;
    vcolor = vec4(aColor,1.0);
}
