#version 330 core
out vec4 frag_colour;

in vec4 v_colour;
in vec2 v_uv;
in vec3 v_pos;

// thanks to https://www.youtube.com/watch?v=VEnglRKNHjU
void main() {
    // square both sides to avoid an unnecessary sqrt
    // length(a) = a.a = ||a||^2
    // 1 = 1^2
    if(dot(v_pos, v_pos) > 1)
        discard;
    frag_colour = v_colour;
};
