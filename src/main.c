#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <general.h>
#include <shader.h>
#include <camera.h>

int main(void) {
    GLFWwindow *window = init_window("Familiar");
    if(window == NULL) return 1;
    bool wireframe = false;
    Renderer renderer;
    render_init(&renderer);

    while(!glfwWindowShouldClose(window)) {
        update_delta_time();
        render_frame_begin(&renderer);

        glClearColor(0.1, 0.1, 0.1, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        process_general_input(window, &wireframe);
        render_push_triangle(&renderer, (RenderVertex){0}, (RenderVertex){10}, (RenderVertex){10});

        glfwSwapBuffers(window);
        glfwPollEvents();

        render_frame_end(&renderer);
    }

    render_free(&renderer);
    glfwTerminate();
    return 0;
}
