#ifndef __SHADER_H__
#define __SHADER_H__

#include <stdbool.h>

const char *_read_file(const char *file_path);  /// reads a file to a string
bool _error_check_shader(unsigned int shader);  /// checks for errors in shader source
bool _error_check_program(unsigned int program);/// checks (linking?) errors in shader program

unsigned int shader_make(const char *vertex_path, const char *fragment_path); // builds the shader program

#endif // __SHADER_H__
