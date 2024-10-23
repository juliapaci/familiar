#ifndef __GENERAL_H__
#define __GENERAL_H__

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdbool.h>
#include <cglm/cglm.h>
#include <camera.h>

// TODO: siwtch to global arena allocator

#define MAX_TRIANGLES   2048
#define MAX_VERTICES    MAX_TRIANGLES * 3

// temporary solution
extern float delta_time;
extern float last_frame;
void update_delta_time(void);

// NULL if error
GLFWwindow *init_window(const char *name);

void _framebuffer_size_callback(GLFWwindow *window, int width, int height);
inline void set_framebuffer_size_callback(GLFWwindow *window) {
    glfwSetFramebufferSizeCallback(window, _framebuffer_size_callback);
}

void process_general_input(GLFWwindow *window, bool *wireframe);

typedef struct {
    vec3 pos;
    vec4 colour;

    vec2 uv;
    GLuint texture;
} RenderVertex;

// TODO: lookat useing TEXTURE_2D_ARRAY
typedef struct {
    // OpenGL objects
    GLuint vao;
    GLuint vbo;
    GLuint shader;

    // camera
    Camera camera;

    // Triangle data (cpu mirror of gpu buffer)
    RenderVertex triangle_data[MAX_VERTICES];
    uint16_t triangle_count;

    GLuint textures[8];
    uint8_t texture_count;
} Renderer;

void render_init(Renderer *renderer);
void render_free(Renderer *renderer);

void render_frame_begin(Renderer *renderer);
void render_frame_end(Renderer *renderer);

// Note: texture id is found in `a`
void render_push_triangle(Renderer *renderer, RenderVertex a, RenderVertex b, RenderVertex c);

GLuint render_get_white_texture(void);

#endif // __GENERAL_H__
