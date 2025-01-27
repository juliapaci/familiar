#version 330 core
out vec4 frag_colour;

in vec4 v_colour;
in vec2 v_uv;

uniform int u_texture_index;

// TODO: maybe switch to sampler2DArray
uniform sampler2D u_textures[8];

void main() {
    switch(u_texture_index) {
        case 0: frag_colour = v_colour * texture(u_textures[0], v_uv); break;
        case 1: frag_colour = v_colour * texture(u_textures[1], v_uv); break;
        case 2: frag_colour = v_colour * texture(u_textures[2], v_uv); break;
        case 4: frag_colour = v_colour * texture(u_textures[4], v_uv); break;
        case 5: frag_colour = v_colour * texture(u_textures[5], v_uv); break;
        case 6: frag_colour = v_colour * texture(u_textures[6], v_uv); break;
        case 7: frag_colour = v_colour * texture(u_textures[7], v_uv); break;
    }
};
