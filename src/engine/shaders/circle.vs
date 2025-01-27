#version 330 core
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec4 a_colour;
layout (location = 2) in float a_radius;
layout (location = 3) in float a_fade;
layout (location = 4) in float a_fullness;

out vec3 v_pos;
out vec4 v_colour;
out float v_fade;
out float v_fullness;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;



void main() {
    // camera transforms allows us to avoid aspect ratio nonsense
    gl_Position = u_projection * u_view * u_model * vec4(a_pos, 1.0);

    float pointsize = a_radius * 100.;
    vec4 camera_pos = u_view * u_model * vec4(vec3(0), 1);
    gl_PointSize = pointsize / (length(camera_pos.xyz) + 1.0);

    v_pos = vec3(1.0);
    v_colour = a_colour;
    v_fade = a_fade;
    v_fullness = a_fullness;
}
