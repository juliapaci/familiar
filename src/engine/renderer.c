#include "renderer.h"

#include <stb/stb_image.h>
#include <string.h>

void render_init(Renderer *r) {
    *r = (Renderer){0};

    // initialise buffers
    // TODO: auto generate vertex attributes: serialise structs to auto generate layout and shader ig possibly using stb_c_lexer or maybe some reflection
    // TODO: make an ecs and add default transform component so we can define local space vertices for shapes and then transform them to their position (that way theres easier maths in shaders)
    { // triangle
        glGenVertexArrays(1, &r->triangle.vao);
        glBindVertexArray(r->triangle.vao);

        glGenBuffers(1, &r->triangle.ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r->triangle.ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_INDICES * sizeof(GLuint), NULL, GL_DYNAMIC_DRAW);

        glGenBuffers(1, &r->triangle.vbo);
        glBindBuffer(GL_ARRAY_BUFFER, r->triangle.vbo);
        glBufferData(GL_ARRAY_BUFFER, MAX_VERTICES * sizeof(RenderVertexTriangle), NULL, GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(RenderVertexTriangle), (void *)offsetof(RenderVertexTriangle, pos));
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(RenderVertexTriangle), (void *)offsetof(RenderVertexTriangle, colour));
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(RenderVertexTriangle), (void *)offsetof(RenderVertexTriangle, uv));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
    }


    { // circle
        glGenVertexArrays(1, &r->circle.vao);
        glBindVertexArray(r->circle.vao);

        glGenBuffers(1, &r->circle.ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r->circle.ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_INDICES * sizeof(GLuint), NULL, GL_DYNAMIC_DRAW);

        glGenBuffers(1, &r->circle.vbo);
        glBindBuffer(GL_ARRAY_BUFFER, r->circle.vbo);
        glBufferData(GL_ARRAY_BUFFER, MAX_VERTICES * sizeof(RenderVertexTriangle), NULL, GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(RenderVertexCircle), (void *)offsetof(RenderVertexCircle, pos));
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(RenderVertexCircle), (void *)offsetof(RenderVertexCircle, colour));
        glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(RenderVertexCircle), (void *)offsetof(RenderVertexCircle, radius));
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(RenderVertexCircle), (void *)offsetof(RenderVertexCircle, fade));
        glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(RenderVertexCircle), (void *)offsetof(RenderVertexCircle, fullness));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glEnableVertexAttribArray(3);
        glEnableVertexAttribArray(4);
    }

    // shader
    shader_init(&r->shaders[OBJECT_TRIANGLE], SHADER_TRIANGLE);
    shader_init(&r->shaders[OBJECT_CIRCLE], SHADER_CIRCLE);
    r->object_kind = OBJECT_TRIANGLE;
    glUseProgram(render_shader(r)->id);

    // textures
    // TODO: GL_MAX_TEXTURE_IMAGE_UNITS but also in fragment shader
    GLint textures[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
    glUniform1iv(shget(render_shader(r)->uniforms, "u_textures[0]"), 8, textures);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // default "no texture" as index 0
    r->textures[r->texture_count++] = render_get_white_texture();
    glUniform1i(shget(render_shader(r)->uniforms, "u_texture_index"), 0);

    // camera
    r->camera = camera_init();
    camera_update(&r->camera, render_shader(r));
}

void render_free(Renderer *r) {
    glDeleteBuffers(1, &r->triangle.vbo);
    glDeleteBuffers(1, &r->triangle.ibo);
    glDeleteVertexArrays(1, &r->triangle.vao);

    glDeleteBuffers(1, &r->circle.vbo);
    glDeleteBuffers(1, &r->circle.ibo);
    glDeleteVertexArrays(1, &r->circle.vao);

    for(size_t i = 0; i < sizeof(r->shaders)/sizeof(r->shaders[0]); i++)
        shader_free(&r->shaders[i]);

    glDeleteTextures(1, &r->textures[0]); // white texture
}

extern Shader *render_shader(Renderer *r);

void render_frame_begin(Renderer *r) {
    r->triangle.vertex_count = 0;
    r->triangle.index_count = 0;
    r->circle.vertex_count = 0;
}

