#version 330 core
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec4 a_colour;
layout (location = 2) in float a_radius;
layout (location = 3) in float a_fade;
layout (location = 4) in float a_fullness;

layout (std140) uniform Camera {
    mat4 projection;
    mat4 view;
};
uniform mat4 u_model;

out vec3 v_pos;
out vec4 v_colour;
out float v_fade;
out float v_fullness;

// the perfect size triangle which a circle can fit into
const vec2 CANVAS[3] = vec2[3](
    vec2( 0.0   , 2.0),
    vec2( 1.7321,-1.0), // sqrt(3)
    vec2(-1.7321,-1.0)
);

void main() {
    vec2 local_space = CANVAS[gl_VertexID%3];
    vec3 world_space = (a_pos + vec3(a_radius * local_space, 0.0));
    // camera transforms allows us to avoid aspect ratio nonsense
    gl_Position = projection * view * vec4(world_space, 1.0);

    v_pos = vec3(local_space, 0.0);
    v_colour = a_colour;
    v_fade = a_fade;
    v_fullness = a_fullness;
}
