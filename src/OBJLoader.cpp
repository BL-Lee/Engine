  #include <stdio.h>
#include <iostream>
#include <strings.h>
#if 0
Mesh* loadOBJFile(const char* fileName)
{
  FILE* fileHandle = fopen(fileName, "rb");
  if (!fileHandle)
    {
      printf("WARNING: CANNOT LOAD OBJ FILE: %s\n", fileName);
      return NULL;
    }

  //TODO: arraylist because obj doesnt use a header
  u32 triangleCount = 2503;
  u32 indexCount = 4968 * 3;

  Mesh* mesh = (Mesh*)malloc(sizeof(Mesh));
  
  mesh->vertices = (Vertex*)calloc(triangleCount, sizeof(Vertex));
  mesh->indices = (u32*)malloc(sizeof(u32) * indexCount);
  
  Vertex* vertex = mesh->vertices;  
  u32* index = mesh->indices;
  vec2 texCoords[2048];
  vec2* coord = texCoords;  
  vec3 normals[2048];
  vec3* norm = normals;

  
  char* buffer = (char*)malloc(sizeof(char) * 512);
  size_t linecapp;
  ssize_t line;
  while ((line = getline(&buffer, &linecapp, fileHandle)) > 0)
    {
      //vertex
      //texture coords
      if (strncmp(buffer, "vt", 2) == 0)
	{	  
	  sscanf(buffer,"vt %f %f", &coord->x, &coord->y);
	  //printf("coord: %f %f \n",coord->x, coord->y);
	  coord++;
	}
      //normal
      else if (strncmp(buffer, "vn", 2) == 0)
	{	  
	  sscanf(buffer,"vn %f %f %f", &norm->x, &norm->y, &norm->z);
	  //printf("norm: %f %f %f\n", norm->x, norm->y, norm->z);
	  norm++;
	}
      else if (strncmp(buffer, "v", 1) == 0)
	{
	  sscanf(buffer,"v %f %f %f", &vertex->pos.x, &vertex->pos.y, &vertex->pos.z);
	  //printf("vertex: %f %f %f\n", vertex->pos.x, vertex->pos.y, vertex->pos.z);
	  vertex++;
	}

      //face index
      else if (strncmp(buffer, "f", 1) == 0)
	{
	  u32 indices[3];
	  /*
	  u32 texIndex[3];
	  u32 normIndex[3];
	  sscanf(buffer, "f %d/%d/%d %d/%d/%d %d/%d/%d",
		 &indices[0], &texIndex[0], &normIndex[0],
		 &indices[1], &texIndex[1], &normIndex[1],
		 &indices[2], &texIndex[2], &normIndex[2]);
	  for (int i = 0; i < 3; i++)
	    {
	      indices[i]--;
	      texIndex[i]--;
	      normIndex[i]--;
	    }
	  */
	  sscanf(buffer, "f %d %d %d",
		 &indices[0], &indices[1], &indices[2]);

	  
	  //printf("index:");
	  for (int i = 0; i < 3; i++)
	    {
	      //mesh->vertices[indices[i]].normal = normals[normIndex[i]];
	      //mesh->vertices[indices[i]].texCoord = texCoords[texIndex[i]];
	      indices[i]--;
	      *index = indices[i];
	      //printf("%d ", *index);
	      index++;
	    }
	  vec3 a = mesh->vertices[indices[0]].pos;
	  vec3 b = mesh->vertices[indices[1]].pos;
	  vec3 c = mesh->vertices[indices[2]].pos;
	  vec3 t = Normalize(Cross(b - a, c - b));
	  //vec3 o = Normalize(Cross(t, c - a);
	  for (int i = 0; i < 3; i++)
	    {
	      mesh->vertices[indices[i]].normal = t;
	    }
	  
	  //printf("\n");
	  
	}
    }

  mesh->vertexCount = triangleCount;
  mesh->indexCount = indexCount;
  free(buffer);
  fclose(fileHandle);
  
  return mesh;

}
#endif
Mesh** loadOBJFile_Loader(const char* filename, u32* count)
{
  objl::Loader loader;
  bool loaded = loader.LoadFile(filename);
  if (!loaded)
    {
      printf("Failed to load OBJ file: %s\n", filename);
      return NULL;
    }

  Mesh** meshes = (Mesh**)malloc(sizeof(Mesh*) * loader.LoadedMeshes.size());
  for (int i = 0; i < loader.LoadedMeshes.size(); i++)
    {
      Mesh* outMesh = (Mesh*)calloc(sizeof(Mesh),1);
      outMesh->visible = true;
      meshes[i] = outMesh;
      objl::Mesh mesh = loader.LoadedMeshes[i];
      std::cout << mesh.MeshMaterial.name << std::endl;

      //copy vertices
      outMesh->vertices = (void*)malloc(sizeof(Vertex) * mesh.Vertices.size());
      Vertex* vertices = (Vertex*)outMesh->vertices;
      outMesh->vertexCount = mesh.Vertices.size();
      for (int j = 0; j < mesh.Vertices.size(); j++)
	{
	  vertices[j].pos.x = mesh.Vertices[j].Position.X;
	  vertices[j].pos.y = mesh.Vertices[j].Position.Y;
	  vertices[j].pos.z = mesh.Vertices[j].Position.Z;

	  vertices[j].normal.x = mesh.Vertices[j].Normal.X;
	  vertices[j].normal.y = mesh.Vertices[j].Normal.Y;
	  vertices[j].normal.z = mesh.Vertices[j].Normal.Z;

	  vertices[j].texCoord.x = mesh.Vertices[j].TextureCoordinate.X;
	  vertices[j].texCoord.y = mesh.Vertices[j].TextureCoordinate.Y;
	}
      
	     //copy indices
      outMesh->indices = (u32*)malloc(sizeof(u32) * mesh.Indices.size());
      for (int j = 0; j < mesh.Indices.size(); j++)
	{
	  outMesh->indices[j] = mesh.Indices[j];
	}
      outMesh->indexCount = mesh.Indices.size();

      //copy material
      objl::Material mat = mesh.MeshMaterial;
      if (mat.name.length() > 2) {
	outMesh->material.name = (char*)malloc(mat.name.length() + 1);
	strcpy(outMesh->material.name, mat.name.c_str());
      }
      memcpy(&outMesh->material.ambient, &mat.Ka, sizeof(vec3) * 3 + sizeof(f32) * 3 + sizeof(int));

      if (mat.map_Ka.length() > 2)
	{
	  outMesh->material.ambientMap = (char*)malloc(mat.map_Ka.length() + 1);
	  std::replace(mat.map_Ka.begin(), mat.map_Ka.end(), '\\', '/');
	  strcpy(outMesh->material.ambientMap, mat.map_Ka.c_str());
	}
      if (mat.map_Kd.length() > 2)
	{
	  outMesh->material.diffuseMap = (char*)malloc(mat.map_Kd.length() + 1);
	  std::replace(mat.map_Kd.begin(), mat.map_Kd.end(), '\\', '/');
	  strcpy(outMesh->material.diffuseMap, mat.map_Kd.c_str());
	}
      if (mat.map_Ks.length() > 2)
	{
	  outMesh->material.specularTextureMap = (char*)malloc(mat.map_Ks.length() + 1);
	  std::replace(mat.map_Ks.begin(), mat.map_Ks.end(), '\\', '/');
	  strcpy(outMesh->material.specularTextureMap, mat.map_Ks.c_str());
	}
      if (mat.map_Ns.length() > 2)
	{
	  outMesh->material.specularHighlightMap = (char*)malloc(mat.map_Ns.length() + 1);
	  std::replace(mat.map_Ns.begin(), mat.map_Ns.end(), '\\', '/');
	  strcpy(outMesh->material.specularHighlightMap, mat.map_Ns.c_str());
	}
      if (mat.map_d.length() > 2)
	{
	  outMesh->material.alphaMap = (char*)malloc(mat.map_d.length() + 1);
	  strcpy(outMesh->material.alphaMap, mat.map_d.c_str());
	}
      if (mat.map_bump.length() > 2)
	{
	  outMesh->material.bumpMap = (char*)malloc(mat.map_bump.length() + 1);
	  strcpy(outMesh->material.bumpMap, mat.map_bump.c_str());
	}
      if (outMesh->material.diffuseMap)
	{
	  //printf("Loading texture:%s\n", outMesh->material.diffuseMap);
	  /*
	  u8* data = loadTexture(&outMesh->rendererData.diffuseTextureKey,
				 outMesh->material.diffuseMap);
	  //printf("Key provided: %d\n", outMesh->rendererData.diffuseTextureKey);
	  if (data)
	  stbi_image_free(data);*/
	  outMesh->rendererData.diffuseTextureKey = requestTextureKey(outMesh->material.diffuseMap);
	}

    }
  *count = loader.LoadedMeshes.size();
  return meshes;
}
