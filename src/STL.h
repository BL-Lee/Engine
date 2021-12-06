#ifndef STL_LOADER_HEADER
#define STL_LOADER_HEADER

Mesh** loadSTLShape(const char* fileName, u32* count);
Mesh** loadSTLShape(const char* fileName, vec3 offset, u32* count);

#endif


