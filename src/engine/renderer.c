#include "renderer.h"

#include <engine/shader.h>
#include <stb/stb_image.h>
#include <string.h>

void render_init(Renderer *r) {
    *r = (Renderer){0};

    glGenVertexArrays(1, &r->vao);
    glBindVertexArray(r->vao);

    glGenBuffers(1, &r->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, r->vbo);
    glBufferData(GL_ARRAY_BUFFER, MAX_VERTICES * sizeof(RenderVertex), NULL, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(RenderVertex), (void *)offsetof(RenderVertex, pos));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(RenderVertex), (void *)offsetof(RenderVertex, colour));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(RenderVertex), (void *)offsetof(RenderVertex, uv));
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);

    r->shader = shader_make("src/engine/shader.vs", "src/engine/shader.fs");
    glUseProgram(r->shader);

    r->camera = camera_init();
    camera_update(&r->camera, r->shader);

    // TODO: GL_MAX_TEXTURE_IMAGE_UNITS but also in fragment shader
    GLint textures[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
    glUniform1iv(glGetUniformLocation(r->shader, "u_textures"), 8, textures);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // default "no texture" as index 0
    r->textures[r->texture_count++] = render_get_white_texture();
    glUniform1i(glGetUniformLocation(r->shader, "texture_index"), 0);
}

void render_free(Renderer *r) {
    glDeleteBuffers(1, &r->vbo);
    glDeleteVertexArrays(1, &r->vao);
    glDeleteProgram(r->shader);

    glDeleteTextures(1, &r->textures[0]); // white texture
}

void render_frame_begin(Renderer *r) {
    r->triangle_count = 0;
}

void render_frame_end(Renderer *r) {
    const GLuint texture = r->textures[r->texture_index];
    size_t index = 0;
    for(; index < r->texture_count; index++)
        if(r->textures[index] == texture)
            break;

    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(glGetUniformLocation(r->shader, "texture_index"), index);

    glUseProgram(r->shader);
    glBindVertexArray(r->vao);
    glBindBuffer(GL_ARRAY_BUFFER, r->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, r->triangle_count * 3 * sizeof(RenderVertex), r->triangle_data);

    glDrawArrays(GL_TRIANGLES, 0, r->triangle_count * 3);
}

void render_switch_projection(Renderer *r, Projection projection) {
    render_frame_end(r);
    render_frame_begin(r);

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    const mat4s projection_matrix =
        projection == PROJECTION_PERSPECTIVE ?
            glms_perspective(
                glm_rad(r->camera.fov),
                (float)viewport[2]/viewport[3],
                NEAR_PLANE,
                FAR_PLANE
            )
        : // PROJECTION_ORTHOGRAPHIC
            glms_ortho(
                -viewport[2]/(2.0 * r->camera.fov),
                viewport[2]/(2.0 * r->camera.fov),
                -viewport[3]/(2.0 * r->camera.fov),
                viewport[3]/(2.0 * r->camera.fov),
                // TODO: make everything in scope somehow
                -100.0, 100.0
            )
        ;


    glUseProgram(r->shader);
    glUniformMatrix4fv(
        glGetUniformLocation(r->shader, "projection"),
        1,
        GL_FALSE,
        (const GLfloat *)&projection_matrix.raw
    );

}

void render_push_triangle(Renderer *r, RenderVertex a, RenderVertex b, RenderVertex c, GLuint texture) {
    bool same_texture = texture == r->textures[r->texture_index];

    // flush batch
    if(r->triangle_count == MAX_TRIANGLES || r->texture_count > 8 || !same_texture) {
        render_frame_end(r);
        render_frame_begin(r);
        if(r->texture_count > 8)
            r->texture_count = 0;
    }

    // new texture
    if(!same_texture && r->texture_count < 8) {
        bool exists = false;
        for(uint8_t i = 0; i < r->texture_count; i++) {
            if(r->textures[i] == texture) {
                exists = true;
                r->texture_index = i;
            }
        }

        if(!exists)
            r->textures[r->texture_count++] = texture;
    }


    const size_t offset = r->triangle_count++ * 3;
    r->triangle_data[offset + 0] = a;
    r->triangle_data[offset + 1] = b;
    r->triangle_data[offset + 2] = c;
}

