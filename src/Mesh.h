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
struct SkinnedVertex
{
  vec3 pos;
  vec3 normal;
  vec2 texCoord;
  float texUnit;
  float jointIndices[4];
  float jointWeights[4];
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
  void* vertices;
  
  u32* indices;
  
  RendererMeshData rendererData;
  Material material;

  void* skinnedMesh;
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


/*
  
  TODO: consider storing like this instead:
  SkinnedJoint {
    mat4* transforms; (1 for each frame)
  }
  We always are interpolating between frames, so why store like this:
  frame 1:
  joint 1 info
  joint 2 info
  joint 3 info...

  instead of this?
  joint 1 info:
   frame 1 transform
   frame 2 transform ...
  joint 2 info:
   frame 1 transform
   frame 2 transform ...

   Would probably get better cache performance this way

 */


struct SkinnedJoint
{
  char* name;
  int parentIndex;
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
/*
TODO: Curves between frames,
If we can fit a bunch of frames to a single curve then we cut down on storing that many frames
Ie interpolating between N many frames using a bezier curve instead of storing all the N frames and interpolating between 2 of them at a time.
 */
struct SkinnedAnimation
{
  u32 frameCount;
  SkinnedAnimationFrame baseFrame;
  SkinnedAnimationFrame* frames;
  u32 jointCount;
  u32 frameRate;

  u32 currentFrames[4];
  f32 currentInterps[4];

  mat4* compositeMatrices;
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

  Mesh** meshes;
  u32 meshCount;
  //TEMP
  vec3** tempPositions;


  SkinnedAnimation* animations;//TODO: multiple animations
  s16 currentAnimation;
  //u32 animationCount;
  
};

//BIG TODO:
/*
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
