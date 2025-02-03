#version 330 core
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec4 a_colour;

out vec4 v_colour;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

void main() {
    gl_Position = u_projection * u_view * u_model * vec4(a_pos, 0.0);
    v_colour = a_colour;
}
