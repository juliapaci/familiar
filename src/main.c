#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <engine/general.h>
#include <engine/shader.h>
#include <engine/camera.h>
#include <engine/renderer.h>

typedef struct {
    GLuint texture;
    vec3s pos;
} Entity;

void draw_entity(Renderer *renderer, Entity *entity) {
    render_push_triangle(renderer,
        (RenderVertex){
            .pos = entity->pos,
            .colour = {1, 1, 1, 1},
            .uv = {0, 0},
            .texture = entity->texture
        }, (RenderVertex){
            .pos = {10, 0, 0},
            .colour = {1, 1, 1, 1},
            .uv = {1, 0},
        }, (RenderVertex){
            .pos =  {10, 10, 0},
            .colour = {1, 1, 1, 1},
            .uv = {1, 1},
        }
    );
}

void free_entity(Entity *entity) {
    render_texture_free(entity->texture);
}

int main(void) {
    GLFWwindow *window = init_window("Familiar");
    glEnable(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    if(window == NULL) return 1;
    bool wireframe = false;
    Renderer renderer;
    render_init(&renderer);
    set_cam_callback(window, &renderer.camera);

    Entity entity = {
        .texture    = render_texture_load("build/awesomeface.png"),
        .pos        = {0, 0, 0}
    };

    while(!glfwWindowShouldClose(window)) {
        glClearColor(0.1, 0.1, 0.1, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        render_frame_begin(&renderer);
            update_delta_time();
            camera_update(&renderer.camera, renderer.shader);

            draw_entity(&renderer, &entity);
            if(glfwGetKey(window, GLFW_KEY_O))
                entity.pos.y += 4 * delta_time;

            process_camera_input(window, &renderer.camera);
            process_general_input(window, &wireframe);

        render_frame_end(&renderer);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    free_entity(&entity);
    render_free(&renderer);
    glfwTerminate();
    return 0;
}