void render_push_quad(Renderer *r, RenderVertex a, RenderVertex b, RenderVertex c, RenderVertex d, GLuint texture) {
    // TODO: triangle strip/fan or index buffer?
    render_push_triangle(r, a, b, c, texture);
    render_push_triangle(r, b, c, d, texture);
}

void render_draw_rectangle_uv(Renderer *r, Rectangle uv, Rectangle rect, GLuint texture) {
    render_push_quad(
        r,
        (RenderVertex){
            .pos    = {rect.x, rect.y, 0},
            .colour = {1, 1, 1, 1},
            .uv     = {uv.x, uv.y},
        },
        (RenderVertex){
            .pos    = {rect.x + rect.width, rect.y, 0},
            .colour = {1, 1, 1, 1},
            .uv     = {uv.x + uv.width, uv.y}
        },
        (RenderVertex){
            .pos    = {rect.x, rect.y + rect.height, 0},
            .colour = {1, 1, 1, 1},
            .uv     = {uv.x, uv.y + uv.height}
        },
        (RenderVertex){
            .pos    = {rect.x + rect.width, rect.y + rect.height, 0},
            .colour = {1, 1, 1, 1},
            .uv     = {uv.x + uv.width, uv.y + uv.height}
        },
        texture
    );
}

void render_draw_rectangle(Renderer *r, Rectangle rect, GLuint texture) {
    render_draw_rectangle_uv(
        r,
        (Rectangle){
            .x = 0,
            .y = 0,
            .width = 1,
            .height = 1
        },
        rect,
        texture
    );
}

void render_draw_cube(Renderer *r, Cube cube, GLuint texture) {
    // can generalize/macroize this but its more clear this way
    const vec3s vertices[6*4] = {
        {cube.x, cube.y, cube.z},
        {cube.x + cube.width, cube.y, cube.z},
        {cube.x, cube.y + cube.height, cube.z},
        {cube.x + cube.width, cube.y + cube.height, cube.z},

        {cube.x, cube.y, cube.z},
        {cube.x, cube.y, cube.z + cube.depth},
        {cube.x, cube.y + cube.height, cube.z},
        {cube.x, cube.y + cube.height, cube.z + cube.depth},

        {cube.x + cube.width, cube.y, cube.z},
        {cube.x + cube.width, cube.y, cube.z + cube.depth},
        {cube.x + cube.width, cube.y + cube.height, cube.z},
        {cube.x + cube.width, cube.y + cube.height, cube.z + cube.depth},

        {cube.x, cube.y, cube.z + cube.depth},
        {cube.x + cube.width, cube.y, cube.z + cube.depth},
        {cube.x, cube.y + cube.height, cube.z + cube.depth},
        {cube.x + cube.width, cube.y + cube.height, cube.z + cube.depth},

        {cube.x, cube.y, cube.z},
        {cube.x + cube.width, cube.y, cube.z},
        {cube.x, cube.y, cube.z + cube.depth},
        {cube.x + cube.width, cube.y, cube.z + cube.depth},

        {cube.x, cube.y + cube.height, cube.z},
        {cube.x + cube.width, cube.y + cube.height, cube.z},
        {cube.x, cube.y + cube.height, cube.z + cube.depth},
        {cube.x + cube.width, cube.y + cube.height, cube.z + cube.depth}
    };

    for(uint8_t i = 0; i < 6; i++) {
        render_push_quad(
            r,
            (RenderVertex){
                .pos    = vertices[4*i + 0],
                .uv     = {0, 0},
                .colour = {1, 1, 1, 1},
            },
            (RenderVertex){
                .pos    = vertices[4*i + 1],
                .uv     = {1, 0},
                .colour = {1, 1, 1, 1}
            },
            (RenderVertex){
                .pos    = vertices[4*i + 2],
                .uv     = {0, 1},
                .colour = {1, 1, 1, 1}
            },
            (RenderVertex){
                .pos    = vertices[4*i + 3],
                .uv     = {1, 1},
                .colour = {1, 1, 1, 1}
            },
            texture
        );
    }
}

