#include "EntityRegistry.h"
//Finds an open slot in the entity pool, initializes it and returns a pointer to it
//Currently uses the entity's id as the key into the pool as a hash table
// hash key: id % MAX_REGISTRY_SIZE
Entity* requestNewEntity()
{
  EntityRegistry* g = globalEntityRegistry;
  for (int i = 0; i < MAX_REGISTRY_SIZE; i++)
    {
      g->entityCounter++;      
      u32 index = g->entityCounter % MAX_REGISTRY_SIZE;

      if (!g->occupiedIndices[index])
	{
	  g->occupiedIndices[index] = 1;
	  g->entities[index].id = g->entityCounter;
	  g->entities[index].visible = true;
	  g->entities[index].vertShader = NULL;
	  g->entities[index].fragShader = NULL;
	  g->entities[index].scale.x = 1.0;
	  g->entities[index].scale.y = 1.0;
	  g->entities[index].scale.z = 1.0;
	  sprintf(g->entities[index].name, "ENTITY %d", g->entityCounter);
	  return g->entities + index;
	}
      Assert(g->entityCounter != 0xFFFFFFFF);
    }
  Assert(0);
  return NULL;
}

Entity* requestNewEntity(const char* name)
{
  Entity* e = requestNewEntity();
  //e->name = (char*)malloc(strlen(name) + 1);
  #if DEBUG_GL
  if (strlen(name) > 255)
    {
      fprintf(stderr,"WARNING: ENTITY NAME > 255 CHARACTERS\n");
    }
  #endif
  strcpy(e->name, name);
  return e;
}

void setEntityName(Entity* e, const char* name)
{
#if DEBUG_GL
  if (strlen(name) > 255)
    {
      fprintf(stderr,"WARNING: ENTITY NAME > 255 CHARACTERS\n");
    }
#endif
  strcpy(e->name, name);
}

Entity* getEntityById(u32 id)
{
  if (globalEntityRegistry->occupiedIndices[id % MAX_REGISTRY_SIZE])
    {
      return globalEntityRegistry->entities + (id % MAX_REGISTRY_SIZE);
    }
  return NULL;
}

void deleteEntity(u32 id)
{
  if (!globalEntityRegistry->occupiedIndices[id % MAX_REGISTRY_SIZE])
    {
      fprintf(stderr,"ERROR: cannot delete entity with id %d. Already deleted\n", id);
      return;
    }
  //Assert(globalEntityRegistry->occupiedIndices[id % MAX_REGISTRY_SIZE]);
  Entity* e = &globalEntityRegistry->entities[id % MAX_REGISTRY_SIZE];
  if (e->vertShader)
      free(e->vertShader);
  if (e->fragShader)
      free(e->fragShader);
  for (int i = 0; i < e->meshCount; i++)
    {
      deleteMesh(e->meshes[i]);
    }
  free(e->meshes);
  globalEntityRegistry->occupiedIndices[id % MAX_REGISTRY_SIZE] = 0;
}

