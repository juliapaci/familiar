#version 330 core
out vec4 frag_colour;

in vec4 v_colour;
in vec2 v_uv;
in float v_texture;

// layout (location = 0) out vec4 f_colour

uniform sampler2D u_tex[8];

void main() {
   // frag_colour = v_colour * texture(u_tex[int(v_texture)], v_uv);
   frag_colour = vec4(1);
    // switch (int(v_texture)) {
    //     case 0: frag_colour = v_colour * texture(u_tex[0], v_uv); break;
    //     case 1: frag_colour = v_colour * texture(u_tex[1], v_uv); break;
    //     case 2: frag_colour = v_colour * texture(u_tex[2], v_uv); break;
    //     case 3: frag_colour = v_colour * texture(u_tex[3], v_uv); break;
    //     case 4: frag_colour = v_colour * texture(u_tex[4], v_uv); break;
    //     case 5: frag_colour = v_colour * texture(u_tex[5], v_uv); break;
    //     case 6: frag_colour = v_colour * texture(u_tex[6], v_uv); break;
    //     case 7: frag_colour = v_colour * texture(u_tex[7], v_uv); break;
    //     default: discard;
    // }
};