void render_frame_end(Renderer *r) {

    // TODO: possibly keep a "different_shape" bool in `Renderer` (see `render_switch_object`)
    //       which tells us if the object context has changed since the last frame
    //       so we dont have to bind the same vertex arrays?
    // buffer updates and draw calls
    switch(r->object_kind) {
        // case OBJECT_CUBE:
        // case OBJECT_RECTANGLE:
        case OBJECT_TRIANGLE: {
            // current batch's texture
            glActiveTexture(GL_TEXTURE0 + r->texture_index);
            glBindTexture(GL_TEXTURE_2D, r->textures[r->texture_index]);
            glUniform1i(shget(render_shader(r)->uniforms, "u_texture_index"), r->texture_index);

            glBindVertexArray(r->triangle.vao);

            glBindBuffer(GL_ARRAY_BUFFER, r->triangle.vbo);
            glBufferSubData(
                GL_ARRAY_BUFFER,
                0,
                r->triangle.vertex_count * sizeof(RenderVertexTriangle),
                r->triangle.vertex_buffer
            );

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r->triangle.ibo);
            glBufferSubData(
                GL_ELEMENT_ARRAY_BUFFER,
                0,
                r->triangle.index_count * sizeof(GLuint),
                r->triangle.index_buffer
            );

            glDrawElements(
                GL_TRIANGLES,
                r->triangle.index_count,
                GL_UNSIGNED_INT,
                NULL
            );
        } break;

        case OBJECT_CIRCLE: {
            glBindVertexArray(r->circle.vao);
            glBindBuffer(GL_ARRAY_BUFFER, r->circle.vbo);
            glBufferSubData(
                GL_ARRAY_BUFFER,
                0,
                r->circle.vertex_count * sizeof(RenderVertexCircle),
                r->circle.vertex_buffer
            );

            GLuint indices[r->circle.vertex_count * 3];
            for(size_t i = 0; i < r->circle.vertex_count; i++) {
                indices[i*3 + 0] = i;
                indices[i*3 + 1] = i;
                indices[i*3 + 2] = i;
            }
            for(size_t i = 0; i < r->circle.vertex_count * 3; i++)
                indices[i] = i;
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r->circle.ibo);
            glBufferSubData(
                GL_ELEMENT_ARRAY_BUFFER,
                0,
                sizeof(indices),
                indices
            );

            // TODO: instancing?
            glDrawElements(
                GL_TRIANGLES,
                r->circle.vertex_count * 3,
                GL_UNSIGNED_INT,
                NULL
            );
        } break;

        default: return;
    }
}

void render_frame_flush(Renderer *r) {
    if(r->triangle.vertex_count + r->circle.vertex_count == 0)
        return;

    render_frame_end(r);
    render_frame_begin(r);
    r->texture_index = 0;
}

void render_switch_object(Renderer *r, ObjectKind kind) {
    // if(r->object_kind == kind)
    //     return;

    render_frame_flush(r);
    r->object_kind = kind;
    glUseProgram(render_shader(r)->id);
}

void render_switch_projection(Renderer *r, Projection projection) {
    render_frame_flush(r);

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
                -0.1f, 0.1f
            )
        ;


    glUseProgram(render_shader(r)->id);
    glUniformMatrix4fv(
        shget(render_shader(r)->uniforms, "u_projection"),
        1,
        GL_FALSE,
        (const GLfloat *)&projection_matrix.raw
    );

}

void render_switch_2d(Renderer *r) {
    // projection change flushes flushes aswell
    render_switch_projection(r, PROJECTION_ORTHOGRAPHIC);
    glDisable(GL_DEPTH_TEST);
    const mat4s view = GLMS_MAT4_IDENTITY;
    const mat4s model = GLMS_MAT4_IDENTITY;
    glUniformMatrix4fv(
        shget(render_shader(r)->uniforms, "u_view"),
        1,
        GL_FALSE,
        (const GLfloat *)&view.raw
    );
    glUniformMatrix4fv(
        shget(render_shader(r)->uniforms, "u_model"),
        1,
        GL_FALSE,
        (const GLfloat *)&view.raw
    );
}

void render_switch_3d(Renderer *r) {
    // flush
    render_frame_flush(r);
    glEnable(GL_DEPTH_TEST);
    camera_update(&r->camera, render_shader(r));
}

void render_submit_batch(Renderer *r, GLuint texture) {
    const bool same_texture = texture == r->textures[r->texture_index];

    // flush batch
    if(
        r->triangle.vertex_count + r->circle.vertex_count == MAX_VERTICES ||
        r->texture_count > 8 ||
        !same_texture
    ) {
        render_frame_flush(r);
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
                break;
            }
        }

        if(!exists)
            r->textures[r->texture_count++] = texture;
    }
}

// TODO: maybe generalise all of this with variadic functions for auto index and vertex buffer filling

void render_populate_index_buffer(Renderer *r, size_t index_count) {
    // const size_t index = r->vertex_count;
    // GLuint indices[index_count];
    // for(size_t i = 0; i < index_count; i++) {
    //     indices[i + 0] = index + i;
    // }
    //
    // memcpy(r->index_buffer + r->index_count, indices, sizeof(indices));
    // r->index_count += index_count;
}

