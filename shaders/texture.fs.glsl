#version 430

uniform sampler2D uTexture;

in vec2 vTexCoords;
out vec3 fColor;

void main() {
    fColor = texture(uTexture, vTexCoords).rgb;
}