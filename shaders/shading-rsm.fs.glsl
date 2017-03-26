#version 430

#define PI 3.1415926535897932384626433832795

// Light.
uniform vec3 uLDirection;
uniform vec3 uLIntensity;

// Shadow map.
uniform mat4 uShadowMapMatrix;
// uniform sampler2DShadow uShadowMap;
uniform sampler2D uShadowMap;
uniform float uShadowMapBias;

// Indirect lighting.
uniform sampler2D uRSMPosition;
uniform sampler2D uRSMNormal;
uniform sampler2D uRSMFlux;
uniform uint uMaxSamples;
uniform uint uCeil;

// Input GBuffers.
uniform sampler2D uGPosition;
uniform sampler2D uGNormal;
uniform sampler2D uGAmbient;
uniform sampler2D uGDiffuse;
uniform sampler2D uGGlossyShininess;

out vec3 fColor;

uint hash(uint x) {
    x += ( x << 10u );
    x ^= ( x >>  6u );
    x += ( x <<  3u );
    x ^= ( x >> 11u );
    x += ( x << 15u );
    return x;
}

uint hash(uvec2 v) {
    return hash(v.x ^ hash(v.y));
}

float random(vec2 v) {
    const uint mantissaMask = 0x007FFFFFu;
    const uint one          = 0x3F800000u;
    
    uint h = hash(uvec2(floatBitsToUint(v.x), floatBitsToUint(v.y)));
    h &= mantissaMask;
    h |= one;
    
    float  r2 = uintBitsToFloat(h);
    return r2 - 1.0;
}


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
    // fColor = ka;

    // Light occlusion.
    vec4 pls = uShadowMapMatrix * vec4(position, 1);
    vec3 plNDC = 0.5 * pls.xyz / pls.w + 0.5;
    float occluder = texture(uShadowMap, plNDC.xy).r;
    float visibility = plNDC.z < occluder + uShadowMapBias ? 1 : 0;
    // float visibility = textureProj(uShadowMap, vec4(plNDC.xy, plNDC.z - uShadowMapBias, 1.0), 0);

    vec3 lighting = vec3(0);

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

        lighting = visibility * li * (kd * (max(0, dot(wi, n))) + ks * gi);
        fColor += lighting;

    }

    if (length(lighting) < 0.1f) {
        ivec2 size = textureSize(uRSMPosition, 0);
        vec3 color = vec3(0);

        // Sample pixels in the RSM.
        vec2 seed = gl_FragCoord.xy;

        int N = int(sqrt(float(uMaxSamples)));
        float deltaN = 1.f / float(N);

        int trueCount = N * N;

        int bad = 0, good = 0;
        for (int i = 0; i < trueCount; ++ i) {
            int x = i % int(N);
            int y = i / int(N);

            float e1 = deltaN * (float(x) + 0.5);
            float e2 = deltaN * (float(y) + 0.5);

            vec2 ilight = vec2(e1, e2) * size;
            vec2 ll = vec2(e1,e2);
            if (ilight.x < 0 || ilight.x >= size.x || ilight.y < 0 || ilight.y >= size.y) {
                continue;
            }

            vec4 pvpl = texelFetch(uRSMPosition, ivec2(ilight), 0);
            vec3 ni = vec3(texelFetch(uRSMNormal, ivec2(ilight), 0));

            float vplpdf = dot(uLDirection, ni);

            // Contribution
            vec3 wi = vec3(pvpl) - position;
            float dist = length(wi);
            wi /= dist;

            if (length(ni) < 0.99 || dot(wi, ni) >= 0) {
                continue;
            }

            vec3 flux = vec3(texelFetch(uRSMFlux, ivec2(ilight), 0));

            // Intensity.
            vec3 li = size.x * size.y * max(0, dot(-wi, ni)) * flux / (dist * dist * vplpdf);

            // Half vector.
            vec3 hi = (wi + wo) / 2.f;

            // Glossiness.
            float gi = 0;
            if (s != 0) {
                float d = max(0, dot(hi, n));
                gi = (s == 1 ? d : pow(d, s));
            }

            color += li * (kd * (max(0, dot(wi, n))) + ks * gi);
            ++ good;
        }
        fColor += color * deltaN * deltaN;
    }

    fColor = pow(fColor, vec3(1.0 / 2.2));
}