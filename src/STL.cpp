#include "STL.h"
#include <stdio.h>

Mesh** loadSTLShape(const char* fileName, u32* count)
{
  vec3 zero = {0.0f,0.0f,0.0f};
  return loadSTLShape( fileName, zero ,count);
}
Mesh** loadSTLShape(const char* fileName, vec3 offset, u32* count)
{
  FILE* fileHandle = fopen(fileName, "rb");
  if (!fileHandle)
    {
      printf("WARNING: CANNOT LOAD STL FILE: %s\n", fileName);
      return NULL;
    }
  
  u8 header[80];
  fread((void*)header, sizeof(u8), 80, fileHandle);
  u32 triangleCount;
  u16 attributeByteCount;
  fread((void*)&triangleCount, sizeof(u32), 1, fileHandle);

  Mesh* mesh = (Mesh*)calloc(sizeof(Mesh), 1);
  mesh->vertices = malloc(sizeof(Vertex) * triangleCount * 3);

  Vertex* vertex = (Vertex*)mesh->vertices;
  for (u32 i = 0; i < triangleCount; i++)
    {
      //TODO: vertex normal
      u32 index;
      vec3 normal;
      fread((void*)&normal,                    sizeof(vec3), 1, fileHandle);
      fread((void*)&(vertex+0)->pos,        sizeof(vec3), 1, fileHandle);
      fread((void*)&(vertex+1)->pos,        sizeof(vec3), 1, fileHandle);
      fread((void*)&(vertex+2)->pos,        sizeof(vec3), 1, fileHandle);      
      fread((void*)&attributeByteCount, sizeof(u16),  1, fileHandle);
      (vertex + 0)->normal = normal;
      (vertex + 1)->normal = normal;
      (vertex + 2)->normal = normal;
      vertex += 3;
    }

  mesh->vertexCount = triangleCount * 3;
  mesh->indices = (u32*)malloc(sizeof(u32) * mesh->vertexCount);
  if (mesh->rendererData.indexCount == 0)
    {
      for (int i = 0; i < mesh->vertexCount; i++)
	{
	  mesh->indices[i] = i; 
	}
    }
  mesh->rendererData.indexCount = mesh->vertexCount;
  mesh->indexCount = mesh->vertexCount;

  mesh->visible = true;
  fclose(fileHandle);

  Mesh** meshes = (Mesh**)malloc(sizeof(Mesh*) * 1);
  *count = 1;
  meshes[0] = mesh;
  
  return meshes;
}

