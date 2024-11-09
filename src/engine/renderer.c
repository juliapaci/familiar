#include "renderer.h"

#include <engine/shader.h>
#include <stb/stb_image.h>

void render_init(Renderer *r) {
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
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(RenderVertex), (void *)offsetof(RenderVertex, texture));
    glEnableVertexAttribArray(3);

    r->shader = shader_make("src/engine/shader.vs", "src/engine/shader.fs");
    glUseProgram(r->shader);

    r->camera = camera_init();
    camera_update(&r->camera, r->shader);

    GLuint tex_loc = glGetUniformLocation(r->shader, "u_tex");
    GLint textures[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
    glUniform1iv(tex_loc, 8, textures);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void render_free(Renderer *r) {
    glDeleteBuffers(1, &r->vbo);
    glDeleteVertexArrays(1, &r->vao);
    glDeleteProgram(r->shader);
}

void render_frame_begin(Renderer *r) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    r->triangle_count = 0;
    r->texture_count = 0;
}

void render_frame_end(Renderer *r) {
    for(GLuint i = 0; i < r->texture_count; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_3D, r->textures[i]);
    }

    glUseProgram(r->shader);
    glBindVertexArray(r->vao);
    glBindBuffer(GL_ARRAY_BUFFER, r->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, r->triangle_count * 3 * sizeof(RenderVertex), r->triangle_data);

    glDrawArrays(GL_TRIANGLES, 0, r->triangle_count * 3);
}

void render_push_triangle(Renderer *r, RenderVertex a, RenderVertex b, RenderVertex c) {
    // UINT32_MAX is error value
    GLuint texture = UINT32_MAX;
    for(GLuint i = 0; i < r->texture_count; i++) {
        if(r->textures[i] == a.texture) {
            texture = i;
            break;
        }
    }

    if(texture == UINT32_MAX && r->texture_count < 8) {
        r->textures[r->texture_count] = a.texture;
        texture = r->texture_count++;
    }

    if(r->triangle_count == MAX_TRIANGLES || texture == UINT32_MAX) {
        render_frame_end(r);
        render_frame_begin(r);
    }

    texture = c.texture = b.texture = a.texture;
    const size_t offset = r->triangle_count++ * 3;
    r->triangle_data[offset + 0] = a;
    r->triangle_data[offset + 1] = b;
    r->triangle_data[offset + 2] = c;
}

GLuint _white_texture = UINT32_MAX;

GLuint render_get_white_texture(void) {
	if (_white_texture != UINT32_MAX)
        return _white_texture;

	GLuint texture;
	uint8_t image[4] = { 255, 255, 255, 255 };
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    _white_texture = texture;

	return _white_texture;
}

GLuint render_texture_load(const char *path) {
    stbi_set_flip_vertically_on_load(true);
    int32_t width, height, channels;
    uint8_t *data = stbi_load(path, &width, &height, &channels, 0);

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, channels, GL_RGB8 + (GL_RGBA8 - GL_RGB8)*(channels == 4), width, height, 0, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);
    glGenerateMipmap(GL_TEXTURE_2D);

    return texture;
}

void render_texture_free(GLuint texture) {
    glDeleteTextures(1, &texture);
}

void render_font_load(RenderFont *font, const char *path, float size) {
    FILE *ttf = fopen(path, "rb");
    if(ttf == NULL)
        return;

    fseek(ttf, 0, SEEK_END);
    size_t length = ftell(ttf);
    rewind(ttf);
    uint8_t data[length * sizeof(uint8_t)];
    fread(data, length, 1, ttf);
    fclose(ttf);

    uint8_t tmp_bitmap[512 * 512];
    stbtt_fontinfo info;
    stbtt_pack_context spc;
    stbtt_InitFont(&info, data, 0);
    stbtt_PackBegin(&spc, tmp_bitmap, 512, 512, 0, 1, 0);
    stbtt_PackSetOversampling(&spc, 1, 1);
    stbtt_PackFontRange(&spc, data, 0, size, 32, 95, font->cdata);
    stbtt_PackEnd(&spc);

    glGenTextures(1, &font->texture);
    glBindTexture(GL_TEXTURE_2D, font->texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    GLint swizzles[4] = {GL_ONE, GL_ONE, GL_ONE, GL_RED};
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzles);
    glTexImage2D(GL_TEXTURE_2D, 0, 0, 0, 512, 512, GL_RED, GL_UNSIGNED_BYTE, tmp_bitmap);

    font->scale = stbtt_ScaleForPixelHeight(&info, size);
    stbtt_GetFontVMetrics(&info, &font->ascent, &font->descent, NULL);
    font->baseline = (font->ascent * font->scale);
    font->size = size;
}

void render_font_free(RenderFont *font) {
    glDeleteTextures(1, &font->texture);
}
