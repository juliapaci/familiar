#include "shader.h"

#include <engine/utilities.h>

#include <stdio.h>
#include <stdlib.h>

void shader_init(Shader *shader, ShaderPaths path) {
    *shader = (Shader){0};

    shader->id = shader_make(&path);
    // if(shader->id == 0)
    //     return !?
    sh_new_arena(shader->uniforms);
    shdefault(shader->uniforms, -1);
    shader_update_locations(shader);
    glUseProgram(shader->id);
}

void shader_free(Shader *shader) {
    glDeleteProgram(shader->id);
    shfree(shader->uniforms);
}

// pname:
//  GL_COMPILE_STATUS
//  GL_LINK_STATUS
bool _error_check(GLuint shader, GLenum pname) {
    GLint success = GL_FALSE;
    if(pname == GL_COMPILE_STATUS)
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    else if(pname == GL_LINK_STATUS)
        glGetProgramiv(shader, GL_LINK_STATUS, &success);

    if(!success) {
        GLchar info_log[512];
        glGetShaderInfoLog(shader, 512, NULL, info_log);
        fprintf(
            stderr,
            "shader %s error:\n\t%s",
            (const char *[]){"COMPILE", "LINK"}[pname - GL_COMPILE_STATUS],
            info_log
        );
    }

    return success;
}

GLuint _compile_shader(const char *path, GLenum kind) {
    const char *shader_source = read_file(path);
    if(shader_source == NULL) {
        fprintf(stderr, "cannot find shader \"%s\"", path);
        return 0;
    }

    const GLuint shader = glCreateShader(kind);
    glShaderSource(shader, 1, &shader_source, NULL);
    free((void *)shader_source);
    glCompileShader(shader);
    if(!_error_check(shader, GL_COMPILE_STATUS))
        return 0;

    return shader;
}

GLuint shader_make(ShaderPaths *paths) {
    const GLuint program = glCreateProgram();

    // TODO: NULL/non existent path check for default (what should default be?)
    if(paths->geometry != NULL) {
        const GLuint geometry = _compile_shader(paths->geometry, GL_GEOMETRY_SHADER);
        if(geometry == 0)
            return 0;
        glAttachShader(program, geometry);

        // TODO: is linking twice faster than checking paths->geomtry != NULL multiple times to free stuff at the end with everything else?
        glLinkProgram(program);
        _error_check(program, GL_LINK_STATUS);

        glDetachShader(program, geometry);
        glDeleteShader(geometry);
    }

    const GLuint vertex = _compile_shader(paths->vertex, GL_VERTEX_SHADER);
    if(vertex == 0)
        return 0;
    glAttachShader(program, vertex);

    const GLuint fragment = _compile_shader(paths->fragment, GL_FRAGMENT_SHADER);
    if(fragment == 0)
        return 0;
    glAttachShader(program, fragment);

    glLinkProgram(program);
    _error_check(program, GL_LINK_STATUS);

    glDetachShader(program, vertex);
    glDetachShader(program, fragment);
    glDeleteShader(vertex);
    glDeleteShader(fragment);

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
