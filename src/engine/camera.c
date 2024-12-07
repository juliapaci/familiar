#include <engine/general.h>

#include "camera.h"

const vec3s up_unit = {0.0f, 1.0f, 0.0f};
const float camera_speed = 2.5f;

const Camera CAMERA_DEFAULT = {
    .position = {0.0f, 0.0f, 0.0f},
    .front = {0.0f, 0.0f, -1.0f},

    .yaw = 0.0f,
    .pitch = 0.0f,
    .roll = 0.0f,
    .fov = 90.0f,

    .last_x = 0.0f,
    .last_y = 0.0f,
};

Camera camera_init(void) {
    Camera camera = CAMERA_DEFAULT;
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    int width = viewport[2];
    int height = viewport[3];

    camera.last_x = width/2.0f;
    camera.last_y = height/2.0f;

    glm_lookat(
        (vec3) {0.0f, 0.0f, -0.5f},
        (vec3) {0.0f, 0.0f, 0.0f},
        (vec3) {0.0f, 1.0f, 0.0f},
        (mat4) GLM_MAT4_IDENTITY_INIT
    );

    return camera;
}

void camera_update(Camera *camera, Shader *shader) {
    // glEnable(GL_DEPTH_TEST);

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    const GLint width = viewport[2];
    const GLint height = viewport[3];

    const mat4s projection = glms_perspective(
        glm_rad(camera->fov),
        (float)width/(float)height,
        NEAR_PLANE,
        FAR_PLANE
    );

    const mat4s view = glms_lookat(
        camera->position,
        glms_vec3_add(camera->position, camera->front),
        up_unit
    );

    const mat4s model = GLMS_MAT4_IDENTITY;

    glUseProgram(shader->id);
    glUniformMatrix4fv(shget(shader->uniforms, "u_projection"), 1, GL_FALSE, (const GLfloat *)projection.raw);
    glUniformMatrix4fv(shget(shader->uniforms, "u_view"), 1, GL_FALSE, (const GLfloat *)view.raw);
    glUniformMatrix4fv(shget(shader->uniforms, "u_model"), 1, GL_FALSE, (const GLfloat *)model.raw);
}

void process_camera_input(GLFWwindow *window, Camera *camera) {
    camera->front = glms_vec3_scale(camera->front, camera_speed * delta_time);

    if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        camera->position = glms_vec3_add(camera->position, camera->front);
    if(glfwGetKey(window, GLFW_KEY_DOWN))
        camera->position = glms_vec3_sub(camera->position, camera->front);
    if(glfwGetKey(window, GLFW_KEY_LEFT))
        camera->position = glms_vec3_sub(
            camera->position,
            glms_vec3_cross(camera->front, up_unit)
        );
    if(glfwGetKey(window, GLFW_KEY_RIGHT))
        camera->position = glms_vec3_add(
            camera->position,
            glms_vec3_cross(camera->front, up_unit)
        );

    camera->front = glms_vec3_normalize(camera->front);
}

void _mouse_callback(GLFWwindow *window, double x_pos, double y_pos) {
    Camera *camera = glfwGetWindowUserPointer(window);

    float x_off = x_pos - camera->last_x;
    float y_off = camera->last_y - y_pos;
    camera->last_x = x_pos;
    camera->last_y = y_pos;

    const float sensitivity = 0.1f;
    x_off *= sensitivity;
    y_off *= sensitivity;

    camera->yaw += x_off;
    camera->pitch += y_off;

    camera->pitch = glm_clamp(camera->pitch, -89.0f, 89.0f);

    vec3s dir = {
        cos(glm_rad(camera->yaw)) * cos(glm_rad(camera->pitch)),
        sin(glm_rad(camera->pitch)),
        sin(glm_rad(camera->yaw)) * cos(glm_rad(camera->pitch))
    };

    camera->front = glms_vec3_normalize(dir);
}

void _scroll_callback(GLFWwindow *window, double x_off, double y_off) {
    Camera *camera = glfwGetWindowUserPointer(window);

    camera->fov -= (float)y_off;
    camera->fov = glm_clamp(camera->fov, 1.0f, 180.0f);
}
extern void set_cam_callback(GLFWwindow *window, Camera *camera);
