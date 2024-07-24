#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <general.h>
#include <shader.h>
#include <camera.h>
#include <raycaster.h>

int main(void) {
    GLFWwindow *window = init_window("Familiar");
    if(window == NULL) return 1;
    unsigned int shader_program = shader_make("src/render_base/shader.vs", "src/render_base/shader.fs");
    bool wireframe = false;

    Player player;

    while(!glfwWindowShouldClose(window)) {
        update_delta_time();

        glClearColor(0.1, 0.1, 0.1, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        process_general_input(window, &wireframe);
        raycast(&player);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteProgram(shader_program);
    glfwTerminate();
    return 0;
}
