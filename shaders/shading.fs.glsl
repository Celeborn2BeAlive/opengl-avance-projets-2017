#version 430

uniform vec3 uLDirection;
uniform vec3 uLIntensity;

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

    // Ambiant and diffuse coefficient.
    vec3 ka = vec3(texelFetch(uGAmbient, ivec2(gl_FragCoord.xy), 0));
    vec3 kd = vec3(texelFetch(uGDiffuse, ivec2(gl_FragCoord.xy), 0));

    // Glossy reflection coefficient and shininess.
    vec4 gs = vec4(texelFetch(uGGlossyShininess, ivec2(gl_FragCoord.xy), 0));
    vec3 ks = gs.xyz;
    float s = gs.w;
  
    // Color computation.
    fColor = ka;

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

    fColor += li * (kd * (max(0, dot(wi, n))) + ks * gi);
}