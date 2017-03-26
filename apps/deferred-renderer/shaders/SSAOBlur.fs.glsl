#version 330 core
out vec3 fragColor;

uniform sampler2D ssaoInput;

void main() {
    vec2 texelSize = 1.0 / vec2(textureSize(ssaoInput, 0));
    float result = 0.0;
    for (int x = -2; x < 2; ++x) 
    {
        for (int y = -2; y < 2; ++y) 
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += vec3(texelFetch(ssaoInput, ivec2(gl_FragCoord.xy + offset.xy), 0)).r;
        }
    }
    result = result / (16.0);
    fragColor = vec3(result, result, result);
} 