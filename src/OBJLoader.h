#ifndef __OBJ_LOADER_HEADER
#define __OBJ_LOADER_HEADER

#include "Mesh.h"
Mesh* loadOBJFile(const char* fileName);
Mesh** loadOBJFile_Loader(const char* filename, u32* count);

#endif
