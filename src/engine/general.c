#include "general.h"

#include <stdio.h>

float delta_time = 1.0;
float last_frame = 0.0f;

void update_delta_time(void) {
    float current_frame = glfwGetTime();
    delta_time = current_frame - last_frame;
    last_frame = current_frame;
}

GLFWwindow *init_window(const char *name) {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(800, 600, name, NULL, NULL);
    if(window == NULL) {
        fprintf(stderr, "failed to create glfw window\n");
        glfwTerminate();
        return NULL;
    }
    glfwMakeContextCurrent(window);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        fprintf(stderr, "failed to initialize glad\n");
        glfwTerminate();
        return NULL;
    }

    glViewport(0, 0, 800, 600);

    return window;
}

void _framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
}

void process_general_input(GLFWwindow *window, bool *wireframe) {
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    else if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        *wireframe = !*wireframe;
        glPolygonMode(GL_FRONT_AND_BACK, *wireframe ? GL_LINE : GL_FILL);
    }
}
