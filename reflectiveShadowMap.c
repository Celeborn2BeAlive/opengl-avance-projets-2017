float3 ReflectiveShadowMapping(float3 P, bool divideByW, float3 N)
{
 float4 texturePosition = mul(lightViewProjectionTextureMatrix, float4(P, 1.0));//lightViewProjectionTextureMatrix it is the biasMatrix in the compiled project (main.c)
 if (divideByW) texturePosition.xyz /= texturePosition.w;
 
float3 indirectIllumination = float3(0, 0, 0);
 float rMax = rsmRMax;
 
for (uint i = 0; i < rsmSampleCount; ++i)
 {
 float2 rnd = rsmSamples[i].xy;
 
 float2 coords = texturePosition.xy + rMax * rnd;
 //light position and its normal
float3 lPosition = g_rsmPositionWsMap.Sample(g_clampedSampler, coords.xy).xyz;
 float3 lNormal = g_rsmNormalWsMap.Sample(g_clampedSampler, coords.xy).xyz;
 float3 flux = g_rsmFluxMap.Sample(g_clampedSampler, coords.xy).xyz;
 
float3 result = flux* ((max(0, dot(lNormal, P - lPosition))* max(0, dot(N, lPosition - P))) / pow(length(P - lPosition), 4));
 
result *= rnd.x * rnd.x;
 indirectIllumination += result;
 }
 return saturate(indirectIllumination * rsmIntensity);
}
