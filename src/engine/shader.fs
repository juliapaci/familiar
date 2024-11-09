#version 330 core
out vec4 frag_colour;

in vec4 v_colour;
in vec2 v_uv;
in float v_texture;

uniform sampler2D u_tex[8];

void main() {
    frag_colour = v_colour * texture(u_tex[int(v_texture)], v_uv);
};
