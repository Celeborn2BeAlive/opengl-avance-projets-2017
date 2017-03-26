#version 330 core
out vec3 FragColor;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;

uniform vec3 uSSAOGSceneSize;
uniform vec3 samples[64];

// parameters (you'd probably want to use them as uniforms to more easily tweak the effect)
int kernelSize = 64;
float radius = 0.5;
float bias = 0.025;

// tile noise texture over screen based on screen dimensions divided by noise size
const vec2 noiseScale = vec2(1280.0/4.0, 720.0/4.0); 

uniform mat4 projection;

void main()
{
    // Get input for SSAO algorithm
    vec3 fragPos = vec3(texelFetch(gPosition, ivec2(gl_FragCoord.xy), 0))  / uSSAOGSceneSize;
    vec3 normal =  vec3(texelFetch(gNormal, ivec2(gl_FragCoord.xy), 0));
    ivec2 randomVecTile = ivec2(gl_FragCoord.xy/4);
	vec3 randomVec = vec3(normalize(texelFetch(texNoise, ivec2(gl_FragCoord.xy - randomVecTile * 4.0), 0)));
    // Create TBN change-of-basis matrix: from tangent-space to view-space
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);
    // Iterate over the sample kernel and calculate occlusion factor
    float occlusion = 0.0;
    for(int i = 0; i < kernelSize; ++i)
    {
        // get sample position
        vec3 sample = TBN * samples[i]; // From tangent to view-space
        sample = fragPos + sample * radius; 
        
        // project sample position (to sample texture) (to get position on screen/texture)
        vec4 offset = vec4(sample, 1.0);
        offset = projection * offset; // from view to clip-space
        offset.xyz /= offset.w; // perspective divide
        offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0
        
        // get sample depth
        float sampleDepth = vec3(texelFetch(gPosition, ivec2(offset.xy), 0)).z;

        // range check & accumulate
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
        occlusion += (sampleDepth >= sample.z + bias ? 1.0 : 0.0) * rangeCheck;           
    }
    occlusion = 1.0 - (occlusion / kernelSize);
    occlusion = pow(occlusion, 10);
    
    FragColor = vec3(occlusion, occlusion, occlusion);;
}