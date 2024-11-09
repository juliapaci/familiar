#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GL/glext.h>

#include <engine/general.h>
#include <engine/shader.h>
#include <engine/camera.h>
#include <engine/renderer.h>

#if OPENGL_DEBUG_APP == 1
void GLAPIENTRY debug_log_callback(
        GLenum source,
        GLenum type,
        GLuint id,
        GLenum severity,
        GLsizei length,
        const GLchar *message,
        const void *userParam
) {
    fprintf( stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
            ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
            type, severity, message );
}
#endif // OPENGL_DEBUG_APP

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
            .pos = {10, 10, 0},
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
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);
#if OPENGL_DEBUG_APP == 1
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(debug_log_callback, NULL);
#endif // OPENGL_DEBUG_APP

    glEnable(GL_DEPTH_TEST);

    if(window == NULL) return 1;
    bool wireframe = false;
    Renderer renderer;
    render_init(&renderer);
    set_cam_callback(window, &renderer.camera);

    Entity entity = {
        .texture    = render_texture_load_file("build/awesomeface.png"),
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
