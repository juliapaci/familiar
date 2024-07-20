#ifndef __GENERAL_H__
#define __GENERAL_H__

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdbool.h>

// temporary fix
extern float delta_time;

// NULL if error
GLFWwindow *init_window(void);

void _framebuffer_size_callback(GLFWwindow *window, int width, int height);
inline void set_framebuffer_size_callback(GLFWwindow *window) {
    glfwSetFramebufferSizeCallback(window, _framebuffer_size_callback);
}

void process_general_input(GLFWwindow *window, bool *wireframe);

#endif // __GENERAL_H__
