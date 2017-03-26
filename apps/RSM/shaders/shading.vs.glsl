#version 330

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aColor;

out vec3 vPosView;
out vec3 vNrmView;
out vec3 vLightPos;
out vec3 vColor;

out vec3 vPosWorld;
out vec3 vNrmWorld;
out vec4 vPosLightSpace;

uniform mat4 umvMat;
uniform mat4 umvpMat;
uniform vec3 ulightPos;

uniform mat4 ulightMvpMat;

void main(void) {
    gl_Position = umvpMat * vec4(aPosition, 1.0);

    vPosView = (umvMat * vec4(aPosition, 1.0)).xyz;
    vNrmView = (transpose(inverse(umvMat)) * vec4(aNormal, 1.0)).xyz;
    vLightPos = (umvMat * vec4(ulightPos, 1.0)).xyz;
    vColor   = aColor;

    vPosWorld = aPosition;
    vNrmWorld = aNormal;
    vPosLightSpace = ulightMvpMat * vec4(aPosition, 1.0);
}
