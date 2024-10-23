#include "general.h"
#include <camera.h>
#include <shader.h>

#include <stdio.h>
#include <limits.h>

float delta_time = 1.0;
float last_frame = 0.0f;

void update_delta_time(void) {
    float current_frame = glfwGetTime();
    delta_time = current_frame - last_frame;
    last_frame = current_frame;
}

GLFWwindow *init_window(const char *name) {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(800, 600, name, NULL, NULL);
    set_framebuffer_size_callback(window);
    if(window == NULL) {
        fprintf(stderr, "failed to create glfw window\n");
        glfwTerminate();
        return NULL;
    }
    glfwMakeContextCurrent(window);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        fprintf(stderr, "failed to initialize glad\n");
        glfwTerminate();
        return NULL;
    }

    glViewport(0, 0, 800, 600);

    return window;
}

void _framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
}
extern void set_framebuffer_size_callback(GLFWwindow *window);

void process_general_input(GLFWwindow *window, bool *wireframe) {
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    else if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        *wireframe = !*wireframe;
        glPolygonMode(GL_FRONT_AND_BACK, *wireframe ? GL_LINE : GL_FILL);
    }
}

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
	if (_white_texture == UINT32_MAX) {
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
	}

	return _white_texture;
}
