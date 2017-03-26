#version 330

layout(triangles) in;
layout(triangle_strip, max_vertices=18) out;

in vec3 vposition[];
in vec3 vnormal[];
in vec4 vcolor[];

out vec3 gposWorld;
out vec3 gnrmWorld;
out vec4 gposScreen;
out vec4 gcolor;

uniform mat4 u_projMat;
uniform mat4 u_mvMat[6];

int cubeX[6] = int[6]( 0, 2, 1, 1, 1, 3 );
int cubeY[6] = int[6]( 1, 1, 0, 2, 1, 1 );

void main(void) {
    for (int face = 0; face < 6; face++) {
        mat4 mvpMat = u_projMat * u_mvMat[face];
        for (int k = 0; k < 3; k++) {
            gl_Position = mvpMat * vec4(vposition[k], 1.0);
            gl_Position.x = 0.25 * gl_Position.x + 0.25 * (2.0 * cubeX[face] + 1.0) - 1.0;
            gl_Position.y = 0.33 * gl_Position.y + 0.33 * (2.0 * (2 - cubeY[face]) + 1.0) - 1.0;

            gposWorld = vposition[k];
            gnrmWorld = vnormal[k];
            gposScreen = mvpMat * vec4(vposition[k], 1.0);
            gcolor = vcolor[k];

            EmitVertex();
        }
        EndPrimitive();
    }
}
