#ifndef __FAMILIAR_SHADER_H__
#define __FAMILIAR_SHADER_H__

#include <glad/glad.h>
#include <stdbool.h>
#include <stb/stb_ds.h>

typedef struct {
    GLuint id;
    struct {
        // NOTE: array types have a suffix of "[0]"
        char *key;  // name
        GLint value;// location
    } *uniforms; // needs to be a reference for `hm*`macros to work (without using opaque pointers on temporary hashmaps)
                 // allocated in an arena
} Shader;

void shader_init(Shader *shader, const char *vertex_path, const char *fragment_path);
void shader_free(Shader *shader);

const char *_read_file(const char *file_path);  // reads a file to a string
// GL_COMPILE_STATUS
// GL_LINK_STATUS
bool _error_check(GLuint shader, GLenum pname); // checks for errors in shader source

unsigned int shader_make(const char *vertex_path, const char *fragment_path); // builds the shader program
void shader_update_locations(Shader *shader);

// TODO: uniform update wrappers

#endif // __FAMILIAR_SHADER_H__
