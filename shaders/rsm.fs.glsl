#version 430

uniform vec3 uIntensity;

uniform vec3 uKd;
uniform sampler2D uKdSampler;

in vec3 vViewSpacePosition;
in vec3 vViewSpaceNormal;
in vec2 vTexCoords;

layout(location = 0) out vec3 fPosition;
layout(location = 1) out vec3 fNormal;
layout(location = 2) out vec3 fFlux;


void main() {
    fPosition = vViewSpacePosition;
    fNormal = normalize(vViewSpaceNormal);
    fFlux = 1200000 * uIntensity * uKd * texture(uKdSampler, vTexCoords).rgb;
}
