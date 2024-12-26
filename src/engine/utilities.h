#ifndef __FAMILIAR_UTILITIES_H__

float lerp_f32(float a, float b, float t);

// Note: allocated on the heap.
// caller must free
const char *read_file(const char *file_path);  // reads a file to a string

#endif // __FAMILIAR_UTILITIES_H__
