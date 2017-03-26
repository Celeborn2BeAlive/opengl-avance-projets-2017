#version 430

uniform sampler2D uGPositionSampler;
uniform sampler2D uGNormalSampler;
uniform sampler2D uGAmbientSampler;
uniform sampler2D uGDiffuseSampler;
uniform sampler2D uGlossyShininessSampler;
uniform sampler2D uGDepthSampler;
uniform sampler2D uSSAOSampler;

uniform vec3 uDirectionalLightDir;
uniform vec3 uDirectionalLightColor;
uniform float uDirectionalLightIntensity;

uniform bool uEnablePointLights;
uniform int uNbPointLights;
layout(std430, binding = 4) buffer aPointLightPosition
{
    vec4 PointLightPositions[];
};

layout(std430, binding = 5) buffer aPointLightColor
{
    vec4 PointLightColors[];
};

layout(std430, binding = 6) buffer aPointLightIntensity
{
    float PointLightIntensities[];
};

layout(std430, binding = 7) buffer aPointLightEnabled
{
    int PointLightEnableds[];
};


out vec4 fColor;


vec3 blinnPhong(vec3 position, vec3 normal, vec3 Ka, vec3 Kd, vec3 Ks, float Ns,
                vec3 lightColor, float lightIntensity, vec3 lightDirection, float distance) {

    vec3 wi = normalize(lightDirection);
    vec3 w0 = normalize(-position);
    vec3 halfVector = normalize((wi + w0) / 2);
    vec3 N = normalize(normal);
    vec3 Li = lightColor * lightIntensity / (distance * distance);

    // Li * [(Kd * (wi.N) + Ks * (halfVector.N) ^ shininess)]

    return Li * (Ka + Kd * max(0.f, dot(wi, N)) + Ks * pow(max(0.f, dot(halfVector, N)), Ns));
}

//vec3 blinnPhong(vec3 position, vec3 normal, vec3 Ka, vec3 Kd, vec3 Ks, float Ns,
//                vec3 lightColor, float lightIntensity, vec3 lightDirection, float distance) {
//
//    vec3 eyeDir = normalize(-position);
//
//    float distToPointLight = length(lightPosition - position);
//    vec3 dirToPointLight = (lightPosition - position) / distance;
//    vec3 pointLightIncidentLight = (lightColor * lightIntensity) / (distance * distance);
//
//    // half vectors, for blinn-phong shading
//    vec3 hPointLight = normalize(eyeDir + lightDirection);
//    vec3 hDirLight = normalize(eyeDir + uDirectionalLightDir);
//
//    float dothPointLight = shininess == 0 ? 1.f : max(0.f, dot(normal, hPointLight));
//    float dothDirLight = shininess == 0 ? 1.f :max(0.f, dot(normal, hDirLight));
//
//    if (shininess != 1.f && shininess != 0.f)
//    {
//        dothPointLight = pow(dothPointLight, shininess);
//        dothDirLight = pow(dothDirLight, shininess);
//    }
//
//    vec3 contribution = Ka;
//    contribution += Kd * (uDirectionalLightIntensity * max(0.f, dot(normal, uDirectionalLightDir)) + pointLightIncidentLight * max(0., dot(normal, dirToPointLight)));
//    contribution += Ks * (pointLightIncidentLight * dothPointLight);
//
//    return contribution;
//}


void main()
{
	vec3 position = vec3(texelFetch(uGPositionSampler, ivec2(gl_FragCoord.xy), 0));
	vec3 normal = vec3(texelFetch(uGNormalSampler, ivec2(gl_FragCoord.xy), 0));
    vec3 Ka = vec3(texelFetch(uGAmbientSampler, ivec2(gl_FragCoord.xy), 0));
    vec3 Kd = vec3(texelFetch(uGDiffuseSampler, ivec2(gl_FragCoord.xy), 0));
    vec4 glossyShininess = vec4(texelFetch(uGlossyShininessSampler, ivec2(gl_FragCoord.xy), 0));
    vec3 Ks = glossyShininess.rgb;
    float Ns = glossyShininess.a;
    //float d = 1.f;
    float depth = texelFetch(uGDepthSampler, ivec2(gl_FragCoord.xy), 0).r;
    float ambientOcclusion = texelFetch(uSSAOSampler, ivec2(gl_FragCoord.xy), 0).r;

    Ka = Ka * ambientOcclusion;

    vec3 contribution = vec3(0, 0, 0);
    contribution += blinnPhong( position, normal, Ka, Kd, Ks, Ns,
                                uDirectionalLightColor, uDirectionalLightIntensity, uDirectionalLightDir, 1);

    if(uEnablePointLights)
        for(int i = 0; i < uNbPointLights; ++i) {
            if(0 == PointLightEnableds[i])
                continue;
            float distToPointLight = length(PointLightPositions[i].xyz - position);
            vec3 dirToPointLight = (PointLightPositions[i].xyz - position) / distToPointLight;
            contribution += blinnPhong( position, normal, Ka, Kd, Ks, Ns,
                                        PointLightColors[i].rgb, PointLightIntensities[i], dirToPointLight, distToPointLight);
        }

    fColor = vec4(contribution, 1.f);
    //fColor = vec4(position, 1.f);


    if(depth == 1.f)
        fColor = vec4(0, 0, 0, 0.f);
    //*/

    //fColor = vec4(ambientOcclusion, 1);
}