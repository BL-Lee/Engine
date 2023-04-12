#ifndef ENTITY_REGISTRY_HEADER
#define ENTITY_REGISTRY_HEADER

#include "Collision.h"
#include "Physics.h"
struct Entity
{
  u32 id;
  char name[256];
  struct Entity* parent;
  struct Entity** children;
  u32 childCount;
  u32 childArraySize;
  vec3 position;
  vec3 scale;
  vec3 rotation;
  mat4 globalTransform;
  bool globalTransformDirty;
  vec3 velocity;
  vec3 angularVelocity;
  Mesh** meshes;
  char* vertShader;
  char* fragShader;
  u32 meshCount;
  bool visible;


  float mass;
  bool physicsEnabled;
  bool gravityEnabled;
  Collider collider;
};

#define MAX_REGISTRY_SIZE 512

struct EntityRegistry
{
  Entity entities[MAX_REGISTRY_SIZE];
  u8 occupiedIndices[MAX_REGISTRY_SIZE];
  u32 entityCount;
  u32 entityCounter;
};

static EntityRegistry* globalEntityRegistry;
mat4 getEntityGlobalTransform(Entity* e);


#endif
