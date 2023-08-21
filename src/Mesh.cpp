#include <stdio.h>
#include "Renderer.h"
#include "STL.h"
#include "OBJLoader.h"
//increment this every time you update the structure
#define MESH_FILE_VERSION 1
//#define DEBUG_PRINT_MESH_SERIALIZATION 1
/*
  
  Mesh file format:
     FileHeader:
         totalMeshes * meshHeader
     for i in meshheaders
         Vertices:
            vec3 pos, vec3 normal, vec2 texCoord, u32 meshIndex
         Indices:
            u32 a, u32 b, u32 c, u32 meshIndex
	 u16 Material name length
	 char Material name

  Material file format:
      name newline
      values
      stuff newlines
      

*/
struct MeshFileHeader
{
  u16 version;
  u64 totalFileSize;
  u32 totalMeshes;  
};
struct MeshHeader
{
  u32 totalVertices;
  u32 totalIndices;  
};

void deleteMaterial(Material mat)
{
  if (mat.name)
    {
      free(mat.name);
    }
  if (mat.ambientMap)
    {
      deleteTexture(mat.ambientMap);
      free(mat.ambientMap);    
    }
  if (mat.specularTextureMap)
    {
      deleteTexture(mat.specularTextureMap);
      free(mat.specularTextureMap);
    }
  if (mat.specularHighlightMap)
    {
      deleteTexture(mat.specularHighlightMap);
      free(mat.specularHighlightMap);
    }
  if (mat.alphaMap)
    {
      deleteTexture(mat.alphaMap);
      free(mat.alphaMap);
    }  
  if (mat.bumpMap)
    {
      deleteTexture(mat.bumpMap);
      free(mat.bumpMap);
    }
}

void deleteMesh(Mesh* mesh)
{
  //TODO: Free stuff from VAO
  
  glDeleteProgram(mesh->rendererData.shaderProgramKey);
  //glDeleteVertexArrays(1, &mesh->rendererData.vertexArrayKey);
  //glDeleteBuffers(1, &mesh->rendererData.vertexBufferKey);
  //glDeleteBuffers(1, &mesh->rendererData.indexBufferKey);
  //glDeleteTextures(1, &mesh->rendererData.ambientTextureKey);
  //glDeleteTextures(1, &mesh->rendererData.diffuseTextureKey);
  free(mesh->vertices);
  if (mesh->indices)
    {
      free(mesh->indices);
    }
  deleteMaterial(mesh->material);  
  //free(mesh);
}

void fputsNewline(const char* str, FILE* fileHandle)
{
  if (str)
    {
      Assert(strlen(str))
      fputs(str, fileHandle);
    }
  fputc('\n', fileHandle);  
}

