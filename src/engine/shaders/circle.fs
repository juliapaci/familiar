#version 330 core
out vec4 frag_colour;

in vec4 v_colour;
in vec3 v_pos;
in float v_fade;
in float v_fullness;

// thanks to
// https://www.youtube.com/watch?v=VEnglRKNHjU
// https://www.shadertoy.com/view/ssdSD2
void main() {
    frag_colour = v_colour;
    float ms = dot(v_pos, v_pos);
    // TODO: fullness
    // frag_colour.a *= smoothstep(0.0, v_fullness, ms);
    // frag_colour.a *= smoothstep(v_fullness, v_fade, ms);
    frag_colour.a *= abs(length(v_pos) - v_fade);
}
