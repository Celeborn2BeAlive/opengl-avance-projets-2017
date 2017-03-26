#version 430

// Light.
uniform vec3 uLDirection;
uniform vec3 uLIntensity;

// Shadow map.
uniform mat4 uShadowMapMatrix;
uniform sampler2DShadow uShadowMap;
uniform float uShadowMapBias;

// Input GBuffers.
uniform sampler2D uGPosition;
uniform sampler2D uGNormal;
uniform sampler2D uGAmbient;
uniform sampler2D uGDiffuse;
uniform sampler2D uGGlossyShininess;

out vec3 fColor;

void main() {
    // Position.
    vec3 position = vec3(texelFetch(uGPosition, ivec2(gl_FragCoord.xy), 0));

    // From object to camera.
    vec3 wo = -normalize(position);

    // Normal.
    vec3 n = vec3(texelFetch(uGNormal, ivec2(gl_FragCoord.xy), 0));

    // Ambiant and diffuse reflection coefficient.
    vec3 ka = vec3(texelFetch(uGAmbient, ivec2(gl_FragCoord.xy), 0));
    vec3 kd = vec3(texelFetch(uGDiffuse, ivec2(gl_FragCoord.xy), 0));

    // Glossy reflection coefficient and shininess.
    vec4 gs = vec4(texelFetch(uGGlossyShininess, ivec2(gl_FragCoord.xy), 0));
    vec3 ks = gs.xyz;
    float s = gs.w;
  
    // Color computation.
    fColor = ka;

    // Light occlusion.
    vec4 pls = uShadowMapMatrix * vec4(position, 1);
    vec3 plNDC = 0.5 * pls.xyz / pls.w + 0.5;
    float visibility = textureProj(uShadowMap, vec4(plNDC.xy, plNDC.z - uShadowMapBias, 1.0), 0);

    if (visibility > 0) {
        // Light contribution.
        vec3 wi = uLDirection;
        vec3 li = uLIntensity;

        // Half vector.
        vec3 hi = (wi + wo) / 2.f;

        // Glossiness.
        float gi = 0;
        if (s != 0) {
            float d = max(0, dot(hi, n));
            gi = (s == 1 ? d : pow(d, s));
        }

        fColor += visibility * li * (kd * (max(0, dot(wi, n))) + ks * gi);
    }
}