GLuint _white_texture = UINT32_MAX;

GLuint render_get_white_texture(void) {
	if (_white_texture != UINT32_MAX)
        return _white_texture;

	GLuint texture;
	uint8_t image[4] = {UINT8_MAX, UINT8_MAX, UINT8_MAX, UINT8_MAX};
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    _white_texture = texture;

	return _white_texture;
}

GLuint render_texture_load(uint8_t *data, int32_t width, int32_t height, int32_t channels) {
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    GLint format = GL_RGB + (GL_RGBA - GL_RGB)*(channels == 4);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);
    glGenerateMipmap(GL_TEXTURE_2D);

    return texture;
}

GLuint render_texture_load_file(const char *path) {
    stbi_set_flip_vertically_on_load(true);
    int32_t width, height, channels;
    uint8_t *data = stbi_load(path, &width, &height, &channels, 0);

    return render_texture_load(data, width, height, channels);
}

void render_texture_free(GLuint texture) {
    glDeleteTextures(1, &texture);
}

void render_font_load(RenderFont *font, const uint8_t *data, size_t data_size, float font_size) {
    uint8_t tmp_bitmap[512 * 512];
    stbtt_fontinfo info;
    stbtt_pack_context spc;
    stbtt_InitFont(&info, data, 0);
    stbtt_PackBegin(&spc, tmp_bitmap, 512, 512, 0, 1, 0);
    stbtt_PackSetOversampling(&spc, 1, 1);
    stbtt_PackFontRange(&spc, data, 0, font_size, 32, 95, font->cdata);
    stbtt_PackEnd(&spc);

    glGenTextures(1, &font->texture);
    glBindTexture(GL_TEXTURE_2D, font->texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    GLint swizzles[4] = {GL_ONE, GL_ONE, GL_ONE, GL_RED};
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzles);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, 512, 512, 0, GL_RED, GL_UNSIGNED_BYTE, tmp_bitmap);
    glGenerateMipmap(GL_TEXTURE_2D);

    font->scale = stbtt_ScaleForPixelHeight(&info, font_size);
    stbtt_GetFontVMetrics(&info, &font->ascent, &font->descent, NULL);
    font->baseline = (font->ascent * font->scale);
    font->size = font_size;
}

void render_font_load_file(RenderFont *font, const char *path, float size) {
    FILE *ttf = fopen(path, "rb");
    if(ttf == NULL)
        return;

    fseek(ttf, 0, SEEK_END);
    size_t length = ftell(ttf);
    rewind(ttf);
    uint8_t data[length * sizeof(uint8_t)];
    fread(data, length, 1, ttf);
    fclose(ttf);

    render_font_load(font, data, length, size);
}

void render_font_free(RenderFont *font) {
    glDeleteTextures(1, &font->texture);
}

void render_draw_text(Renderer *r, RenderFont *font, vec2s pos, const char *text) {
    for(size_t i = 0; i < strlen(text); i++) {
        const stbtt_packedchar *const c = &font->cdata[text[i] - 32];

        Rectangle uv = {
            .x = c->x0 / 512.0f,
            .y = c->y0 / 512.0f,
            .width = (c->x1 - c->x0) / 512.0f,
            .height = (c->y1 - c->y0) / 512.0f
        };

        Rectangle area = {
            .x = pos.x + c->xoff,
            .y = pos.y - c->yoff2,
            .width = c->x1 - c->x0,
            .height = c->y1 - c->y0
        };

        render_draw_rectangle_uv(r, area, uv, font->texture);
        pos.x += c->xadvance;
    }
}
