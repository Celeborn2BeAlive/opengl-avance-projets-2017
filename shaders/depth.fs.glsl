#version 430

uniform sampler2D uDepth;

in vec2 vTexCoords;
out vec3 fColor;

void main() {
    float depth = texture(uDepth, vTexCoords).r;
    fColor = vec3(depth);
}