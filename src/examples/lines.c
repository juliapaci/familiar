#include <engine/general.h>
#include <engine/shader.h>
#include <engine/camera.h>
#include <engine/renderer.h>

int main(void) {
    GLFWwindow *window = init_window("Familiar main example");
    if(window == NULL) return 1;

    Renderer renderer;
    render_init(&renderer);
    set_camera_callback(window, &renderer.camera);

    const vec3s lines[2] = {
        {-1, -1, 0},
        { 1,  1, 0},
    };
    GLuint vao;
    GLuint vbo;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(lines), lines, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3s), 0);
    glEnableVertexAttribArray(0);
    glUseProgram(renderer.shaders[OBJECT_LINE].id);

    while(!glfwWindowShouldClose(window)) {
        glClearColor(0.1, 0.1, 0.1, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDrawArrays(GL_LINES, 0, 2);

        // render_frame_begin(&renderer);
        //     render_switch_object(&renderer, OBJECT_LINE);
        //     render_switch_3d(&renderer);
        //     render_draw_line(&renderer, (Line){
        //         .start_x = -0.01,
        //         .start_y = -0.01,
        //         .start_z = 0,
        //         // .end_x = sin(glfwGetTime()),
        //         // .end_y = cos(glfwGetTime()),
        //         .end_x = 0.5,
        //         .end_y = 0.5,
        //         .end_z = 0,
        //
        //         .thickness = 10.0
        //     });
        //
        //     render_switch_object(&renderer, OBJECT_TRIANGLE);
        //     render_switch_3d(&renderer);
        //     render_push_triangle(&renderer,
        //         (RenderVertexTriangle){
        //             .pos    = {5, 0, 0},
        //             .colour = {1, 1, 1, 1},
        //             .uv     = {0, 0}
        //         },
        //         (RenderVertexTriangle){
        //             .pos    = {7, 0, 0},
        //             .colour = {1, 1, 1, 1},
        //             .uv     = {1, 0}
        //         },
        //         (RenderVertexTriangle){
        //             .pos    = {5, 2, 0},
        //             .colour = {1, 1, 1, 1},
        //             .uv     = {0, 1}
        //         },
        //         render_get_white_texture()
        //     );
        // render_frame_end(&renderer);

        process_general_input(window);
        process_camera_input(window, &renderer.camera);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    render_free(&renderer);
    glfwTerminate();
    return 0;
}
