#ifndef __RENDERER_H__
#define __RENDERER_H__

#include <cglm/struct.h>
#include <glad/glad.h>
#include <engine/camera.h>
#include <stb/stb_truetype.h>

#define MAX_TRIANGLES   2048
#define MAX_VERTICES    MAX_TRIANGLES * 3

// TODO: normals

typedef struct {
    vec3s pos;
    vec4s colour;

    vec2s uv;
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

typedef struct {
    float x;
    float y;
    float width;
    float height;
} Rectangle;

typedef struct {
    float x;
    float y;
    float z;
    float width;
    float height;
    float depth;
} Cube;

void render_init(Renderer *renderer);
void render_free(Renderer *renderer);

void render_frame_begin(Renderer *renderer);
void render_frame_end(Renderer *renderer);

// perspective changes
// NOTE: renders the current batch before proceeding
// TODO: replace frame_end/begin (hacky fix until batching)
typedef enum {
    PROJECTION_PERSPECTIVE,
    PROJECTION_ORTHOGRAPHIC
} Projection;
void render_switch_projection(Renderer *r, Projection projection);

// Note: texture id is found in `a`
void render_push_triangle(Renderer *renderer, RenderVertex a, RenderVertex b, RenderVertex c);
void render_push_quad(Renderer *r, RenderVertex a, RenderVertex b, RenderVertex c, RenderVertex d);
void render_draw_rectangle_uv(Renderer *r, Rectangle uv, Rectangle rect, GLuint texture);
void render_draw_rectangle(Renderer *r, Rectangle rect, GLuint texture);
void render_draw_cube(Renderer *r, Cube cube, GLuint texture);

GLuint render_get_white_texture(void);

GLuint render_texture_load(uint8_t *data, int32_t width, int32_t height, int32_t channels);
GLuint render_texture_load_file(const char *path);
void render_texture_free(GLuint texture);

void render_font_load(RenderFont *font, const uint8_t *data, size_t data_size, float font_size);
void render_font_load_file(RenderFont *font, const char *path, float size);
void render_font_free(RenderFont *font);

// textures of characters making up `text`
void render_draw_text(Renderer *r, RenderFont *font, vec2s pos, const char *text);

#endif // __RENDERER_H__
