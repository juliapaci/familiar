#ifndef __RENDERER_H__
#define __RENDERER_H__

#include <cglm/cglm.h>
#include <glad/glad.h>
#include <engine/camera.h>
#include <stb/stb_truetype.h>

#define MAX_TRIANGLES   2048
#define MAX_VERTICES    MAX_TRIANGLES * 3

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

typedef struct {
    GLuint texture;
    stbtt_packedchar cdata[95];
    float scale;
    float size;
    int32_t ascent;
    int32_t descent;
    int32_t baseline;
} RenderFont;

void render_init(Renderer *renderer);
void render_free(Renderer *renderer);

void render_frame_begin(Renderer *renderer);
void render_frame_end(Renderer *renderer);

// Note: texture id is found in `a`
void render_push_triangle(Renderer *renderer, RenderVertex a, RenderVertex b, RenderVertex c);

GLuint render_get_white_texture(void);

GLuint render_texure_load(const char *path);
void render_texture_free(GLuint texture);

void render_font_load(RenderFont *font, const char *path, float size);
void render_font_free(RenderFont *font);

#endif // __RENDERER_H__
