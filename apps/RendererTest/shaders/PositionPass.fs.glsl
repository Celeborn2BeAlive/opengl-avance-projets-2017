#version 430

uniform vec3 uAdjustment; // Scene size in view space (coordinate of top right view frustum corner)
uniform sampler2D uGPositionSampler;

out vec3 fColor;

void main()
{
    fColor = vec3(texelFetch(uGPositionSampler, ivec2(gl_FragCoord.xy), 0)) / uAdjustment * 10; // Since the depth is between 0 and 1, pow it to darkness its value
}