void render_push_triangle(Renderer *r, RenderVertexTriangle a, RenderVertexTriangle b, RenderVertexTriangle c, GLuint texture) {
    render_submit_batch(r, texture);

    // TODO: fix triangles and rectangles (although they have correct indices) render weirdly
    // 0 1 2
    // 3 4 5
    // ...
    const size_t index = r->triangle.vertex_count;
    const GLuint indices[1 * 3] = {
        index + 0, index + 1, index + 2
    };
    memcpy(r->triangle.index_buffer + r->triangle.index_count, indices, sizeof(indices));
    r->triangle.index_count += 3;

    const size_t offset = r->triangle.vertex_count;
    r->triangle.vertex_buffer[offset + 0] = a;
    r->triangle.vertex_buffer[offset + 1] = b;
    r->triangle.vertex_buffer[offset + 2] = c;
    r->triangle.vertex_count += 3;
}

void render_push_quad(Renderer *r, RenderVertexTriangle a, RenderVertexTriangle b, RenderVertexTriangle c, RenderVertexTriangle d, GLuint texture) {
    render_submit_batch(r, texture);
    // TODO: triangle strip/fan?

    // 0 1 2, 1 2 3
    // 4 5 6, 5 6 7
    // ...
    const size_t index = r->triangle.vertex_count;
    const GLuint indices[2 * 3] = {
        index + 0, index + 1, index + 2,
        index + 1, index + 2, index + 3
    };
    memcpy(r->triangle.index_buffer + r->triangle.index_count, indices, sizeof(indices));
    r->triangle.index_count += 6;

    const size_t offset = r->triangle.vertex_count;
    r->triangle.vertex_buffer[offset + 0] = a;
    r->triangle.vertex_buffer[offset + 1] = b;
    r->triangle.vertex_buffer[offset + 2] = c;
    r->triangle.vertex_buffer[offset + 3] = d;
    r->triangle.vertex_count += 4;
}

void render_push_circle(Renderer *r, RenderVertexCircle point) {
    render_submit_batch(r, 0); // TODO: 0 or 1?

    r->circle.vertex_buffer[r->circle.vertex_count++] = point;
}

void render_draw_rectangle_uv(Renderer *r, Rectangle rect, Rectangle uv, GLuint texture) {
    render_push_quad(
        r,
        (RenderVertexTriangle){
            .pos    = {rect.x, rect.y, 0},
            .colour = {1, 1, 1, 1},
            .uv     = {uv.x, uv.y}
        },
        (RenderVertexTriangle){
            .pos    = {rect.x + rect.width, rect.y, 0},
            .colour = {1, 1, 1, 1},
            .uv     = {uv.x + uv.width, uv.y}
        },
        (RenderVertexTriangle){
            .pos    = {rect.x, rect.y + rect.height, 0},
            .colour = {1, 1, 1, 1},
            .uv     = {uv.x, uv.y + uv.height}
        },
        (RenderVertexTriangle){
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
        rect,
        (Rectangle){
            .x = 0,
            .y = 0,
            .width = 1,
            .height = 1
        },
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
            (RenderVertexTriangle){
                .pos    = vertices[4*i + 0],
                .colour = {1, 1, 1, 1},
                .uv     = {0, 0}
            },
            (RenderVertexTriangle){
                .pos    = vertices[4*i + 1],
                .colour = {1, 1, 1, 1},
                .uv     = {1, 0}
            },
            (RenderVertexTriangle){
                .pos    = vertices[4*i + 2],
                .colour = {1, 1, 1, 1},
                .uv     = {0, 1}
            },
            (RenderVertexTriangle){
                .pos    = vertices[4*i + 3],
                .colour = {1, 1, 1, 1},
                .uv     = {1, 1}
            },
            texture
        );
    }
}

void render_draw_circle(Renderer *r, Circle circle) {
    // render_switch_circle(r);
    render_push_circle(
        r,
        (RenderVertexCircle){
            .pos        = {circle.x, circle.y, 0},
            .colour     = {1, 1, 1, 1},
            .radius     = circle.radius,
            .fade       = 0.95f,
            .fullness   = 1.0f
        }
    );
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
    // TODO: flip bitmap?
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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    GLint swizzles[4] = {GL_ONE, GL_ONE, GL_ONE, GL_RED};
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzles);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, 512, 512, 0, GL_RED, GL_UNSIGNED_BYTE, tmp_bitmap);

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

        const Rectangle uv = {
            .x = c->x0 / 512.0f,
            .y = c->y0 / 512.0f,
            .width = (c->x1 - c->x0) / 512.0f,
            .height = (c->y1 - c->y0) / 512.0f
        };

        const Rectangle area = {
            .x = pos.x + c->xoff,
            .y = pos.y - c->yoff2,
            .width = c->x1 - c->x0,
            .height = c->y1 - c->y0
        };

        render_draw_rectangle_uv(r, area, uv, font->texture);
        pos.x += c->xadvance;
    }
}
