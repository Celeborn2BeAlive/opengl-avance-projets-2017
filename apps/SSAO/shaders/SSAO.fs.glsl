#version 330
#define KERNEL_SIZE 64

out float fColor;

uniform sampler2D uGPosition;
uniform sampler2D uGNormal;
uniform sampler2D uTexNoise;

uniform mat4 uProjMatrix;
uniform ivec2 uScreenSize;
uniform float uRadius;
uniform float uBias;

layout(std140) uniform uSamples
{
	vec3 samples[KERNEL_SIZE];
};

const vec2 noiseScale = vec2(1280.0/4.0, 720.0/4.0); 

void main()
{
	//Tile noise texture over screen based on screen dimensions divided by noise size
	ivec2 noiseTiles = ivec2(uScreenSize.x/4, uScreenSize.y/4);
	ivec2 randomVecTile = ivec2(gl_FragCoord.xy/4);

	vec3 fragPos = vec3(texelFetch(uGPosition, ivec2(gl_FragCoord.xy), 0));
	vec3 normal =  vec3(texelFetch(uGNormal, ivec2(gl_FragCoord.xy), 0));
	vec3 randomVec = vec3(texelFetch(uTexNoise, ivec2(gl_FragCoord.xy - randomVecTile * 4.0), 0)); 
	vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN = mat3(tangent, bitangent, normal);  
	
	float occlusion = 0.0;
	for(int i = 0; i < KERNEL_SIZE; ++i)
	{
		// get sample position
		vec3 sample = TBN * samples[i]; //From tangent to view-space
		sample = fragPos + sample * uRadius; 
    
		vec4 offset = vec4(sample, 1.0);
		offset = uProjMatrix * offset; //From view to clip-space
		offset.xyz /= offset.w; //Perspective divide
		offset.xyz  = offset.xyz * 0.5 + 0.5; //Transform to range 0.0 - 1.0  

		float sampleDepth = texture(uGPosition, offset.xy).z; 
		occlusion += (sampleDepth >= sample.z + uBias ? 1.0 : 0.0);  

		float rangeCheck = smoothstep(0.0, 1.0, uRadius / abs(fragPos.z - sampleDepth));
		occlusion += (sampleDepth >= sample.z + uBias ? 1.0 : 0.0) * rangeCheck;    
	}  
	fColor =  1.0 - (occlusion / KERNEL_SIZE); 
}