void serializeMeshes(Mesh** meshes, u32 meshCount, const char* fileOutput)
{
  FILE* fileHandle = fopen(fileOutput, "wb");
  if (!fileHandle)
    {
      fprintf(stderr,"WARNING: CANNOT OPEN MESH FILE TO WRITE %s\n", fileOutput);
      return;
    }
  
  //Write file header
  u32 totalFileSize = sizeof(MeshHeader) * meshCount + sizeof(MeshFileHeader);
  for (int i = 0; i < meshCount; i++)
    {
      totalFileSize += meshes[i]->vertexCount * sizeof(Vertex);
      totalFileSize += meshes[i]->indexCount * sizeof(u32);
    }  
  MeshFileHeader header = {};
  header.version = MESH_FILE_VERSION;
  header.totalFileSize = totalFileSize;
  header.totalMeshes = meshCount;
  fwrite(&header, sizeof(MeshFileHeader), 1, fileHandle);

  #ifdef DEBUG_PRINT_MESH_SERIALIZATION
  printf("Serializing #%d meshes...\n", meshCount);
  #endif
  
  //Write mesh headers
  MeshHeader mHeaders[meshCount];
  for (int i = 0; i < meshCount; i++)
    {
      mHeaders[i].totalVertices = meshes[i]->vertexCount;
      mHeaders[i].totalIndices = meshes[i]->indexCount;
    }
  fwrite(mHeaders, sizeof(MeshHeader), meshCount, fileHandle);

  //Write meshes
  for (u32 i = 0; i < meshCount; i++)
    {
      Mesh* mesh = meshes[i];
      for (int j = 0; j < mesh->vertexCount; j++)
	{
	  fwrite((Vertex*)mesh->vertices + j, sizeof(Vertex) - sizeof(float), 1, fileHandle);
	}

      fwrite(mesh->indices, sizeof(u32), mesh->indexCount, fileHandle);
      
#ifdef DEBUG_PRINT_MESH_SERIALIZATION      
      printf("\tMesh #%d\n\tVertices: %d Indices %d\n", i, mesh->vertexCount, mesh->indexCount);      
#endif
      
      //Write Material name
      if (mesh->material.name)
	{
	  u16 size = strlen(mesh->material.name);
	  fwrite(&size, sizeof(u16), 1, fileHandle);
	  fputs(mesh->material.name, fileHandle);
#ifdef DEBUG_PRINT_MESH_SERIALIZATION      
	  printf("\tMaterial: %s\n", mesh->material.name);
#endif
	}
      else
	{
	  u16 zero = 0;
	  fwrite(&zero, sizeof(u16), 1, fileHandle);
	}
      if (mesh->material.name)
	{
	  FILE* matFile = fopen(mesh->material.name, "wb");      
	  if (matFile)
	    {
	      fputsNewline(mesh->material.name, matFile);
	      fwrite(&mesh->material.ambient,
		     sizeof(vec3) * 3 + sizeof(f32) * 3 + sizeof(int), 1, matFile);
	      fputsNewline(mesh->material.ambientMap, matFile);
	      fputsNewline(mesh->material.diffuseMap, matFile);
	      fputsNewline(mesh->material.specularTextureMap, matFile);
	      fputsNewline(mesh->material.specularHighlightMap, matFile);
	      fputsNewline(mesh->material.alphaMap, matFile);
	      fputsNewline(mesh->material.bumpMap, matFile);
	      fclose(matFile);
	    }
	}
    }
  
  fclose(fileHandle);
}

void allocAndStrCpy(char** dst, char* src)
{
  if (strlen(src) > 1)
    {
      *dst = (char*)malloc(strlen(src) + 1);  
      strcpy(*dst, src);
      size_t next = strcspn(*dst, "\r\n");
      if ((*dst)[next] != 0)
	{
	  (*dst)[next] = 0;
	}
      next = strcspn(*dst, "\\");      
      while ((*dst)[next] != 0)
	{
	  (*dst)[next] = '/';
	  next = strcspn(*dst, "\\");
	}
    }
}

