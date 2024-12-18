#include "shader.h"

#include <stdio.h>
#include <stdlib.h>

void shader_init(Shader *shader, const char *vertex_path, const char *fragment_path) {
    *shader = (Shader){0};

    shader->id = shader_make(vertex_path, fragment_path);
    sh_new_arena(shader->uniforms);
    shdefault(shader->uniforms, -1);
    shader_update_locations(shader);
    glUseProgram(shader->id);
}

void shader_free(Shader *shader) {
    glDeleteProgram(shader->id);
    shfree(shader->uniforms);
}

const char *_read_file(const char *file_path) {
    FILE *file = fopen(file_path, "rb");
    if(file == NULL)
        return NULL;

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *buffer = malloc(length + 1);
    if(buffer == NULL) {
        fclose(file);
        return NULL;
    }

    fread(buffer, 1, length, file);
    buffer[length] = '\0';

    fclose(file);
    return buffer;
}

bool _error_check(GLuint shader, GLenum pname) {
    GLint success;
    glGetShaderiv(shader, pname, &success);

    if(!success) {
        GLchar info_log[512];
        glGetShaderInfoLog(shader, 512, NULL, info_log);
        fprintf(stderr, "shader error:\n\t%s", info_log);
    }

    return success;
}

GLuint shader_make(const char *vertex_path, const char *fragment_path) {
    const char *vertex_source = _read_file(vertex_path);
    if(vertex_source == NULL) {
        fprintf(stderr, "cannot find vertex shader \"%s\"", vertex_path);
        return 0;
    }
    const char *fragment_source = _read_file(fragment_path);
    if(fragment_source == NULL) {
        fprintf(stderr, "cannot find fragment shader \"%s\"", fragment_path);
        return 0;
    }

    const GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    const GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(vertex, 1, &vertex_source, NULL);
    glShaderSource(fragment, 1, &fragment_source, NULL);
    glCompileShader(vertex);
    glCompileShader(fragment);
    _error_check(vertex, GL_COMPILE_STATUS);
    _error_check(fragment, GL_COMPILE_STATUS);

    const GLuint program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);
    _error_check(program, GL_LINK_STATUS);

    glDetachShader(program, vertex);
    glDetachShader(program, fragment);
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    free((void *)vertex_source);
    free((void *)fragment_source);

    return program;
}

void shader_update_locations(Shader *shader) {
    GLint uniform_amount = 0;
    glGetProgramInterfaceiv(shader->id, GL_UNIFORM, GL_ACTIVE_RESOURCES, &uniform_amount);

    const GLenum properties[3] = {GL_TYPE, GL_NAME_LENGTH, GL_LOCATION};
    for(GLint i = 0; i < uniform_amount; i++) {
        GLint values[3]; // type, size, location
        glGetProgramResourceiv(shader->id, GL_UNIFORM, i, 3, properties, 3, NULL, values);

        char name[values[1]];
        glGetProgramResourceName(shader->id, GL_UNIFORM, i, values[1], NULL, name);

        // TODO: type check to remove array "[0]" suffix
        // if(values[0] == GL_)
        shput(shader->uniforms, name, values[2]);
    }
}
