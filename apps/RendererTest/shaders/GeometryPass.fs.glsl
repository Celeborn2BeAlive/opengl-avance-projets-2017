#version 430

in vec3 vViewSpacePosition; // Position du vertex dans View
in vec3 vViewSpaceNormal; // Normale du vertex dans View
in vec3 vViewSpaceTangent; // Tangente du vertex dans View
in vec2 vTexCoords; // Coordonnées de texture

uniform vec3 uKa;
uniform vec3 uKd;
uniform vec3 uKs;
uniform float uNs;
uniform float ud;
vec3 normal;

uniform sampler2D uKaSampler;
uniform sampler2D uKdSampler;
uniform sampler2D uKsSampler;
uniform sampler2D uNsSampler;
uniform sampler2D udSampler;
uniform sampler2D uNormalSampler;

uniform bool uKaMap;
uniform bool uKdMap;
uniform bool uKsMap;
uniform bool uNsMap;
uniform bool udMap;
uniform bool uNormalMap;


layout(location = 0) out vec3 fPosition;
layout(location = 1) out vec3 fNormal;
layout(location = 2) out vec3 fAmbient;
layout(location = 3) out vec3 fDiffuse;
layout(location = 4) out vec4 fGlossyShininess;
//layout(location = 5) out vec4 fDepth;
layout(location = 5) out float fTest;

vec3 bumpedNormal(vec4 normalTexel)
{
    vec3 normal = normalize(vViewSpaceNormal);
    vec3 tangent = normalize(vViewSpaceTangent);
    tangent = normalize(tangent - dot(tangent, normal) * normal);
    vec3 bitangent = cross(tangent, normal);

    vec3 bumpMapNormal = 2.0 * normalTexel.rgb - vec3(1.0, 1.0, 1.0);
    mat3 TBN = mat3(tangent, bitangent, normal);

    return normalize(TBN * bumpMapNormal);
}


void main()
{
    vec4 texelKa = vec4(1.f, 1.f, 1.f, 1.f);
    if( uKaMap) {
        texelKa = texture(uKaSampler, vTexCoords);
    }

    vec4 texelKd = vec4(1.f, 1.f, 1.f, 1.f);
    if( uKdMap) {
        texelKd = texture(uKdSampler, vTexCoords);
    }

    vec4 texelKs = vec4(1.f, 1.f, 1.f, 1.f);
    if( uKsMap) {
        texelKs = texture(uKsSampler, vTexCoords);
    }

    vec4 texelNs = vec4(1.f, 1.f, 1.f, 1.f);
    if( uNsMap) {
        texelNs = texture(uNsSampler, vTexCoords);
    }

    vec4 texeld = vec4(1.f, 1.f, 1.f, 1.f);
    if( udMap) {
        texeld = texture(udSampler, vTexCoords);
    }

    vec4 texelNormal = vec4(1.f, 1.f, 1.f, 1.f);
    normal = vViewSpaceNormal;
    if( uNormalMap) {
        texelNormal = texture(uNormalSampler, vTexCoords);
        normal = bumpedNormal(texelNormal);
    }
    normal = normalize(normal);

    vec3 Ka = uKa * texelKa.rgb;
    vec3 Kd = uKd * texelKd.rgb;
    vec3 Ks = uKs * texelKs.rgb;
    float Ns = uNs * texelNs.r;
    float d = ud * texeld.r;

    fPosition = vViewSpacePosition;
    //fNormal = normalize(vViewSpaceNormal);
    fNormal = normal;
    fAmbient = Ka;
    fDiffuse = Kd;
    fGlossyShininess = vec4(Ks, Ns);
    //fDepth = vec3(gl_FragCoord.z, 1.f);

    fTest = 1;
}