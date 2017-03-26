#version 430

uniform float uAdjustment;

uniform sampler2D uGDepthSampler;

out vec3 fColor;

void main()
{
    float depth = texelFetch(uGDepthSampler, ivec2(gl_FragCoord.xy), 0).r;
    fColor = vec3(pow(depth, uAdjustment)); // Since the depth is between 0 and 1, pow it to darkness its value
}