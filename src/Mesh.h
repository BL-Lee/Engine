#ifndef __MESH_HEADER
#define __MESH_HEADER


#include "Collision.h"
struct Vertex
{
  vec3 pos;
  vec3 normal;
  vec2 texCoord;
  float texUnit;
};
struct RendererMeshData
{
  u32 vertexBufferKey;
  u32 indexBufferKey;
  u32 indexCount;
  u32 vertexArrayKey;
  u32 shaderProgramKey;
  u32 ambientTextureKey;
  u32 diffuseTextureKey;
};

struct Material
{
  char* name;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  f32 specularExponent;
  f32 opticalDensity;
  f32 dissolve;
  s32 illumination;
  char* ambientMap;
  char* diffuseMap;
  char* specularTextureMap;
  char* specularHighlightMap;
  char* alphaMap;
  char* bumpMap;
};




struct Mesh
{
  char* shaderName;
  
  bool visible;
  u32 vertexCount;
  Vertex* vertices;
  u32* indices;
  
  RendererMeshData rendererData;
  Material material;
  Collider collider;
};


/*

  root
  /      \
  child    sibling
 /\
child sibling(also child of root)
 */
struct SkinnedJointHierarchy
{
};

struct SkinnedJoint
{
  char* name;
  int parentIndex;
  mat4 restInverse;
  mat4 transform;
  vec3 position;
  hmm_quaternion orientation;
};


struct SkinnedAnimationFrame
{
  vec3 mins; //For bounding boxes of this frame
  vec3 maxs;
  SkinnedJoint* joints;
};

//MD5 version at the moment
struct SkinnedAnimation
{
  u32 frameCount;
  SkinnedAnimationFrame baseFrame;
  SkinnedAnimationFrame* frames;
  u32 jointCount;
  u32 frameRate;
};


struct SkinnedWeight
{
  int jointIndex;
  f32 bias;
};

struct SkinnedVertexWeight
{
  int startIndex;
  int count;
};

struct SkinnedMesh
{
  mat4* restInverses;
  SkinnedJoint* joints; //base frame
  u32 jointCount;

  SkinnedVertexWeight** vertexWeights;
  u32* vertexWeightCounts;
  
  SkinnedWeight** weights;
  u32* weightCounts;
  
  Mesh** meshes;
  u32 meshCount;
  //TEMP
  vec3** tempPositions;


  SkinnedAnimation* animations;//TODO: multiple animations
  //u32 animationCount;
  
};

//CURRENTLY UNUSED:
//BIG TODO:
/*

  Base frame rest inverse
  each frames rest inverse
  Position & orientation of each joint
*/


/*

  Skinned mesh should contain:
  Original vertex positions, from rest pose to model's origin
  Some number of weights, and indices for those weights
  
  Skinned animation should contain
  hierarchy of joints
  A bunch of frames
  
  A frame should contain  
  inverse transforms -> puts vertices from rest position to model origin
  transforms into world -> puts the vertex from model origin to world
  composite matrix -> world mat * inverse mat
 */

#endif
