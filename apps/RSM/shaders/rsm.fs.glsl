#version 330

in vec3 gposWorld;
in vec3 gnrmWorld;
in vec4 gposScreen;
in vec4 gcolor;

layout(location = 0) out vec3 fdepth;
layout(location = 1) out vec3 fposition;
layout(location = 2) out vec3 fnormal;
layout(location = 3) out vec3 fcolor;

void main(void) {
    float depth = gposScreen.z / gposScreen.w;
    fdepth = vec3(depth, depth, depth);

    fposition = gposWorld;
    fnormal = normalize(gnrmWorld) * 0.5 + 0.5;
    fcolor = vec3(gcolor.rgb);
}