Mesh** deserializeMeshes(const char* fileInput, u32* meshCount)
{
  //Open file
  FILE* fileHandle = fopen(fileInput, "rb");
  if (!fileHandle)
    {
      fprintf(stderr,"WARNING: CANNOT OPEN MESH FILE TO READ %s\n", fileInput);
      return NULL;
    }
  
  MeshFileHeader header = {};
  fread((void*)&header, sizeof(MeshFileHeader), 1, fileHandle);
  //Validate file
  if (header.version != MESH_FILE_VERSION)
    {
      fprintf(stderr,"WARNING: FILE NOT MESH OR MESH FILE %s OUT OF DATE. FILE VERSION: %d", fileInput, header.version);
      return NULL;
    }  

  //Read header
  MeshHeader mHeaders[header.totalMeshes];
  fread(mHeaders, sizeof(MeshHeader), header.totalMeshes, fileHandle);
  
  //Read meshes
  Mesh** meshes = (Mesh**)malloc(sizeof(Mesh*) * header.totalMeshes);
  u32 meshKey = 0;
  for (int i = 0; i < header.totalMeshes; i++)
    {
      meshes[i] = (Mesh*)calloc(sizeof(Mesh), 1);
      meshes[i]->visible = true;
      //Load vertices
      meshes[i]->vertices = malloc(sizeof(Vertex) * mHeaders[i].totalVertices);
      meshes[i]->vertexCount = mHeaders[i].totalVertices;
      
      for (int j = 0; j < mHeaders[i].totalVertices; j++)
	{
	  //subtract float because of texUnit. Dont want to serialize that
	  fread((Vertex*)meshes[i]->vertices + j, sizeof(Vertex) - sizeof(float), 1, fileHandle);
	  ((Vertex*)meshes[i]->vertices)[j].texUnit = 0;
	}
      
      //Load indices
      meshes[i]->indices = (u32*)malloc(sizeof(u32) * mHeaders[i].totalIndices);
      meshes[i]->indexCount = mHeaders[i].totalIndices;  
      fread(meshes[i]->indices, sizeof(u32), mHeaders[i].totalIndices, fileHandle);

      //Load material
      u16 matNameLength;
      fread(&matNameLength, sizeof(u16), 1, fileHandle);
      char* matName = (char*)malloc(matNameLength + 1);
      fread(matName, matNameLength, 1, fileHandle);
      matName[matNameLength] = 0;
      
      //Read material
      //TODO: find file function
      FILE* matFile = fopen(matName, "rb");
      if (matFile)
	{
	  char buffer[512];
	  meshes[i]->material.name = matName;
	  fgets(buffer, 512, matFile);
	  fread(&meshes[i]->material.ambient,
		sizeof(vec3) * 3 + sizeof(f32) * 3 + sizeof(int), 1, matFile);
	  
	  fgets(buffer, 512, matFile);	  
	  allocAndStrCpy(&meshes[i]->material.ambientMap, buffer);
	  fgets(buffer, 512, matFile);	  
	  allocAndStrCpy(&meshes[i]->material.diffuseMap, buffer);
	  fgets(buffer, 512, matFile);
	  allocAndStrCpy(&meshes[i]->material.specularTextureMap, buffer);
	  fgets(buffer, 512, matFile);
	  allocAndStrCpy(&meshes[i]->material.specularHighlightMap, buffer);
	  fgets(buffer, 512, matFile);
	  allocAndStrCpy(&meshes[i]->material.alphaMap, buffer);
	  fgets(buffer, 512, matFile);
	  allocAndStrCpy(&meshes[i]->material.bumpMap, buffer);
	  fclose(matFile);
	}
      else
	{
	  free(matName);
	}
      if (meshes[i]->material.diffuseMap)
	{
	  meshes[i]->rendererData.diffuseTextureKey = requestTextureKey(meshes[i]->material.diffuseMap);	    
	}
    }

  *meshCount = header.totalMeshes;
  fclose(fileHandle);
  return meshes;
}

//TODO: please find a better solution for the skinned vertex situation
void calculateNormals(Mesh* mesh)
{
  if (mesh->skinnedMesh)
    {
      SkinnedVertex* vertices = (SkinnedVertex*)mesh->vertices;	  
      for (int i = 0; i < mesh->indexCount; i += 3)
	{
	  vec3 normal = Cross(vertices[mesh->indices[i + 2]].pos - vertices[mesh->indices[i + 1]].pos,
			      vertices[mesh->indices[i + 1]].pos - vertices[mesh->indices[i + 0]].pos);
	  vertices[mesh->indices[i + 0]].normal += normal;
	  vertices[mesh->indices[i + 1]].normal += normal;
	  vertices[mesh->indices[i + 2]].normal += normal;
	}
      for (int i = 0; i < mesh->vertexCount; i += 3)
	{
	  vertices[i].normal = Normalize(vertices[i].normal);
	}
    }
  else
    {
      Vertex* vertices = (Vertex*)mesh->vertices;
      for (int i = 0; i < mesh->indexCount; i += 3)
	{
	  vec3 normal = Cross(vertices[mesh->indices[i + 2]].pos - vertices[mesh->indices[i + 1]].pos,
			      vertices[mesh->indices[i + 1]].pos - vertices[mesh->indices[i + 0]].pos);
	  vertices[mesh->indices[i + 0]].normal += normal;
	  vertices[mesh->indices[i + 1]].normal += normal;
	  vertices[mesh->indices[i + 2]].normal += normal;
	}
      for (int i = 0; i < mesh->vertexCount; i += 3)
	{
	  vertices[i].normal = Normalize(vertices[i].normal);
	}
    }
}

