#ifndef __FAMILIAR_CAMERA_H__
#define __FAMILIAR_CAMERA_H__

#include <engine/shader.h>
#include <GLFW/glfw3.h>
#include <cglm/struct.h>

#define NEAR_PLANE 0.01f
#define FAR_PLANE 100.0f

typedef struct {
    mat4s projection;
    mat4s view;
} CameraUniforms;

typedef struct {
    vec3s position;
    vec3s front;

    float yaw;
    float pitch;
    float roll;
    float fov;

    // TODO: not sure if i should keep these here
    // mouse state
    float last_x;
    float last_y;

    bool enabled;

    CameraUniforms uniforms;
} Camera;

extern const vec3s up_unit;
extern const float camera_speed;
extern const Camera CAMERA_DEFAULT;
Camera camera_init(void);

// updates camera projection transformation matrices in `camera.uniforms`
// NOTE: loads a perspective projection
void camera_update(Camera *camera);

// glfw stuff
void _mouse_callback(GLFWwindow *window, double x_pos, double y_pos);
void _scroll_callback(GLFWwindow *window, double x_off, double y_off);
inline void set_camera_callback(GLFWwindow *window, Camera *camera) {
    glfwSetWindowUserPointer(window, camera);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, _mouse_callback);
    glfwSetScrollCallback(window, _scroll_callback);
}

void process_camera_input(GLFWwindow *window, Camera *camera);

#endif // __FAMILIAR_CAMERA_H__
