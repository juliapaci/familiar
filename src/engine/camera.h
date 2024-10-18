#ifndef __CAMERA_H__
#define __CAMERA_H__

#include <GLFW/glfw3.h>
#include <cglm/types.h>

typedef struct {
    vec3 position;
    vec3 target;
    vec3 front;

    float yaw;
    float pitch;
    float roll;
    float fov;

    // TODO: not sure if i should keep these here
    // mouse state
    float last_x;
    float last_y;
} Camera;

extern const vec3 up_unit;
extern const float camera_speed;
extern const Camera CAMERA_DEFAULT;
Camera camera_init(void);

// updates camera shader values and stuff
void camera_update(Camera *camera, unsigned int shader);

// glfw stuff
void _mouse_callback(GLFWwindow *window, double x_pos, double y_pos);
void _scroll_callback(GLFWwindow *window, double x_off, double y_off);
inline void set_cam_callback(GLFWwindow *window, Camera *camera) {
    glfwSetWindowUserPointer(window, camera);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, _mouse_callback);
    glfwSetScrollCallback(window, _scroll_callback);
}

void process_camera_input(GLFWwindow *window, Camera *camera);

#endif // __CAMERA_H__
