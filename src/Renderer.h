#ifndef __RENDERER_HEADER
#define __RENDERER_HEADER
#include "Camera.h"
#include "Mesh.h"
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

#define RENDERER_MAX_QUADS 10000
#define RENDERER_MAX_VERTICES (RENDERER_MAX_QUADS * 4)
#define RENDERER_MAX_INDICES (RENDERER_MAX_QUADS * 6)
#define RENDERER_BUFFER_COUNT 2

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

struct RendererData
{
  u8 initialized;  
  s32 maxTextureUnits;
  char** textureNames;
  u32* textureKeys;
  u32* totalUsingTexture;
  u32 textureCount;
  u32 textureBufferSize;
  bool wireFrameMode;

  //Following is for manual triangle/quad addition.
  //Mainly for UI

  //Array of stuff because this way we can buffer the
  //Triangles by shader type without using extra draw calls
  //ie 0 being UI
  //   1 being Diffuse maybe?
  u32 _texturesToBind[RENDERER_BUFFER_COUNT][16];
  u32 _texturesToBindCount[RENDERER_BUFFER_COUNT];

  Vertex* _vertexBufferBase[RENDERER_BUFFER_COUNT];
  Vertex* _vertexBufferPtr[RENDERER_BUFFER_COUNT];

  u32* _indexBufferBase[RENDERER_BUFFER_COUNT];
  u32 _indexOffset[RENDERER_BUFFER_COUNT];
  u32 _indexCount[RENDERER_BUFFER_COUNT];
  
  u32 _vertexBufferKey[RENDERER_BUFFER_COUNT];
  u32 _indexBufferKey[RENDERER_BUFFER_COUNT];
  u32 _vertexArrayKey[RENDERER_BUFFER_COUNT];
  u32 _shaderProgramKey[RENDERER_BUFFER_COUNT];

  //These are here to provide syntactic sugar. so array indexing doesnt need to happen everywhere
  //ie: these point to a certain index of underscore'd versions.
  //Consider just using the arrays. This could produce overhead from switching shaders
  //Also this is very overcomplicated. REWORK

  //Sept 24 2022 - might not even need this anymore unless you wanted to do something fancy
  //Any geometry in the world is done through meshes, then UI through this. So maybe only need the 1?
  u32* texturesToBind;
  u32* texturesToBindCount;

  Vertex* vertexBufferBase;
  Vertex* vertexBufferPtr;

  u32* indexBufferBase;
  u32* indexOffset;
  u32* indexCount;
  
  u32 vertexBufferKey;
  u32 indexBufferKey;
  u32 vertexArrayKey;
  u32 shaderProgramKey;

  u32 currentShaderIndex;

  //Lights
  PointLight pointLights[RENDERER_POINT_LIGHT_COUNT];
  s32 pointLightCount;
  DirectionalLight dirLights[RENDERER_DIRECTIONAL_LIGHT_COUNT];
  s32 dirLightCount;

  //Frame Buffer & post processing info
  u32 frontBuffer;
  u32 frontBufferTexture;

  u32 colourPaletteLUT;
  bool palettize;

  u32 frameBufferShader;
  u32 frameBufferQuadVAO;
  u32 frameBufferVB;

  s32 frameBufferWidth;
  s32 frameBufferHeight;

  u32 postProcessingShaders[5];
  u32 enabledScreenShader;
  f32 exposure;
  vec4 averageColour;
  f32 luminanceTemporal;
  f32 exposureChangeRate;

  u32 shadowMap;
  u32 shadowMapTexture;
  u32 shadowMapShader;
  u32 skinnedShadowMapShader;
  
  s32 shadowMapWidth, shadowMapHeight; //currently uses frame buffer VAO and VB

  s32 viewportWidth, viewportHeight;

  u32 depthShader;

  u32 blueNoiseTex;

  Mesh* meshesToDraw[RENDERER_MESH_DRAW_COUNT];
  vec3 meshTransforms[RENDERER_MESH_DRAW_COUNT * 3];
  mat4 meshModelMatrices[RENDERER_MESH_DRAW_COUNT];
  u32 meshesToDrawCount;

  
  DebugLine debugLines[RENDERER_MAX_DEBUG_LINE_COUNT];
  u32 debugLineIndex;
  Mesh* debugGeometryMesh;


};

struct VertexLayoutComponent
{
  char name[64];
  u32 size;
  s32 location;
  GLenum type;
  u32 count;
};

static VertexLayoutComponent defaultLayout[4] =
  {
    "position", sizeof(vec3), 0, GL_FLOAT, 3, 
    "normal", sizeof(vec3), 1, GL_FLOAT, 3, 
    "texCoord", sizeof(vec2), 2, GL_FLOAT, 2,
    "texUnit", sizeof(float), 3, GL_FLOAT, 1
  };

#define SKINNED_MAX_JOINT_COUNT 4
static  VertexLayoutComponent skinnedDefaultlayout[6] =
  {
    "position", sizeof(vec3), 0, GL_FLOAT, 3, 
    "normal", sizeof(vec3), 1, GL_FLOAT, 3, 
    "texCoord", sizeof(vec2), 2, GL_FLOAT, 2,
    "texUnit", sizeof(float), 3, GL_FLOAT, 1,
    "jointIndices", sizeof(vec4), 4, GL_FLOAT, SKINNED_MAX_JOINT_COUNT,
    "jointWeights", sizeof(vec4), 5, GL_FLOAT, SKINNED_MAX_JOINT_COUNT
  };

  

static RendererData globalRenderData;


void initRenderer();
u8* loadTexture(u32* textureKey, const char* textureFile);
u32 requestTextureKey(const char* textureName);
u32 getTextureUnitByKey(u32 texKey);
void deleteTexture(const char* textureName);
void deleteRenderer();
void addMesh(Mesh* mesh, const char* vertexShader, const char* fragmentShader);
void drawMesh(Mesh* mesh, vec3 position, vec3 rotation, vec3 scale);
void rendererBeginScene();
void rendererEndScene();

#endif
