#ifndef __RENDERER_HEADER
#define __RENDERER_HEADER
#include "Camera.h"

static Camera mainCamera;

struct PointLight
{
  vec3 position;
  vec3 ambientColour;
  vec3 diffuseColour;
  vec3 specularColour;

  float intensity;
  u32 entityGizmoID;
};

struct DirectionalLight
{
  vec3 direction;
  vec3 ambientColour;
  vec3 diffuseColour;
  vec3 specularColour;
  mat4 shadowMatrix;
};

#define RENDERER_MESH_DRAW_COUNT 512

#define RENDERER_UI_MODE 0
#define RENDERER_TEXTURE_MODE 1

#define RENDERER_POINT_LIGHT_COUNT 2
#define RENDERER_DIRECTIONAL_LIGHT_COUNT 1

#define RENDERER_MAX_DEBUG_LINE_COUNT 512

#define GAMMA_MASK 0x1
#define TONE_MAP_MASK 0x2
#define AUTO_EXPOSURE_MASK 0x4

struct DebugLine {
  bool active;
  vec3 min, max, colour;
  f32 lifeTime;
  u32 shapeID;
};

#define BLOOM_SAMPLE_COUNT 3
struct BloomInfo {
  u32 frameBuffer;
  vec2 sizes[BLOOM_SAMPLE_COUNT];
  u32 downSampleProgram;
  u32 firstDownSampleProgram;
  u32 upSampleProgram;
  u32 mipTextures[BLOOM_SAMPLE_COUNT];
  
  u32 blendingProgram;
  f32 strength;

  f32 cutoff;
};

struct FrameBufferInfo
{
  u32 width, height;
  u32 key;
  u32 textureKey;
  u32 renderBufferKey;
};

#define RENDERER_STANDARD_VAO_VBO_MAX_COUNT 2048
#define RENDERER_SKINNED_VAO_VBO_MAX_COUNT 2048
#define RENDERER_STANDARD_VAO_MESH_MAX_COUNT 256 * 8
#define RENDERER_SKINNED_VAO_MESH_MAX_COUNT 2

struct VertexLayoutComponent
{
  char name[64];
  u32 size;
  s32 location;
  GLenum type;
  u32 count;
  u64 offset;
};


struct RendererMeshVAOInfo
{
  u32 valid;
  u32 id;
  u64 startVBOIndex, endVBOIndex;
  u64 startIBIndex, endIBIndex;
  u32 programKey; //TODO: have meshes share programs if using the same shaders
  void* mesh;
  struct RendererMeshVAOInfo* next;
  struct RendererMeshVAOInfo* prev;
};


#define RENDERER_VAO_MESH_DRAW_COUNT 512
struct RendererVAOInfo
{
  u32 id;
  u32 key;
  u32 VBOKey;
  u32 IBKey;
  u32 shadowMapProgramKey;
  
  u64 maxVertexCount;
  u64 maxIndexCount;  

  u32 meshInfoCount;
  RendererMeshVAOInfo* meshInfoHead;
  RendererMeshVAOInfo* meshInfoTail;

  u32 meshesToDrawCount;
  RendererMeshVAOInfo* meshesToDraw[RENDERER_VAO_MESH_DRAW_COUNT];
  mat4 meshModelMatrices[RENDERER_VAO_MESH_DRAW_COUNT];

  VertexLayoutComponent* layout;
  u32 layoutCount;
  
  u32 vertexSize;
  //format
};
#include "Mesh.h"


struct RendererData
{
  u8 initialized;  
  bool wireFrameMode;

  RendererVAOInfo standardVAO;
  RendererVAOInfo skinnedVAO;
  RendererVAOInfo screenQuadVAO;
  RendererVAOInfo debugGeometryVAO;

  FrameBufferInfo outputFBO;
  FrameBufferInfo shadowMapFBO;

  //Lights
  PointLight pointLights[RENDERER_POINT_LIGHT_COUNT];
  s32 pointLightCount;
  DirectionalLight dirLights[RENDERER_DIRECTIONAL_LIGHT_COUNT];
  s32 dirLightCount;

  s32 viewportWidth, viewportHeight;

  u32 depthShader;
  u32 blueNoiseTex;

  u32 screenShaderProgramKey;

  u64 totalVertices;
  u64 totalIndices;
  u64 totalTriangles;
  DebugLine debugLines[RENDERER_MAX_DEBUG_LINE_COUNT];
  u32 debugLineIndex;
  Mesh* debugGeometryMesh;

};


static VertexLayoutComponent defaultLayout[4] =
  {
    "position", sizeof(vec3), 0, GL_FLOAT, 3, offsetof(Vertex, pos), 
    "normal", sizeof(vec3), 1, GL_FLOAT, 3, offsetof(Vertex, normal),
    "texCoord", sizeof(vec2), 2, GL_FLOAT, 2, offsetof(Vertex, texCoord),
    "texUnit", sizeof(float), 3, GL_FLOAT, 1, offsetof(Vertex, texUnit)
  };

#define SKINNED_MAX_JOINT_COUNT 4
static  VertexLayoutComponent skinnedDefaultLayout[6] =
  {
    "position", sizeof(vec3), 0, GL_FLOAT, 3, offsetof(SkinnedVertex, pos), 
    "normal", sizeof(vec3), 1, GL_FLOAT, 3, offsetof(SkinnedVertex, normal), 
    "texCoord", sizeof(vec2), 2, GL_FLOAT, 2, offsetof(SkinnedVertex, texCoord), 
    "texUnit", sizeof(float), 3, GL_FLOAT, 1,offsetof(SkinnedVertex, texUnit), 
    "jointIndices", sizeof(vec4), 4, GL_FLOAT, SKINNED_MAX_JOINT_COUNT,offsetof(SkinnedVertex, jointIndices), 
    "jointWeights", sizeof(vec4), 5, GL_FLOAT, SKINNED_MAX_JOINT_COUNT,offsetof(SkinnedVertex, jointWeights)
  };

static VertexLayoutComponent screenDefaultLayout[2] =
  {
    "position", sizeof(vec2), 0, GL_FLOAT, 2, 0,
    "texCoord", sizeof(vec2), 1, GL_FLOAT, 2, sizeof(vec2)
  };

/*
  glBindAttribLocation(globalRenderData.frameBufferShader, 0, "position");
  glBindAttribLocation(globalRenderData.frameBufferShader, 1, "texCoord");

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float)*4 , 0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT,   GL_FALSE, sizeof(float)*4 , (const void*)(sizeof(float)*2));
*/

static RendererData globalRenderData;


void initRenderer();
void initVAO(RendererVAOInfo* info, u32 vertexSize, VertexLayoutComponent* layout, u32 layoutCount, u32 shadowMapKey);
u8* loadTexture(u32* textureKey, const char* textureFile);
u32 requestTextureKey(const char* textureName);
u32 getTextureUnitByKey(u32 texKey);
void deleteTexture(const char* textureName);
void deleteRenderer();
void addMesh(Mesh* mesh, const char* vertexShader, const char* fragmentShader);
void addMesh(Mesh* mesh, const char* vertexShader, const char* fragmentShader, VertexLayoutComponent* layout, RendererVAOInfo* vao);
void drawMesh(Mesh* mesh, vec3 position, vec3 rotation, vec3 scale);
void rendererBeginScene();
void rendererEndScene();

#endif
