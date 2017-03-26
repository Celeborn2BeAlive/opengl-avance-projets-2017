#version 430

uniform sampler2D uGPositionSampler;
uniform sampler2D uGNormalSampler;
uniform sampler2D uNoiseSampler;

uniform vec3 uKernel[64];
uniform mat4 uProjMatrix;

uniform vec2 uFramebufferSize;
uniform vec2 uNoiseTileSize;

uniform float uRadius;
uniform float uBias;

layout(location = 0) out float fColor;

// tile noise texture over screen based on screen dimensions divided by noise size
const vec2 noiseScale = vec2(uFramebufferSize.x / uNoiseTileSize.x, uFramebufferSize.y / uNoiseTileSize.y);

void main()
{
    vec3 position = vec3(texelFetch(uGPositionSampler, ivec2(gl_FragCoord.xy), 0));
    vec3 normal = vec3(texelFetch(uGNormalSampler, ivec2(gl_FragCoord.xy), 0));
    //vec3 noise = vec3(texelFetch(uNoiseSampler, ivec2(gl_FragCoord.xy * noiseScale), 0));
	vec3 noise = vec3(1.f, 1.f, 1.f);
	// TODO Properly send and read the noise texture instead of cheating

    vec3 tangent = normalize(noise - normal * dot(noise, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);


    const int kernelSize = 64;
    float radius = uRadius;
    float bias = uBias;

    float occlusion = 0.0f;
    for(int i = 0; i < kernelSize; ++i)
    {
        // sample position in viewspace
        vec3 sampled = TBN * uKernel[i];
        sampled = position + sampled * radius;

		// send sampled postion to clip-space
		vec4 offset = vec4(sampled, 1.0f);
        offset      = uProjMatrix * offset;    // from view to clip-space
        offset.xyz /= offset.w;               // perspective divide
        offset.xyz  = offset.xyz * 0.5f + 0.5f; // transform to range 0.0 - 1.0

		float sampledDepth = texture(uGPositionSampler, offset.xy).z;
		//float sampledDepth = vec3(texelFetch(uGPositionSampler, ivec2(gl_FragCoord.xy), 0)).z;

        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(position.z - sampledDepth));
        occlusion       += (sampledDepth >= sampled.z + bias ? 1.0 : 0.0) * rangeCheck;
        //occlusion       += (sampledDepth >= sampled.z + bias ? 1.0 : 0.0);
    }

	occlusion = 1.0f - (occlusion / kernelSize);

    fColor = occlusion;
}