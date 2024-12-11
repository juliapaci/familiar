#version 330 core
out vec4 frag_colour;

in vec4 v_colour;
in vec2 v_uv;

uniform int u_texture_index;

// TODO: maybe switch to sampler2DArray
uniform sampler2D u_textures[8];

void main() {
    frag_colour = v_colour * texture(u_textures[u_texture_index], v_uv);
};
