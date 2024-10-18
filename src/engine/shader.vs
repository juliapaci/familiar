#version 330 core
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec4 a_colour;

out vec4 v_colour;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(a_pos, 1.0);
    v_colour = a_colour;
}
