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
    set_cam_callback(window, &renderer.camera);

    while(!glfwWindowShouldClose(window)) {
        glClearColor(0.1, 0.1, 0.1, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        update_delta_time();
        render_frame_begin(&renderer);
        camera_update(&renderer.camera, renderer.shader);

        process_camera_input(window, &renderer.camera);
        process_general_input(window, &wireframe);

        render_push_triangle(&renderer,
            (RenderVertex){
                .pos = {0, 0, 0},
                .colour = {1, 1, 1, 1},
                .uv = {0, 0},
                .texture = render_get_white_texture()
            },
            (RenderVertex){
                .pos = {10, 0, 0},
                .colour = {1, 1, 1, 1},
                .uv = {1, 0},
            },
            (RenderVertex){
                .pos = {0, 10, 10},
                .colour = {1, 1, 1, 1},
                .uv = {1, 1},
            }
        );
        render_frame_end(&renderer);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    render_free(&renderer);
    glfwTerminate();
    return 0;
}
