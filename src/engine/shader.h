#ifndef __SHADER_H__
#define __SHADER_H__

#include <glad/glad.h>
#include <stdbool.h>
#include <stb/stb_ds.h>

typedef struct {
    GLuint id;
    struct {
        char *key;  // name
        GLint value;// location
    } *uniforms; // needs to be a reference for `hm*`macros to work (without using opaque pointers on temporary hashmaps)
} Shader;

const char *_read_file(const char *file_path);  /// reads a file to a string
bool _error_check_shader(unsigned int shader);  /// checks for errors in shader source
bool _error_check_program(unsigned int program);/// checks (linking?) errors in shader program

unsigned int shader_make(const char *vertex_path, const char *fragment_path); // builds the shader program
void shader_update_locations(Shader *shader);

#endif // __SHADER_H__
