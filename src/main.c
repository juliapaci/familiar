#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cglm/cglm.h>

#include <shader.h>
#include <camera.h>
#include <general.h>

#include <stdio.h>
#include <stdbool.h>

float last_frame = 0.0f;

int main(void) {
    GLFWwindow *window = init_window();
    if(window == NULL) return 1;
    unsigned int shader_program = shader_make("src/render_base/shader.vs", "src/render-base/shader.fs");
    bool wireframe = false;
    Camera camera = init_camera();

    while(!glfwWindowShouldClose(window)) {
        float current_frame = glfwGetTime();
        delta_time = current_frame - last_frame;
        last_frame = current_frame;

        process_general_input(window, &wireframe);

        glClearColor(0.1, 0.1, 0.1, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteProgram(shader_program);
    glfwTerminate();
    return 0;
}
