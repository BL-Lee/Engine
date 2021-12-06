#ifndef __RENDERER_HEADER
#define __RENDERER_HEADER

#include "Mesh.h"

void initRenderer();
u8* loadTexture(u32* textureKey, const char* textureFile);
u32 requestTextureKey(const char* textureName);
void deleteTexture(const char* textureName);
void deleteRenderer();
void addMesh(Mesh* mesh, const char* vertexShader, const char* fragmentShader);
void drawMesh(Mesh* mesh, vec3 position, vec3 rotation, vec3 scale);

#endif
