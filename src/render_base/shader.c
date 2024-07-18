#include "shader.h"

#include <glad/glad.h>
#include <stdio.h>
#include <stdlib.h>

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

bool _error_check_shader(unsigned int shader) {
    int success;
    char info_log[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(shader, 512, NULL, info_log);
        fprintf(stderr, "shader source compilation error:\n\t%s", info_log);
        return false;
    }

    return true;
}

bool _error_check_program(unsigned int program) {
    int success;
    char info_log[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(program, 512, NULL, info_log);
        fprintf(stderr, "shader program linking error:\n\t%s", info_log);
        return false;
    }

    return true;
}

unsigned int shader_make(const char *vertex_path, const char *fragment_path) {
    const char *vertex_source = _read_file(vertex_path);
    const char *fragment_source = _read_file(fragment_path);

    unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
    unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(vertex, 1, &vertex_source, NULL);
    glShaderSource(fragment, 1, &fragment_source, NULL);
    glCompileShader(vertex);
    glCompileShader(fragment);
    _error_check_shader(vertex);
    _error_check_shader(fragment);

    unsigned int program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);
    _error_check_program(program);


    glDeleteShader(vertex);
    glDeleteShader(fragment);

    return program;
}
