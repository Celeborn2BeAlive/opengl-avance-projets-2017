#version 430

struct Light {
    vec4 position;
    vec4 intensity;
};

layout(std430, binding = 0) buffer Lights {
    Light lights[];
};

uniform uint uLightCount;

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
    uint l;
    for (l = 0; l < uLightCount; ++ l) {
        vec3 wi, li;

        if (lights[l].position.w == 0) {
            wi = lights[l].position.xyz;
            li = lights[l].intensity.xyz;

        } else {
            wi = lights[l].position.xyz - position;
            float distance = length(wi);
            wi /= distance;

            li = lights[l].intensity.xyz / (distance * distance);
        }

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
}