#version 430 core
// thanks https://stackoverflow.com/a/60440937

layout (std140) uniform Camera {
    mat4 projection;
    mat4 view;
};
uniform mat4 u_model;

layout (std430, binding = 1) buffer TriangleVertices {
    vec4 vertex[];
};
uniform float u_thickness; // in pixels

out vec4 v_colour;

void main() {
    int line_index = gl_VertexID / 6;
    int triangle_index = gl_VertexID % 6;

    // the surrounding vertices
    vec4 proximity[4];
    for(int i = 0; i < 4; ++i) {
        proximity[i] = projection * view * vertex[line_index + i];
        // w should be 1?
        proximity[i].xyz /= proximity[i].w;
        // doesnt projection do this for me?
        // proximity[i].xy = (proximity[i].xy + 1.0) * 0.5 * res;
    }

    vec2 line_dir   = normalize(proximity[2].xy - proximity[1].xy);
    vec2 line_normal= vec2(-line_dir.y, line_dir.x);

    vec4 pos;
    if(triangle_index <= 3) { // if its part of the first segment
        vec2 predecessor_dir = normalize(proximity[1].xy - proximity[0].xy);
        vec2 miter_dir = normalize(line_normal + vec2(-predecessor_dir.y, predecessor_dir.x));

        pos     = proximity[1];
        pos.xy += miter_dir * u_thickness * (0.5 * int(triangle_index == 1)*2-1) / dot(miter_dir, line_normal);
    } else {
        vec2 successor_dir = normalize(proximity[3].xy - proximity[2].xy);
        vec2 miter_dir = normalize(line_normal + vec2(-successor_dir.y, successor_dir.x));

        pos     = proximity[2];
        pos.xy += miter_dir * u_thickness * (0.5 * int(triangle_index == 5)*2-1) / dot(miter_dir, line_normal);
    }

    pos.xyz *= pos.w;

    gl_Position = projection * view * pos;
    // v_colour = a_colour;
    v_colour = vec4(1.0);
}
