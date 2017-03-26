#version 430

struct Light {
    vec4 position;
    vec4 intensity;
};

layout(std430, binding = 0) buffer Lights {
    Light lights[];
};

uniform uint uLightCount;

uniform vec3 uKa;
uniform vec3 uKd;
uniform vec3 uKs;
uniform float uShininess;

uniform sampler2D uKaSampler;
uniform sampler2D uKdSampler;
uniform sampler2D uKsSampler;
uniform sampler2D uShininessSampler;

in vec3 vViewSpacePosition;
in vec3 vViewSpaceNormal;
in vec2 vTexCoords;

out vec3 fColor;

void main() {
    // From object to camera.
    vec3 wo = -normalize(vViewSpacePosition);
    
    // Normal.
    vec3 n = vViewSpaceNormal / length(vViewSpaceNormal);

    // Ambiant coefficients.
    vec3 ka = uKa * texture(uKaSampler, vTexCoords).rgb;

    // Diffuse and glossy reflection coefficients.
    vec3 kd = uKd * texture(uKdSampler, vTexCoords).rgb;
    vec3 ks = uKs * texture(uKsSampler, vTexCoords).rgb;
  
    // Shininess.
    float s = uShininess * texture(uShininessSampler, vTexCoords).x;

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
            wi = lights[l].position.xyz - vViewSpacePosition;
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