Entity* deserializeEntity(const char* filename)
{
  FILE* fileHandle = fopen(filename, "r");
  if (!fileHandle)
    {
      fprintf(stderr,"WARNING: Cannot open entity file: %s\n", filename);
      return NULL;
    }

  printf("DESERIALIZING %s\n", filename);
    
  Entity* entity = requestNewEntity(strrchr(filename,'/') + 1);
  vec3 zero = {0.0,0.0,0.0};
  vec3 one = {1.0,1.0,1.0};
  entity->position = zero;
  entity->scale = one;
  entity->rotation = zero;
  entity->velocity = zero;
  entity->angularVelocity = zero;
  entity->physicsEnabled = true;
  entity->gravityEnabled = false;
  entity->meshes = NULL;
  entity->meshCount = 0;
  entity->visible = true;

  char buffer[512];
  while (fgets(buffer, 512, fileHandle))
    {
      if (strstr(buffer, "#position"))
	{
	  entity->position = loadVec3Line("#position", buffer);
	}
      else if (strstr(buffer, "#scale"))
	{
	  entity->scale = loadVec3Line("#scale", buffer);
	}
      else if (strstr(buffer, "#rotation"))
	{
	  entity->rotation = loadVec3Line("#rotation", buffer);
	}
      else if (strstr(buffer, "#velocity"))
	{
	  entity->velocity = loadVec3Line("#velocity", buffer);
	}
      else if (strstr(buffer, "#angularVelocity"))
	{
	  entity->angularVelocity = loadVec3Line("#angularVelocity", buffer);;
	}
      else if (strstr(buffer, "#meshFile"))
	{	  
	  u32 ret = sscanf(buffer, "#meshFile %s", buffer);	  
	  entity->meshes = loadModel(buffer, &entity->meshCount);
	}
      else if (strstr(buffer, "#vert"))
	{
	  u32 ret = sscanf(buffer, "#vert %s", buffer);
	  entity->vertShader = (char*)malloc(strlen(buffer) + 1);
	  strcpy(entity->vertShader, buffer);
	}
      else if (strstr(buffer, "#frag"))
	{
	  u32 ret = sscanf(buffer, "#frag %s", buffer);
	  entity->fragShader = (char*)malloc(strlen(buffer) + 1);
	  strcpy(entity->fragShader, buffer);
	}
      else if (strstr(buffer, "#material"))
	{
	  u32 ret = sscanf(buffer, "#frag %s", buffer);	  
	  Material mat;
	  if (!loadMaterial(buffer, &mat))
	    {
	      for (int i = 0; i < entity->meshCount; i++)
		{
		  loadMaterial("res/materials/defaultMaterial.mat", &entity->meshes[i]->material);
		}
	    }
	  else
	    {
	      deleteMaterial(mat);
	      for (int i = 0; i < entity->meshCount; i++)
		{
		  loadMaterial(buffer, &entity->meshes[i]->material);
		}
	    }
	}
    }
  if (!entity->meshes[0]->material.name)
    {
      for (int i = 0; i < entity->meshCount; i++)
	{
	  loadMaterial("res/materials/defaultMaterial.mat", &entity->meshes[i]->material);
	}
    }
  if (entity->vertShader && entity->fragShader && entity->meshCount)
    {
      for (int i = 0; i < entity->meshCount; i++)
	{
	  addMesh(entity->meshes[i], entity->vertShader, entity->fragShader);
	}
    }
  setEntityAABBCollider(entity);
  return entity;
}



Entity* createEntityFromTriangles(Vertex* vertices, u32 vertexCount, u32* indices, u32 indexCount)
{
  Entity* entity = requestNewEntity();
  
  entity->meshes = (Mesh**)malloc(sizeof(Mesh*));
  entity->meshes[0] = (Mesh*)calloc(sizeof(Mesh),1);
  
  Mesh* mesh = entity->meshes[0];
  entity->meshes[0]->vertices = (Vertex*)malloc(sizeof(Vertex) * vertexCount);
  memcpy(entity->meshes[0]->vertices, vertices, sizeof(Vertex) * vertexCount);
  entity->meshes[0]->vertexCount = vertexCount;
  
  entity->meshes[0]->indices = (u32*)malloc(sizeof(u32) * indexCount);
  memcpy(entity->meshes[0]->indices, indices, sizeof(u32) * indexCount);
  entity->meshes[0]->rendererData.indexCount = indexCount;
  
  entity->meshCount = 1;
  entity->meshes[0]->visible = true;
  addMesh(mesh, "res/shaders/basicLightVertex.glsl","res/shaders/basicLightFrag.glsl");
  
  entity->scale.x = 1.0;
  entity->scale.y = 1.0;
  entity->scale.z = 1.0;
  
  errCheck();
  return entity;
}

void initEntityRegistry()
{
  globalEntityRegistry = (EntityRegistry*)calloc(1,sizeof(EntityRegistry));
}
void deleteEntityRegistry()
{
  for (int i = 0; i < MAX_REGISTRY_SIZE; i++)
    {
      if (globalEntityRegistry->occupiedIndices[i])
	{
	  deleteEntity(globalEntityRegistry->entities[i].id);
	}
    }
  free(globalEntityRegistry);
}
