#ifndef __GENERAL_H__
#define __GENERAL_H__

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdbool.h>
#include <cglm/struct.h>
#include <engine/camera.h>

// TODO: siwtch to global arena allocator

// temporary solution
extern float delta_time;
extern float last_frame;
void update_delta_time(void);

// NULL if error
GLFWwindow *init_window(const char *name);

void _framebuffer_size_callback(GLFWwindow *window, int width, int height);
inline void set_framebuffer_size_callback(GLFWwindow *window) {
    glfwSetFramebufferSizeCallback(window, _framebuffer_size_callback);
}

void process_general_input(GLFWwindow *window, bool *wireframe);

#endif // __GENERAL_H__
