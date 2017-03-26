#version 330

in vec3 vPosView;
in vec3 vNrmView;
in vec3 vLightPos;
in vec3 vColor;

in vec3 vPosWorld;
in vec3 vNrmWorld;

in vec4 vPosLightSpace;

out vec3 fColor;

uniform sampler1D urandMap;
uniform sampler2D udepthMap;
uniform sampler2D upositionMap;
uniform sampler2D unormalMap;
uniform sampler2D udiffuseMap;



void main(void) {
    // INDIRECT IRRADIANCE : E = FLUX*max(0,dot(normal,x-xp))*max(0,dot(normal,xp-x))/((x-xp)⁴)

    vec3 indirect = vec3(0.0, 0.0, 0.0);
    vec2 texCoordLightSpace = vPosLightSpace.xy / vPosLightSpace.w * 0.5 + 0.5;

    //SELECT n RANDOM POINTS WHICH WILL REPRESENT x : SEE SAMPLING PATTERN EXAMPLE on http://www.klayge.org/material/3_12/GI/rsm.pdf
    for (int i = 0; i < 64; i++) {

        float u1 = 2.0 * 4*atan(1.0) * texture(urandMap, 0.5 * (i * 2 + 1) / 64).x;
        float u2 = 0.5 * texture(urandMap, 0.5 * (i * 2) / 64).x;
        vec2 offset = u2 * vec2(cos(u1), sin(u1));
        vec2 texCoord = texCoordLightSpace + offset;

        vec3 pos = texture(upositionMap, texCoord).xyz;
        vec3 nrm = normalize(texture(unormalMap, texCoord).xyz);
        vec3 diff = texture(udiffuseMap, texCoord).rgb;

        float dot1 = max(0.0, dot(pos - vPosWorld, normalize(vNrmWorld)));
        float dot2 = max(0.0, -dot(vPosWorld - pos, nrm));
        float dist = length(pos - vPosWorld);

        indirect += diff * (dot1 * dot2) / (dist * dist * dist * dist);
    }
    indirect = 4.0 * 4*atan(1.0) * indirect / 64; //moyenne sur un cercle de 64 echantillons

    // IMPACT OF A STANDARD SHADOW MAP WITH DEPTH
    float depth = texture(udepthMap, texCoordLightSpace).x;
    float zValue = vPosLightSpace.z / vPosLightSpace.w;
    float visibility = 1.0;
    if (vPosLightSpace.w > 0.0 && depth < zValue - 0.01) {
        visibility = 0.5;
    }

    // IMPACT OF LIGHT
    vec3 N = normalize(vNrmView);
    vec3 L = normalize(vLightPos - vPosView);
    float ndotl = max(0.0, dot(N, L));

    // FINAL COLOR
    fColor = visibility * (vColor * ndotl + vColor * indirect);
    
}
