#version 330 core
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec4 a_colour;
layout (location = 2) in vec2 a_uv;

layout (std140) uniform Camera {
    mat4 projection;
    mat4 view;
};
uniform mat4 u_model;

out vec4 v_colour;
out vec2 v_uv;

void main() {
    gl_Position = projection * view * vec4(a_pos, 1.0);
    v_colour = a_colour;
    v_uv = a_uv;
}
