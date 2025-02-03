#version 330 core
out vec4 frag_colour;

in vec4 v_colour;

void main() {
    frag_colour = v_colour;
}