Mesh** loadModel(const char* modelFile, u32* meshCount)
{
  const char* suffix = strrchr(modelFile, '.');
  if (!suffix)
    {
        fprintf(stderr,"WARNING: UNRECOGNIZED FILE TYPE : %s\n", suffix);
    }
  if (strcmp(".stl", suffix) == 0)
    {
      return loadSTLShape(modelFile, meshCount);
    }
  if (strcmp(".obj", suffix) == 0)
    {
      return loadOBJFile_Loader(modelFile, meshCount);
    }
  if (strcmp(".meshes", suffix) == 0)
    {
      return deserializeMeshes(modelFile, meshCount);
    }
  fprintf(stderr,"WARNING: UNRECOGNIZED FILE TYPE : %s\n", suffix);
  return NULL;
}
bool loadMaterial(const char* filename, Material* material)
{
  FILE* fileHandle = fopen(filename, "r");
  if (!fileHandle)
    {
      fprintf(stderr,"WARNING: Cannot open entity file: %s\n", filename);
      return false;
    }
  material->ambientMap = 0;
  material->diffuseMap = 0;
  material->specularTextureMap = 0;
  material->specularHighlightMap = 0;
  material->alphaMap = 0;
  material->bumpMap = 0;
  material->name = (char*)malloc(strlen(filename) + 1);
  strcpy(material->name, filename);    
    char buffer[512];
  while (fgets(buffer, 512, fileHandle))
    {
      if (strstr(buffer, "#ambient"))
	{
	  vec3 values;
	  u32 ret = sscanf(buffer, "#ambient %f %f %f",
		 &values.x,
		 &values.y,
		 &values.z);
	  if (ret != 3)
	    {
	      fprintf(stderr,"WARNING: invalid ambient tag in %s file\n", filename);
	    }
	  else
	    material->ambient = values;
	  // printf("entity pos %f %f %f\n", entity->position.x, entity->position.y, entity->position.z); 
	}
      else if (strstr(buffer, "#diffuse"))
	{
	  vec3 values;
	  u32 ret = sscanf(buffer, "#diffuse %f %f %f",
		 &values.x,
		 &values.y,
		 &values.z);
	  if (ret != 3)
	    {
	      fprintf(stderr,"WARNING: invalid diffuse tag in %s file\n", filename);
	    }
	  else
	    material->diffuse = values;
	  
	}
      else if (strstr(buffer, "#specular"))
	{
	 
	  vec3 values;
	  u32 ret = sscanf(buffer, "#specular %f %f %f",
		 &values.x,
		 &values.y,
		 &values.z);
	  if (ret != 3)
	    {
	      fprintf(stderr,"WARNING: invalid diffuse tag in %s file\n", filename);
	    }
	  else
	    material->specular = values;
	  
	}
      else if (strstr(buffer, "#shininess"))
	{
	  float value;
	  u32 ret = sscanf(buffer, "#shininess %f", &value);
	  if (ret != 1)
	    {
	      fprintf(stderr,"WARNING: invalid shininess tag in %s file\n", filename);
	    }
	  else
	    material->specularExponent = value;	   
	}
      else
	{
	  fprintf(stderr, "WARNING: invalid tag in material file: %s\n", filename);
	}
    }
  fclose(fileHandle);
  return true;
}
