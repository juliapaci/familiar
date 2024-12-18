#ifndef __FAMILIAR_RENDERER_H__
#define __FAMILIAR_RENDERER_H__

#include <cglm/struct.h>
#include <glad/glad.h>
#include <engine/camera.h>
#include <stb/stb_truetype.h>
#include <engine/shader.h>

#define MAX_QUADS       1024
#define MAX_VERTICES    MAX_QUADS * 4
#define MAX_INDICES     MAX_QUADS * 6

#define SHADER_PATH "src/engine/shaders/"
#define TRIANGLE_VS SHADER_PATH "triangle.vs"
#define TRIANGLE_FS SHADER_PATH "triangle.fs"
#define CIRCLE_VS   SHADER_PATH "circle.vs"
#define CIRCLE_FS   SHADER_PATH "circle.fs"

typedef enum {
    OBJECT_TRIANGLE,
    OBJECT_CIRCLE,
    OBJECT_RECTANGLE,
    OBJECT_CUBE
} ObjectKind;

typedef struct {
    vec3s pos;
    vec4s colour;
    vec2s uv;
} RenderVertexTriangle;

typedef struct {
    vec3s pos;
    vec4s colour;

    float radius;
    float fade;     // [0, 1) -> blurryness
    float fullness; // [0, 1] -> doughnutness

    float index;
} RenderVertexCircle;

// TODO: lookat using TEXTURE_2D_ARRAY
typedef struct {
    // OpenGL objects and cpu mirror of gpu buffers
    struct {
        GLuint vao;
        GLuint ibo;
        GLuint vbo;

        GLuint index_buffer[MAX_INDICES];
        RenderVertexTriangle vertex_buffer[MAX_VERTICES];
        // TODO: theres definantly a way to derive index_count from vertex_count
        uint16_t index_count;
        uint16_t vertex_count;
    } triangle;

    struct {
        GLuint vao;
        GLuint vbo;

        RenderVertexCircle vertex_buffer[MAX_VERTICES];
        uint16_t vertex_count;
    } circle;

    // Current texture for batching textures in independant draw calls
    // TODO: need to properly batch renders but its difficult since i cant index into the sampler2d array with a non uniform value like a texture index vertex attribute
    GLuint textures[8];
    uint8_t texture_count;
    GLuint texture_index; // current index

    // shader (triangle, circle)
    ObjectKind object_kind;
    Shader shaders[2];

    // camera
    Camera camera;
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

// Note: circles dont have textures. use a texture in the shape of a circle on a quad if you need to
typedef struct {
    // centre of circle
    float x;
    float y;
    float radius;
} Circle;

void render_init(Renderer *renderer);
void render_free(Renderer *renderer);

// wrappers
// TODO: not sure if we should restrict to amount of shaders (i.e. OBJECT_RECTANGLE doesnt have a shader but it should only be used for animation anyway so i dont know)
inline Shader *render_shader(Renderer *r) { return r->shaders + r->object_kind; };

// batch control
void render_frame_begin(Renderer *renderer);
void render_frame_end(Renderer *renderer);
void render_frame_flush(Renderer *renderer);    // flushes current batch

// shader and (maybe in the future) buffer changes
void render_switch_triangle(Renderer *r);
void render_switch_circle(Renderer *r);

// perspective changes
// NOTE: renders the current batch before proceeding
// TODO: replace frame_end/begin (hacky fix until batching)
typedef enum {
    PROJECTION_PERSPECTIVE,
    PROJECTION_ORTHOGRAPHIC
} Projection;
void render_switch_projection(Renderer *r, Projection projection);
void render_switch_2d(Renderer *r);
void render_switch_3d(Renderer *r);

void render_populate_index_buffer(Renderer *r, size_t index_count);
void render_submit_batch(Renderer *r, GLuint texture); // update batch information like current texture and flushes the batch if certain criteria is met
void render_push_triangle(Renderer *r, RenderVertexTriangle a, RenderVertexTriangle b, RenderVertexTriangle c, GLuint texture);
void render_push_quad(Renderer *r, RenderVertexTriangle a, RenderVertexTriangle b, RenderVertexTriangle c, RenderVertexTriangle d, GLuint texture);
void render_push_circle(Renderer *r, RenderVertexCircle point);

void render_draw_rectangle_uv(Renderer *r, Rectangle rect, Rectangle uv, GLuint texture);
void render_draw_rectangle(Renderer *r, Rectangle rect, GLuint texture);
void render_draw_cube(Renderer *r, Cube cube, GLuint texture);
void render_draw_circle(Renderer *r, Circle circle);

GLuint render_get_white_texture(void);
GLuint render_texture_load(uint8_t *data, int32_t width, int32_t height, int32_t channels);
GLuint render_texture_load_file(const char *path);
void render_texture_free(GLuint texture);

void render_font_load(RenderFont *font, const uint8_t *data, size_t data_size, float font_size);
void render_font_load_file(RenderFont *font, const char *path, float size);
void render_font_free(RenderFont *font);

// textures of characters making up `text`
void render_draw_text(Renderer *r, RenderFont *font, vec2s pos, const char *text);

#endif // __FAMILIAR_RENDERER_H__
