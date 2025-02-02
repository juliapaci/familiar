#ifndef __FAMILIAR_SHADER_H__
#define __FAMILIAR_SHADER_H__

#include <glad/glad.h>
#include <stdbool.h>
#include <stb/stb_ds.h>

// TODO: dont use relative pathing
#define SHADER_PATH "src/engine/shaders/"

#define SHADER_TRIANGLE SHADERPATH_CREATE(SHADER_PATH "triangle")
#define SHADER_CIRCLE   SHADERPATH_CREATE(SHADER_PATH "circle")
#define SHADERPATH_CREATE_FULL(name) (ShaderPaths){ \
    .vertex     = name".vs",                        \
    .fragment   = name".fs",                        \
    .geometry   = name".gs"                         \
}
#define SHADERPATH_CREATE(name) (ShaderPaths){      \
    .vertex     = name".vs",                        \
    .fragment   = name".fs",                        \
    .geometry   = NULL                              \
}

// leaving as NULL should result in the default shader path
// (depending on the shader type of shader program)
typedef struct {
    const char *vertex;
    const char *fragment;
    const char *geometry;
} ShaderPaths;

// TODO: should we keep `ShaderPaths` tied to this?
typedef struct {
    GLuint id;
    struct {
        // NOTE: array types have a suffix of "[0]"
        char *key;  // name
        GLint value;// location
    } *uniforms; // needs to be a reference for `hm*`macros to work (without using opaque pointers on temporary hashmaps)
                 // allocated in an arena
} Shader;

void shader_init(Shader *shader, ShaderPaths paths);
void shader_free(Shader *shader);

unsigned int shader_make(ShaderPaths *paths); // builds the shader program
void shader_update_locations(Shader *shader);

// TODO: uniform update wrappers

#endif // __FAMILIAR_SHADER_H__
