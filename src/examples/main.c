#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GL/glext.h>

#include <engine/general.h>
#include <engine/shader.h>
#include <engine/camera.h>
#include <engine/renderer.h>

typedef struct {
    GLuint texture;
    vec3s pos;
    float size;
} Entity;

void draw_entity(Renderer *renderer, Entity *entity) {
    render_draw_cube(
        renderer,
        (Cube){
            entity->pos.x,
            entity->pos.y,
            entity->pos.z,
            entity->size,
            entity->size,
            entity->size
        },
        entity->texture
    );
}

void free_entity(Entity *entity) {
    render_texture_free(entity->texture);
}

int main(void) {
    GLFWwindow *window = init_window("Familiar main example");
    if(window == NULL) return 1;

    Renderer renderer;
    render_init(&renderer);
    set_camera_callback(window, &renderer.camera);

    RenderFont font;
    render_font_load_file(&font, "assets/OpenSans-VariableFont_wdth,wght.ttf", 10);

    Entity entity = {
        .texture    = render_texture_load_file("assets/awesomeface.png"),
        .pos        = {0, 0, 0},
        .size       = 1
    };

    while(!glfwWindowShouldClose(window)) {
        glClearColor(0.1, 0.1, 0.1, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        update_delta_time();
        if(glfwGetKey(window, GLFW_KEY_A))
            entity.pos.x += 4 * delta_time;
        else if(glfwGetKey(window, GLFW_KEY_D))
            entity.pos.x -= 4 * delta_time;
        else if(glfwGetKey(window, GLFW_KEY_W))
            entity.pos.y += 4 * delta_time;
        else if(glfwGetKey(window, GLFW_KEY_S))
            entity.pos.y -= 4 * delta_time;
        else if(glfwGetKey(window, GLFW_KEY_Q))
            entity.size -= delta_time;
        else if(glfwGetKey(window, GLFW_KEY_E))
            entity.size += delta_time;

        render_frame_begin(&renderer);
            render_switch_object(&renderer, OBJECT_TRIANGLE);
            render_switch_3d(&renderer);
            draw_entity(&renderer, &entity);

            render_switch_2d(&renderer);
            render_draw_text(&renderer, &font, (vec2s){-10, 0}, "a");
            render_push_triangle(&renderer,
                (RenderVertexTriangle){
                    .pos    = {5, 0, 0},
                    .colour = {1, 1, 1, 1},
                    .uv     = {0, 0}
                },
                (RenderVertexTriangle){
                    .pos    = {7, 0, 0},
                    .colour = {1, 1, 1, 1},
                    .uv     = {1, 0}
                },
                (RenderVertexTriangle){
                    .pos    = {5, 2, 0},
                    .colour = {1, 1, 1, 1},
                    .uv     = {0, 1}
                },
                1
            );
            render_switch_object(&renderer, OBJECT_CIRCLE);
            render_draw_circle(&renderer, (Circle){entity.pos.x, entity.pos.y, entity.size});

            render_switch_3d(&renderer);
            render_draw_circle(&renderer, (Circle){entity.pos.x + 2, entity.pos.y - 2, entity.size});
        render_frame_end(&renderer);

        process_general_input(window);
        process_camera_input(window, &renderer.camera);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    free_entity(&entity);
    render_font_free(&font);
    render_free(&renderer);
    glfwTerminate();
    return 0;
}
