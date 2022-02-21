#include "Shader.h"
#include "Camera.h"
#include "Skinned.cpp"

/* TODO

   Consider having one big buffer that it copies data into, 
that way the binding wouldnt have to switch as much? idk,
Also that way it would just be 1 draw call for shadow maps.
Look into this

 --Update: probably one bigger one for static meshes, and keep track
 of where they are in the buffer, and small ones for dhnamic meshes
 */
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

#define GAMMA_MASK 0x1
#define TONE_MAP_MASK 0x2
#define AUTO_EXPOSURE_MASK 0x4

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
  
  s32 shadowMapWidth; //currently uses frame buffer VAO and VB
  s32 shadowMapHeight;

  u32 depthShader;

  u32 blueNoiseTex;

  Mesh* meshesToDraw[RENDERER_MESH_DRAW_COUNT];
  vec3 meshTransforms[RENDERER_MESH_DRAW_COUNT * 3];
  mat4 meshModelMatrices[RENDERER_MESH_DRAW_COUNT];
  u32 meshesToDrawCount;
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

//Switch the shader type for future renderer calls
void setRendererShaderMode(u32 i)
{
  globalRenderData._vertexBufferPtr[globalRenderData.currentShaderIndex] = globalRenderData.vertexBufferPtr;
  Assert(i < RENDERER_BUFFER_COUNT);
  globalRenderData.texturesToBind = globalRenderData._texturesToBind[i];
  globalRenderData.texturesToBindCount = &globalRenderData._texturesToBindCount[i];
  globalRenderData.vertexBufferBase = globalRenderData._vertexBufferBase[i];
  globalRenderData.vertexBufferPtr = globalRenderData._vertexBufferPtr[i];
  globalRenderData.indexBufferBase = globalRenderData._indexBufferBase[i];
  globalRenderData.indexOffset = globalRenderData._indexOffset + i;
  globalRenderData.indexCount = globalRenderData._indexCount + i;
  globalRenderData.vertexBufferKey = globalRenderData._vertexBufferKey[i];
  globalRenderData.indexBufferKey = globalRenderData._indexBufferKey[i];
  globalRenderData.vertexArrayKey = globalRenderData._vertexArrayKey[i];
  globalRenderData.shaderProgramKey = globalRenderData._shaderProgramKey[i];
  globalRenderData.currentShaderIndex = i;
}

/*

  Shader functions

 */
void validateShaderCompilation(u32 shaderID)
{
#if DEBUG_GL
  char log[8000];
  int logLength;
  int status;
  glGetShaderiv(shaderID, GL_COMPILE_STATUS, &status);
  if (status != GL_TRUE)
    {
      glGetShaderInfoLog( shaderID,
			  8000,
			  &logLength,
			  log);
      fprintf(stderr, "ERROR: SHADER COMPILATION FAILED\n");
      printf("Shader log length: %d\n", logLength);
      printf("%s\n", log);
      Assert(0);
    }
#endif
}
void validateShaderLink(u32 programID)
{
#if DEBUG_GL
  char log[8000];
  int logLength;
  int status;
  glGetProgramiv(programID, GL_LINK_STATUS, &status);
  if (status != GL_TRUE)
    {
      glGetProgramInfoLog( programID,
			  8000,
			  &logLength,
			  log);
      fprintf(stderr, "ERROR: SHADER LINKING FAILED");
      printf("Shader log length: %d\n", logLength);
      printf("%s\n", log);
      Assert(0);

    }
#endif
}

u32 loadAndValidateShaderPair(const char* vert, const char* frag)
{
  u32 program = glCreateProgram();
  u32 vs = loadShader(vert, GL_VERTEX_SHADER);
  validateShaderCompilation(vs);
  u32 fs = loadShader(frag, GL_FRAGMENT_SHADER);
  validateShaderCompilation(fs);
      

  //link program
  glAttachShader(program, vs);
  glAttachShader(program, fs);
  glLinkProgram(program);           
  glValidateProgram(program);
      
  validateShaderLink(program);

  return program;
}
//Generates a texture width x and height y and returns the openGL key
u32 genTexture(void* data, u32 x, u32 y, u32 channelType, GLenum wrapType, GLenum filterType)
{
  u32 textureKey;
  //Gen texture  
  glGenTextures(1, &textureKey);
  glBindTexture(GL_TEXTURE_2D, textureKey);
  errCheck()  
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterType);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterType);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapType);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapType);
  errCheck();
  
  //set texture data
  // texture_data is the source data of your texture, in this case
  // its size is sizeof(unsigned char) * texture_width * texture_height * 4
  glTexImage2D(GL_TEXTURE_2D, 0, channelType, x, y, 0, channelType, GL_UNSIGNED_BYTE, data);
  errCheck();
  return textureKey;
}
//Generates a texture width x and height y and returns the openGL key
u32 genTexture(void* data, u32 x, u32 y, u32 channelType)
{
  return genTexture(data, x, y, channelType, GL_CLAMP_TO_BORDER, GL_LINEAR);
}

//Loads a texture from a file. Places the opengl Key in textureKey and returns the texture data
u8* loadTexture(u32* textureKey, const char* textureFile, GLenum wrapType, GLenum filterType)
{
  //load image
  int x, y, n;
  u8* data = stbi_load(textureFile, &x, &y, &n, 0);

  if (!data)
    {
      char texPath[512] = "textures/";
      strcpy(texPath + 9, textureFile);
      u8* data = stbi_load(texPath, &x, &y, &n, 0);
      if (!data)
	{
	  printf("WARNING: FAILED TO LOAD TEXTURE:%s\n", textureFile);
	  printf("STBI ERROR: %s\n",stbi_failure_reason());
	  //Assert(0);
	  return NULL;
	}
    }
  GLenum channelType;
  switch (n)
    {
    case 1:
      channelType = GL_RED;
      break;
    case 2:
      channelType = GL_RG;
      break;
    case 3:
      channelType = GL_RGB;
      break;
    case 4:
      channelType = GL_RGBA;
      break;
    default:
      assert(0);
      break;
    }
  *textureKey = genTexture(data, x, y, channelType, wrapType, filterType);
  return data;  
}
u8* loadTexture(u32* textureKey, const char* textureFile)
{
  return loadTexture(textureKey, textureFile, GL_CLAMP_TO_BORDER, GL_LINEAR);
}

//requesst a key to be put in the renderer's texture buffer pool. returns the opengl Key
u32 requestTextureKey(const char* textureName)
{  
  Assert(globalRenderData.textureCount < 64);
  Assert(globalRenderData.initialized);
  //printf("Requesting texture: %s... ", textureName);
  for (int i = 0; i < globalRenderData.textureBufferSize; i++)
    {
      if (globalRenderData.totalUsingTexture[i] &&
	  strcmp(textureName, globalRenderData.textureNames[i]) == 0)
	{
	  //printf("Exists! Total using this texture = %d\n", globalRenderData.totalUsingTexture[i] + 1);
	  globalRenderData.totalUsingTexture[i]++;
	  return globalRenderData.textureKeys[i];
	}
    }
  //printf("Placing in map.\n");
  u32 key;
  u8* data = loadTexture(&key, textureName);
  if (data)
    {
      globalRenderData.textureNames[globalRenderData.textureCount] = (char*)malloc(strlen(textureName) + 1);
      strcpy(globalRenderData.textureNames[globalRenderData.textureCount], textureName);
      
      globalRenderData.textureKeys[globalRenderData.textureCount] = key;
      globalRenderData.totalUsingTexture[globalRenderData.textureCount]++;
      
      globalRenderData.textureCount++;
      stbi_image_free(data);
      return key;
    }
  else
    {
      return (u32)-1;
    }
}

//Requests a texture key from a raw image width x and height y.
u32 requestTextureKey(const char* textureName, void* data, u32 x, u32 y, u32 channelType)
{  
  Assert(globalRenderData.textureCount < 64);
  Assert(globalRenderData.initialized);
  for (int i = 0; i < globalRenderData.textureBufferSize; i++)
    {
      if (globalRenderData.totalUsingTexture[i] &&
	  strcmp(textureName, globalRenderData.textureNames[i]) == 0)
	{
	  globalRenderData.totalUsingTexture[i]++;
	  return globalRenderData.textureKeys[i];
	}
    }

  if (data)
    {
      u32 key = genTexture(data, x, y, channelType);
      globalRenderData.textureNames[globalRenderData.textureCount] = (char*)malloc(strlen(textureName) + 1);
      strcpy(globalRenderData.textureNames[globalRenderData.textureCount], textureName);
      
      globalRenderData.textureKeys[globalRenderData.textureCount] = key;
      globalRenderData.totalUsingTexture[globalRenderData.textureCount]++;
      
      globalRenderData.textureCount++;

      return key;

    }
  else
    {
      return (u32)-1;
    }
}

//Removes a texture from the pool. If no other entities are using this texture then the texture's memory is freed
void deleteTexture(const char* textureName)
{
  for (int i = 0; i < globalRenderData.textureBufferSize; i++)
    {
      if (globalRenderData.totalUsingTexture[i] && strcmp(textureName, globalRenderData.textureNames[i]) == 0)
	{
	  //Decrement the # things using this texture
	  globalRenderData.totalUsingTexture[i]--;
	  if (globalRenderData.totalUsingTexture[i] == 0)
	    {
	      //If nothing else using it then delete it
	      glDeleteTextures(1, &globalRenderData.textureKeys[i]);
	      free(globalRenderData.textureNames[i]);
	      return;
	    }
	  return;
	}
    }
}

/* 

   Frame buffer functions

 */
void initializeFrameBufferShaders()
{

  globalRenderData.postProcessingShaders[0] =
    loadAndValidateShaderPair( "res/shaders/screenShadervert.glsl",
			       "res/shaders/screen_basic.glsl");
  globalRenderData.postProcessingShaders[1] =
    loadAndValidateShaderPair( "res/shaders/screenShadervert.glsl",
			       "res/shaders/screen_gamma.glsl");
  globalRenderData.postProcessingShaders[2] =
    loadAndValidateShaderPair( "res/shaders/screenShadervert.glsl",
			       "res/shaders/screen_tone_map.glsl");
  globalRenderData.postProcessingShaders[3] =
    loadAndValidateShaderPair( "res/shaders/screenShadervert.glsl",
			       "res/shaders/screen_tone_map_gamma.glsl");
  globalRenderData.postProcessingShaders[4] =
    loadAndValidateShaderPair( "res/shaders/screenShadervert.glsl",
			       "res/shaders/screen_auto_exposure.glsl");
}

void setFrameBufferShader(int shader)
{
  globalRenderData.frameBufferShader = globalRenderData.postProcessingShaders[shader];
}
			
void initializeFrameBuffer()
{
  //Get size, since mac retina displays need different resolutions than window size
  s32 framebufferWidth;
  s32 framebufferHeight;;
  glfwGetFramebufferSize(mainWindow.glWindow, &framebufferWidth, &framebufferHeight);

  globalRenderData.frameBufferHeight = framebufferHeight;
  globalRenderData.frameBufferWidth = framebufferWidth;

  //Gen frame buffer
  glGenFramebuffers(1, &globalRenderData.frontBuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, globalRenderData.frontBuffer);
  
  // generate texture
  glGenTextures(1, &globalRenderData.frontBufferTexture);
  glBindTexture(GL_TEXTURE_2D, globalRenderData.frontBufferTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, framebufferWidth, framebufferHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

  //Bind texture
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, globalRenderData.frontBufferTexture, 0);

  //Generate renderbuffer
  unsigned int rbo;
  glGenRenderbuffers(1, &rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo); 
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, framebufferWidth, framebufferHeight);  

  //Bind render buffer
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

  //Validate
  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
      fprintf(stderr,"WARNING: FRONT BUFFER FAILED INIT\n");
      fprintf(stderr, "ERRCODE: %d\n", glCheckFramebufferStatus(GL_FRAMEBUFFER));
    }

  //Set shaders for post processing
  initializeFrameBufferShaders();
  globalRenderData.frameBufferShader = globalRenderData.postProcessingShaders[4];


  //Generate gpu buffers for full screen quad
  //Array
  glGenVertexArrays(1, &globalRenderData.frameBufferQuadVAO);
  glBindVertexArray(globalRenderData.frameBufferQuadVAO);
  //VertexBuffer
  glGenBuffers(1, &globalRenderData.frameBufferVB);
  glBindBuffer(GL_ARRAY_BUFFER, globalRenderData.frameBufferVB);
  float frameBufferVertices[4 * 6] =
    {
      -1.0, -1.0, 0.0, 0.0,
       1.0, -1.0, 1.0, 0.0,
       1.0,  1.0, 1.0, 1.0,
       
      -1.0, -1.0, 0.0, 0.0,
      -1.0,  1.0, 0.0, 1.0,
       1.0,  1.0, 1.0, 1.0
    };
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * 6, frameBufferVertices, GL_STATIC_DRAW);

  glBindAttribLocation(globalRenderData.frameBufferShader, 0, "position");
  glBindAttribLocation(globalRenderData.frameBufferShader, 1, "texCoord");

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float)*4 , 0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT,   GL_FALSE, sizeof(float)*4 , (const void*)(sizeof(float)*2));
 
  errCheck();
}

void initRenderer()
{  
  stbi_set_flip_vertically_on_load(1);
  glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &globalRenderData.maxTextureUnits);
  glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &globalRenderData.maxTextureUnits);
  #if DEBUG_INIT_STATS
  printf("INIT STAT: Maximum Texture Units: %d\n", globalRenderData.maxTextureUnits);
  printf("INIT STAT: Maximum Combined Texture Units: %d\n", globalRenderData.maxTextureUnits);
  #endif

  u32 maxTextures = 64;
  
  globalRenderData.textureNames = (char**)malloc(sizeof(char*) * maxTextures);
  globalRenderData.textureKeys = (u32*)malloc(sizeof(u32) * maxTextures);
  globalRenderData.totalUsingTexture = (u32*)calloc(sizeof(u32), maxTextures);
  globalRenderData.textureBufferSize = maxTextures;
  globalRenderData.wireFrameMode = 0;
  globalRenderData.initialized = 1;

  globalRenderData.meshesToDrawCount = 0;

  globalRenderData.exposure = 0.2f;
  globalRenderData.enabledScreenShader = 0;
  setFrameBufferShader(0);
  globalRenderData.luminanceTemporal = 1.0f;
  globalRenderData.exposureChangeRate = 1.0f;

  globalRenderData.dirLightCount = -1;
  globalRenderData.pointLightCount = -1;

  //Dynamic geometry buffers for ui and stuff
  for (int i = 0; i < RENDERER_BUFFER_COUNT; i++)
    {
      globalRenderData._vertexBufferBase[i] = (Vertex*)malloc(sizeof(Vertex) * RENDERER_MAX_VERTICES);
      globalRenderData._vertexBufferPtr[i] = globalRenderData._vertexBufferBase[i];

      globalRenderData._indexBufferBase[i] = (u32*)malloc(sizeof(u32) * RENDERER_MAX_INDICES);
      globalRenderData._indexOffset[i] = 0;
      globalRenderData._indexCount[i] = 0;

      switch (i)
	{
	case 0:
	  {
	    globalRenderData._shaderProgramKey[i] =
	      loadAndValidateShaderPair("res/shaders/msdfVert.glsl",
					"res/shaders/msdfFrag.glsl");
	  }break;
	case 1:
	  {
	    globalRenderData._shaderProgramKey[i] =
	      loadAndValidateShaderPair("res/shaders/dynamicVert.glsl",
					"res/shaders/dynamicFrag.glsl");
	  }break;
	default:
	  {
	    Assert(0);
	  }break;
	}
      
      u32 program = globalRenderData._shaderProgramKey[i];
      
      //Generate gpu buffers
      //Array
      glGenVertexArrays(1, &globalRenderData._vertexArrayKey[i]);
      glBindVertexArray(globalRenderData._vertexArrayKey[i]);
      //VertexBuffer
      glGenBuffers(1, &globalRenderData._vertexBufferKey[i]);
      glBindBuffer(GL_ARRAY_BUFFER, globalRenderData._vertexBufferKey[i]);
      glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * RENDERER_MAX_VERTICES, NULL, GL_DYNAMIC_DRAW);

      //IndexBuffer
      glGenBuffers(1, &globalRenderData._indexBufferKey[i]);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, globalRenderData._indexBufferKey[i]);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * RENDERER_MAX_INDICES, NULL, GL_DYNAMIC_DRAW);

      glBindAttribLocation(program, 0, "position");
      glBindAttribLocation(program, 1, "normal");
      glBindAttribLocation(program, 2, "texCoord");
      glBindAttribLocation(program, 3, "texUnit");
      //generate gpu vertex layout
      glEnableVertexAttribArray(0);

      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float)*9 , 0);
      glEnableVertexAttribArray(1);
      glVertexAttribPointer(1, 3, GL_FLOAT,   GL_FALSE, sizeof(float)*9 , (const void*)(sizeof(float)*3));
      glEnableVertexAttribArray(2);
      glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float)*9 , (const void*)(sizeof(float)*6));
      glEnableVertexAttribArray(3);
      glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(float)*9 , (const void*)(sizeof(float)*8));

    }
  setRendererShaderMode(0);

  initializeFrameBuffer();
        glGenerateMipmap(GL_TEXTURE_2D);
  // //Initialize shadow map // //
  globalRenderData.shadowMapWidth = 1024;
  globalRenderData.shadowMapHeight = 1024;

  //gen frame buffer
  glGenFramebuffers(1, &globalRenderData.shadowMap);
  glBindFramebuffer(GL_FRAMEBUFFER, globalRenderData.shadowMap);
  
  //generate texture
  glGenTextures(1, &globalRenderData.shadowMapTexture);
  glBindTexture(GL_TEXTURE_2D, globalRenderData.shadowMapTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
	       globalRenderData.shadowMapWidth, globalRenderData.shadowMapHeight,
	       0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, globalRenderData.shadowMapTexture, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);


  globalRenderData.shadowMapShader = loadAndValidateShaderPair("res/shaders/shadowMapVert.glsl",
							       "res/shaders/shadowMapFrag.glsl");
   globalRenderData.skinnedShadowMapShader = loadAndValidateShaderPair("res/shaders/skinnedShadowMapVert.glsl",
  								      "res/shaders/shadowMapFrag.glsl");
  
  globalRenderData.depthShader = loadAndValidateShaderPair("res/shaders/screenShadervert.glsl",
							   "res/shaders/depthFrag.glsl");

  u8* data = loadTexture(&globalRenderData.blueNoiseTex, "res/textures/LDR_RGB1_0.png", GL_REPEAT, GL_NEAREST);
  errCheck();
  free(data);
  
  errCheck();
}

//TODO: Clean up Renderer resources
void deleteRenderer()
{
  Assert(globalRenderData.initialized);
  for (int i = 0; i < globalRenderData.textureCount; i++)
    {
      if (globalRenderData.totalUsingTexture[i])
	{
	  glDeleteTextures(1, &globalRenderData.textureKeys[i]);
	  free(globalRenderData.textureNames[i]);
	}
    }
  glDeleteProgram(globalRenderData.shaderProgramKey);
  glDeleteVertexArrays(1, &globalRenderData.vertexArrayKey);
  glDeleteBuffers(1, &globalRenderData.vertexBufferKey);
  glDeleteBuffers(1, &globalRenderData.indexBufferKey);

  glDeleteFramebuffers(1, &globalRenderData.frontBuffer);

  free(globalRenderData.textureNames);
  free(globalRenderData.textureKeys);
  free(globalRenderData.totalUsingTexture);
  free(globalRenderData.vertexBufferBase);
  free(globalRenderData.indexBufferBase);  
}

  
void addMesh(Mesh* mesh, const char* vertexShader, const char* fragmentShader, VertexLayoutComponent* layout, u32 layoutCount)
{
  RendererMeshData* meshData = &mesh->rendererData;
  //create shader program
  u32 program = glCreateProgram();
  u32 vs = loadShader(vertexShader, GL_VERTEX_SHADER);
  u32 fs = loadShader(fragmentShader, GL_FRAGMENT_SHADER);

  //link program
  u32 totalLayoutSize = 0;
  glAttachShader(program, vs);
  for (int i = 0; i < layoutCount; i++)
    {
      glBindAttribLocation(program, layout[i].location, layout[i].name);
      totalLayoutSize += layout[i].size;
    }
  glAttachShader(program, fs);

  validateShaderCompilation(vs);
  validateShaderCompilation(fs);

  glLinkProgram(program);
  glValidateProgram(program);
  validateShaderLink(program);
  mesh->rendererData.shaderProgramKey = program;

  errCheck();
  //Generate gpu buffers
  //Array
  glGenVertexArrays(1, &meshData->vertexArrayKey);
  glBindVertexArray(meshData->vertexArrayKey);
  //VertexBuffer
  glGenBuffers(1, &meshData->vertexBufferKey);
  glBindBuffer(GL_ARRAY_BUFFER, meshData->vertexBufferKey);
  if (mesh->skinnedMesh)
    glBufferData(GL_ARRAY_BUFFER, sizeof(SkinnedVertex) * mesh->vertexCount, mesh->vertices, GL_DYNAMIC_DRAW);
  else
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * mesh->vertexCount, mesh->vertices, GL_DYNAMIC_DRAW);

  //IndexBuffer
  glGenBuffers(1, &meshData->indexBufferKey);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshData->indexBufferKey);
  //Adding data to thebuffer
  //Add indices
    
  //TODO: triangles using indices
  if (meshData->indexCount == 0)
    {
      u32 indexData[mesh->vertexCount];
      for (int i = 0; i < mesh->vertexCount; i++)
	{
	  indexData[i] = i; 
	}
      meshData->indexCount = mesh->vertexCount;
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, meshData->indexCount * sizeof(u32), indexData, GL_DYNAMIC_DRAW);
    }
  else
    {
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, meshData->indexCount * sizeof(u32), mesh->indices, GL_DYNAMIC_DRAW);   
    }

  
  //generate gpu vertex layout
  u32 currentOffset = 0;
  for (int i = 0; i < layoutCount; i++)
    {
      glEnableVertexAttribArray(layout[i].location);
      glVertexAttribPointer(layout[i].location, layout[i].count, layout[i].type, GL_FALSE, totalLayoutSize, (const void*)currentOffset);
      currentOffset += layout[i].size;
    }

  
  glDetachShader(program, vs);
  glDetachShader(program, fs);
 }

void addMesh(Mesh* mesh, const char* vertexShader, const char* fragmentShader)
{
  addMesh(mesh, vertexShader, fragmentShader, defaultLayout, 4);
}


void setVec3Uniform(s32 program, const char* name, vec3 values)
{
  s32 location = glGetUniformLocation(program, name);
  glUniform3f(location, values.x, values.y, values.z );  
}

void setFloatUniform(s32 program, const char* name, float value)
{
  s32 location = glGetUniformLocation(program, name);
  glUniform1f(location, value );  
}

void setLightUniform(s32 program)
{
  //Set lights
  
  //Holy moly, jeepers creepers....
  //You cannot GetUniformLocation on a struct, only members of the struct
  //IE: getloc(program, "light") would NOT work because it is not a member of the struct
  s32 location = glGetUniformLocation(program, "pointLights[0].ambientColour");
  if (location != -1)
    {
      //printf("doing the thing\n");
      char uniformBuffer[64];
      for (int i = 0; i < RENDERER_POINT_LIGHT_COUNT; i++)
	{
	  sprintf(uniformBuffer, "pointLights[%d].ambientColour", i);
	  setVec3Uniform(program, uniformBuffer, globalRenderData.pointLights[i].ambientColour
			 * globalRenderData.pointLights[i].intensity);
	  sprintf(uniformBuffer, "pointLights[%d].diffuseColour", i);	  
	  setVec3Uniform(program, uniformBuffer, globalRenderData.pointLights[i].diffuseColour
			 * globalRenderData.pointLights[i].intensity);
	  sprintf(uniformBuffer, "pointLights[%d].specularColour", i);
	  setVec3Uniform(program, uniformBuffer, globalRenderData.pointLights[i].specularColour
			 * globalRenderData.pointLights[i].intensity);
	  sprintf(uniformBuffer, "pointLights[%d].position", i);
	  setVec3Uniform(program, uniformBuffer, globalRenderData.pointLights[i].position);
	  sprintf(uniformBuffer, "pointLights[%d].linearFalloff", i);	  
	  setFloatUniform(program, uniformBuffer, 1 / globalRenderData.pointLights[i].intensity);
	}
      errCheck();
    }
  location = glGetUniformLocation(program, "dirLights[0].ambientColour");
  if (location != -1)
    {
      //printf("doing the thing\n");
      char uniformBuffer[64];
      for (int i = 0; i < RENDERER_DIRECTIONAL_LIGHT_COUNT; i++)
	{
	  vec3 dir = NormalizeVec3(globalRenderData.dirLights[i].direction);
	  sprintf(uniformBuffer, "dirLights[%d].ambientColour", i);
	  setVec3Uniform(program, uniformBuffer, globalRenderData.dirLights[i].ambientColour);
	  sprintf(uniformBuffer, "dirLights[%d].diffuseColour", i);	  
	  setVec3Uniform(program, uniformBuffer, globalRenderData.dirLights[i].diffuseColour);
	  sprintf(uniformBuffer, "dirLights[%d].specularColour", i);
	  setVec3Uniform(program, uniformBuffer, globalRenderData.dirLights[i].specularColour);
	  sprintf(uniformBuffer, "dirLights[%d].direction", i);
	  setVec3Uniform(program, uniformBuffer, dir);
	}
      errCheck();
    }
  
}

void setMaterialUniform(s32 program, Material* mat)
{
  s32 location = glGetUniformLocation(program, "material.ambient");
  if (location != -1)
    {
      char uniformBuffer[64];
      setVec3Uniform(program, "material.ambient", mat->ambient);
      setVec3Uniform(program, "material.diffuse", mat->diffuse);
      setVec3Uniform(program, "material.specular", mat->specular);
      setFloatUniform(program, "material.shininess", mat->specularExponent);        
    }
}

void drawMesh(Mesh* mesh, vec3 position, vec3 rotation, vec3 scale)
{
  Assert(globalRenderData.meshesToDrawCount < RENDERER_MESH_DRAW_COUNT);
  globalRenderData.meshesToDraw[globalRenderData.meshesToDrawCount] = mesh;
  globalRenderData.meshTransforms[globalRenderData.meshesToDrawCount * 3 + 0] = position;
  globalRenderData.meshTransforms[globalRenderData.meshesToDrawCount * 3 + 1] = rotation;
  globalRenderData.meshTransforms[globalRenderData.meshesToDrawCount * 3 + 2] = scale;  
  globalRenderData.meshesToDrawCount++;
}

void rendererBeginScene()
{
  int prev = globalRenderData.currentShaderIndex;
  for (int i = 0; i < RENDERER_BUFFER_COUNT; i++)
    {
      setRendererShaderMode(i);
      globalRenderData.vertexBufferPtr = globalRenderData.vertexBufferBase;
      *globalRenderData.indexCount = 0;
      *globalRenderData.indexOffset = 0;      
    }
  setRendererShaderMode(prev);
}

//Flushes all buffers and draws scenes
void rendererEndScene()
{
  int prev = globalRenderData.currentShaderIndex;
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  
  //Iterate through all shaders to draw
  for (int i = RENDERER_BUFFER_COUNT - 1; i >= 0; i--)
    {
      setRendererShaderMode(i);
      glUseProgram(globalRenderData.shaderProgramKey);
      glBindVertexArray(globalRenderData.vertexArrayKey);
      
      //calculate how much vertex data to send
      uint32_t dataSize = (u8*)globalRenderData.vertexBufferPtr - (u8*)globalRenderData.vertexBufferBase; 
      glBindBuffer(GL_ARRAY_BUFFER, globalRenderData.vertexBufferKey);
      glBufferData(GL_ARRAY_BUFFER, dataSize, globalRenderData.vertexBufferBase, GL_DYNAMIC_DRAW);

      //Send index data
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, globalRenderData.indexBufferKey);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, *globalRenderData.indexCount * sizeof(u32), globalRenderData.indexBufferBase, GL_DYNAMIC_DRAW);

      //Bind textures from texture pool
      s32 values[16];
      for (int j = 0; j < *globalRenderData.texturesToBindCount; j++)
	{
	  glActiveTexture(GL_TEXTURE0 + j);
	  glBindTexture(GL_TEXTURE_2D, globalRenderData.texturesToBind[j]);
	  values[j] = j;
	}

      glBindAttribLocation(globalRenderData.shaderProgramKey,4, "tex");
      s32 location = glGetUniformLocation(globalRenderData.shaderProgramKey, "tex");
      //if (location)
	glUniform1iv(location, *globalRenderData.texturesToBindCount, values);

      
      setLightUniform(globalRenderData.shaderProgramKey);
      
      glDrawElements(GL_TRIANGLES, *globalRenderData.indexCount, GL_UNSIGNED_INT, NULL);
      globalRenderData.texturesToBindCount = 0;
    }
  if (globalRenderData.wireFrameMode)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  setRendererShaderMode(prev);
}

void addScreenSpaceTriangle(Vertex* v0, Vertex* v1, Vertex* v2)
{
  if (*globalRenderData.indexCount >= RENDERER_MAX_INDICES - 3)
    {
      rendererEndScene();
      rendererBeginScene();
    }
  v0->texUnit = 0.0f;
  v1->texUnit = 0.0f;
  v2->texUnit = 0.0f;
  *(globalRenderData.vertexBufferPtr  + 0) = *v0;
  *(globalRenderData.vertexBufferPtr  + 1) = *v1;
  *(globalRenderData.vertexBufferPtr  + 2) = *v2;
  globalRenderData.vertexBufferPtr += 3;
  globalRenderData.indexBufferBase[*globalRenderData.indexCount++] = (*globalRenderData.indexOffset)++;
  globalRenderData.indexBufferBase[*globalRenderData.indexCount++] = (*globalRenderData.indexOffset)++;
  globalRenderData.indexBufferBase[*globalRenderData.indexCount++] = (*globalRenderData.indexOffset)++;  
}


u32 getTextureUnitByKey(u32 texKey)
{
  for (int i = 0; i < *globalRenderData.texturesToBindCount; i++)
    {
      if (globalRenderData.texturesToBind[i] == texKey)
	{
	  return i;
	}
    }
  globalRenderData.texturesToBind[*globalRenderData.texturesToBindCount] = texKey;
  *globalRenderData.texturesToBindCount = *globalRenderData.texturesToBindCount + 1;
  return *globalRenderData.texturesToBindCount - 1;
}

void addScreenSpaceQuad(Vertex* v0, Vertex* v1, Vertex* v2, Vertex* v3, u32 texKey)
{
  if (*globalRenderData.indexCount >= RENDERER_MAX_INDICES - 6 ||
      *globalRenderData.texturesToBindCount >= 16)
    {
      rendererEndScene();
      rendererBeginScene();
    }
  u32 texUnit = getTextureUnitByKey(texKey);
  v0->texUnit = texUnit;
  v1->texUnit = texUnit;
  v2->texUnit = texUnit;
  v3->texUnit = texUnit;
  *(globalRenderData.vertexBufferPtr  + 0) = *v0;
  *(globalRenderData.vertexBufferPtr  + 1) = *v1;
  *(globalRenderData.vertexBufferPtr  + 2) = *v2;
  *(globalRenderData.vertexBufferPtr  + 3) = *v3;
  globalRenderData.vertexBufferPtr += 4; 
  globalRenderData.indexBufferBase[(*globalRenderData.indexCount)++] = *globalRenderData.indexOffset + 0;
  globalRenderData.indexBufferBase[(*globalRenderData.indexCount)++] = *globalRenderData.indexOffset + 1;
  globalRenderData.indexBufferBase[(*globalRenderData.indexCount)++] = *globalRenderData.indexOffset + 2;
  globalRenderData.indexBufferBase[(*globalRenderData.indexCount)++] = *globalRenderData.indexOffset + 0;
  globalRenderData.indexBufferBase[(*globalRenderData.indexCount)++] = *globalRenderData.indexOffset + 2;
  globalRenderData.indexBufferBase[(*globalRenderData.indexCount)++] = *globalRenderData.indexOffset + 3;
  *globalRenderData.indexOffset += 4;
}
void addScreenSpaceQuad(Vertex* v0, Vertex* v1, Vertex* v2, Vertex* v3)  
{
  addScreenSpaceQuad(v0, v1, v2, v3, defaultTexture);
}
void addScreenSpaceQuad(vec2 center, vec2 scale)
{
    Vertex a = {
    {center.x - scale.x / 2,center.y - scale.y / 2,0.0},
    {0.0,1.0,0.0},
    {0.0, 0.0}
  };
  Vertex b = {
    {center.x - scale.x / 2,center.y + scale.y / 2,0.0},
    {0.0,1.0,0.0},
    {0.0, 1.0}
  };
  Vertex c = {
    {center.x + scale.x / 2,center.y + scale.y / 2,0.0},
    {0.0,1.0,0.0},
    {1.0, 1.0}
  };
  Vertex d = {
    {center.x + scale.x / 2,center.y - scale.y / 2,0},
    {0.0,1.0,0.0},
    {1.0, 0.0}
  };
  addScreenSpaceQuad(&a, &b, &c, &d, defaultTexture);
}


void addWorldSpaceQuad(Vertex* v0, Vertex* v1, Vertex* v2, Vertex* v3, u32 texKey)
{
  mat4 matrix = mainCamera.projectionMatrix * mainCamera.viewMatrix;
  vec4 a = {v0->pos, 1.0};
  a = matrix * a;
  a /= a.w;
  v0->pos = a.xyz;
  vec4 b = {v1->pos, 1.0};
  b = matrix * b;
  b /= b.w;
  v1->pos = b.xyz;
  vec4 c = {v2->pos, 1.0};
  c = matrix * c;
  c /= c.w;
  v2->pos = c.xyz;
  vec4 d = {v3->pos, 1.0};
  d = matrix * d;
  d /= d.w;
  v3->pos = d.xyz;
  addScreenSpaceQuad(v0,v1,v2,v3,texKey);
}
void addWorldSpaceQuad(Vertex* v0, Vertex* v1, Vertex* v2, Vertex* v3)
{
  addWorldSpaceQuad(v0,v1,v2,v3,defaultTexture);
}


void addWorldSpaceTriangle(Vertex* v0, Vertex* v1, Vertex* v2)
{
  mat4 matrix = mainCamera.projectionMatrix * mainCamera.viewMatrix;
  vec4 a = {v0->pos, 1.0};
  a = matrix * a;
  a /= a.w;
  v0->pos = a.xyz;
  vec4 b = {v1->pos, 1.0};
  b = matrix * b;
  b /= b.w;
  v1->pos = b.xyz;
  vec4 c = {v2->pos, 1.0};
  c = matrix * c;
  c /= c.w;
  v2->pos = c.xyz;
  addScreenSpaceTriangle(v0,v1,v2);
}

//TODO: Calculate tight fit of directional light's orthographic matrix based on what can be seen by the main camera
void calculateDirLightPositions()
{
  vec4 corners[8];//world space
  //NBL, NBR, NTL, NTR, FBL, FBR, FTL, FTR,
  mat4 m = mainCamera.projectionMatrix * mainCamera.viewMatrix;
  mat4 matrix;
  bool result = gluInvertMatrix((float*)&m, (float*)&matrix);
  Assert(result);

  vec4 v0 = {-1, -1, -1, 1};
  vec4 offsets[8] =
    {
      {-1, -1, -1, 1},
      { 1, -1, -1, 1},
      {-1,  1, -1, 1},
      { 1,  1, -1, 1},

      {-1, -1,  1, 1},
      { 1, -1,  1, 1},
      {-1,  1,  1, 1},
      { 1,  1,  1, 1}
    };
  vec3 avgPos = {0.0, 0.0, 0.0};
  for (int i = 0; i < 8; i++)
    {

      corners[i] = matrix * offsets[i];
      corners[i].x /= corners[i].w;
      corners[i].y /= corners[i].w;
      corners[i].z /= corners[i].w;
      corners[i].w = 1.0;
      avgPos += corners[i].xyz;
    }
  avgPos /= 8;

  DirectionalLight* d = globalRenderData.dirLights + 0;
  vec3 lightPos = avgPos - d->direction;
  vec3 xAxis = {1.0, 0.0, 0.0};
  vec3 yAxis = {0.0, 1.0, 0.0};
  vec3 zAxis = {0.0, 0.0, 1.0};
  mat4 lightMatrix = Translate(-lightPos) * Rotate(d->direction.x, xAxis) * Rotate(d->direction.y, yAxis)* Rotate(d->direction.z, zAxis);
  vec4 lightViewCoords[8];
  
  vec3 max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
  vec3 min = {  FLT_MAX,  FLT_MAX,  FLT_MAX };

  for (int i = 0; i < 8; i++)
    {
      lightViewCoords[i] = lightMatrix * corners[i];//light space
      for (int coords = 0; coords < 3; coords++)
	{
	  if (lightViewCoords[i][coords] > max[coords])
	    {
	      max[coords] = lightViewCoords[i][coords];
	    }
	  if (lightViewCoords[i][coords] < min[coords])
	    {
	      min[coords] = lightViewCoords[i][coords];
	    }
	}
    }
  d->shadowMatrix = Orthographic(min.x, max.x, min.y, max.y, 0.1, 30);      
}

void glSetModelViewProjectionMatrices(u32 program, mat4* projection, mat4* view, mat4* model)
{
  s32 location = glGetUniformLocation(program, "mvpMatrix");
  if (location == -1)
    {      
      location = glGetUniformLocation(program, "vpMatrix");
      Assert(location != -1);
      mat4 vpMatrix = *projection * *view;
      glUniformMatrix4fv(location, 1, GL_FALSE, (float*)&vpMatrix);
      location = glGetUniformLocation(program, "mMatrix");
      Assert(location != -1);
      glUniformMatrix4fv(location, 1, GL_FALSE, (float*)model);
    }  
  else
    {
      mat4 mvp =  *projection * *view * *model;
      glUniformMatrix4fv(location, 1, GL_FALSE, (float*)&mvp);
    }
}

//Takes the model matrix. NOT the normal itself
void glSetNormalMatrix(u32 program, mat4* modelMatrix)
{
  s32 location = glGetUniformLocation(program, "normalMatrix");
  if (location != -1)
    {
      mat4 normalMatrix;
      gluInvertMatrix((float*)modelMatrix, (float*)&normalMatrix);
      normalMatrix = Transpose(normalMatrix);
      glUniformMatrix4fv(location, 1, GL_FALSE, (float*)&normalMatrix);
    }
}

void flushMeshesAndRender()
{
  //Set view mode
  if (globalRenderData.wireFrameMode)
    {
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
  else
    {
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
  for (int i = 0; i < globalRenderData.meshesToDrawCount; i++)
    {
      Mesh* mesh = globalRenderData.meshesToDraw[i];
      vec3 position = globalRenderData.meshTransforms[i * 3 + 0];
      vec3 rotation = globalRenderData.meshTransforms[i * 3 + 1];
      vec3 scale = globalRenderData.meshTransforms[i * 3 + 2];

      mat4 modelMatrix = 
	{
	  scale.x,    0.0,              0.0,              0.0,
	  0.0,              scale.y,    0.0,              0.0,
	  0.0,              0.0,              scale.z,    0.0,
	  position.x, position.y, position.z, 1.0
	};
      f32 a = rotation.x;
      f32 b = rotation.y;
      f32 c = rotation.z;
      mat4 rotationXMatrix =
	{
	  1, 0, 0, 0,
	  0, cos(-a), -sin(-a), 0,
	  0, sin(-a), cos(-a), 0,
	  0, 0, 0, 1
	};
      mat4 rotationYMatrix =
	{
	  cos(-b), 0, sin(-b), 0,
	  0, 1, 0, 0,
	  -sin(-b), 0, cos(-b), 0,
	  0, 0, 0, 1
	};
      mat4 rotationZMatrix =
	{
	  cos(-c), -sin(-c), 0, 0,
	  sin(-c), cos(-c), 0, 0,
	  0, 0, 1, 0,
	  0, 0, 0, 1
	};
      mat4 rotationMatrix = rotationXMatrix * rotationYMatrix * rotationZMatrix;
   
      modelMatrix = modelMatrix  * rotationMatrix;
      globalRenderData.meshModelMatrices[i] = modelMatrix;
    }
  
  //shadow map
  glBindFramebuffer(GL_FRAMEBUFFER, globalRenderData.shadowMap);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  vec3 lightPos = globalRenderData.dirLights[0].direction;
  mat4 lightProjection = Orthographic(
				      -8.0f, 8.0f,
				      -8.0f, 8.0f,
				      0.1, 20.0f
				      );
  //lightProjection = globalRenderData.dirLights[0].shadowMatrix;
  vec3 target = {0.0,0.0,0.0};
  vec3 up = {0.0,1.0,0.0};
  mat4 lightMatrix = lightProjection * LookAt(lightPos, target, up);
  glViewport(0,0,globalRenderData.shadowMapWidth, globalRenderData.shadowMapHeight);
  glCullFace(GL_FRONT);
  for (int i = 0; i < globalRenderData.meshesToDrawCount; i++)
    {
      Mesh* mesh = globalRenderData.meshesToDraw[i];
      
      glBindVertexArray(mesh->rendererData.vertexArrayKey);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->rendererData.indexBufferKey);

      u32 shadowShader;
      if (mesh->skinnedMesh)
	{
	  shadowShader = globalRenderData.skinnedShadowMapShader;
	  SkinnedMesh* skinnedMesh = (SkinnedMesh*)mesh->skinnedMesh;
	  
	  glUseProgram(globalRenderData.skinnedShadowMapShader);
	  calculateSkinnedCompositeMatrices(skinnedMesh);
	  if (skinnedMesh->currentAnimation != -1) {
	  s32 location = glGetUniformLocation(globalRenderData.skinnedShadowMapShader, "boneCompositeMatrices");
	  //mAssert(location != -1);
	  
	  glUniformMatrix4fv(location, skinnedMesh->animations->jointCount, GL_FALSE, (float*)skinnedMesh->animations->compositeMatrices);
	  }
	  errCheck();
	}
	else
	{
	  shadowShader = globalRenderData.shadowMapShader;
	  //normal path
	  glUseProgram(globalRenderData.shadowMapShader);

	}
      //assumes orthographic
      s32 location = glGetUniformLocation(globalRenderData.shadowMapShader, "mMatrix");
      glUniformMatrix4fv(location, 1, GL_FALSE, (float*)&globalRenderData.meshModelMatrices[i]);

      location = glGetUniformLocation(globalRenderData.shadowMapShader, "lightMatrix");
      glUniformMatrix4fv(location, 1, GL_FALSE, (float*)&lightMatrix);


      glDrawElements(GL_TRIANGLES, mesh->rendererData.indexCount, GL_UNSIGNED_INT, NULL);
      errCheck();      
    }
  glCullFace(GL_BACK);
  glViewport(0,0,globalRenderData.frameBufferWidth, globalRenderData.frameBufferHeight);

  glBindFramebuffer(GL_FRAMEBUFFER, globalRenderData.frontBuffer);  
  

  for (int i = 0; i < globalRenderData.meshesToDrawCount; i++)
    {
      Mesh* mesh = globalRenderData.meshesToDraw[i];
      vec3 position = globalRenderData.meshTransforms[i * 3 + 0];
      vec3 rotation = globalRenderData.meshTransforms[i * 3 + 1];
      vec3 scale = globalRenderData.meshTransforms[i * 3 + 2];
      //Bind this mesh's arrays
      glBindVertexArray(mesh->rendererData.vertexArrayKey);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->rendererData.indexBufferKey);
      glUseProgram(mesh->rendererData.shaderProgramKey);

      //Generate transformation matrix
      mat4 viewMatrix = mainCamera.viewMatrix;
      //Send matrix

      glSetModelViewProjectionMatrices(mesh->rendererData.shaderProgramKey, &mainCamera.projectionMatrix, &viewMatrix, &globalRenderData.meshModelMatrices[i]);

      glSetNormalMatrix(mesh->rendererData.shaderProgramKey, &globalRenderData.meshModelMatrices[i]);
      s32 location = glGetUniformLocation(mesh->rendererData.shaderProgramKey, "lightSpaceMatrix");
      if (location != -1)
	{
	  Assert(location != -1);
	  glUniformMatrix4fv(location, 1, GL_FALSE, (float*)&lightMatrix);
	}
      location = glGetUniformLocation(mesh->rendererData.shaderProgramKey, "blueNoise");
      if (location != -1)
	{
	  glUniform1i(location, 1);
	}

      if (mesh->skinnedMesh)
	{
	  SkinnedMesh* skinnedMesh = (SkinnedMesh*)mesh->skinnedMesh;
	  if (skinnedMesh->currentAnimation != -1) {
	  s32 location = glGetUniformLocation(mesh->rendererData.shaderProgramKey, "boneCompositeMatrices");
	  //Assert(location != -1);
	  glUniformMatrix4fv(location, skinnedMesh->animations->jointCount, GL_FALSE, (float*)skinnedMesh->animations->compositeMatrices);
	  }
	  errCheck();
	}

      vec3 cameraPos = getCameraPos(&mainCamera);
      setVec3Uniform(mesh->rendererData.shaderProgramKey, "ViewPos", cameraPos);

      setLightUniform(mesh->rendererData.shaderProgramKey);
      setMaterialUniform(mesh->rendererData.shaderProgramKey, &mesh->material);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, globalRenderData.shadowMapTexture);
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, globalRenderData.blueNoiseTex);
      glActiveTexture(GL_TEXTURE0);
      glDrawElements(GL_TRIANGLES, mesh->rendererData.indexCount, GL_UNSIGNED_INT, NULL);
      errCheck();
    }
  globalRenderData.meshesToDrawCount = 0;
}

void swapToFrameBufferAndDraw()
{

  //TODO: FIX this function and optimize it
  //calculateDirLightPositions();
  
  flushMeshesAndRender();

  
  errCheck();
  //Dont draw wireframes on the fullscreen quad
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  
  // swap to screen frameBuffer
  glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default
  glUseProgram(globalRenderData.frameBufferShader);
  glBindVertexArray(globalRenderData.frameBufferQuadVAO);

  glBindTexture(GL_TEXTURE_2D, globalRenderData.frontBufferTexture);
  //If we have autoexposure enabled, we need average luminance which is found by
  //getting 1x1 mip map

  if (globalRenderData.enabledScreenShader == 4)
    {
      //Get mip map pixel
      glGenerateMipmap(GL_TEXTURE_2D);
      int level = 1 + floorf(log2f(fmax(globalRenderData.frameBufferWidth, globalRenderData.frameBufferHeight)));
      glGetTexImage(GL_TEXTURE_2D, level - 1, GL_RGBA, GL_FLOAT, &globalRenderData.averageColour);

      //Extract luminance from pixel
      vec4 pixel = globalRenderData.averageColour;
      f32 luminanceAverage = (0.2125 * pixel.x) + (0.7154 * pixel.y) + (0.0721 * pixel.z);
      f32 luminanceTemporal = globalRenderData.luminanceTemporal + (luminanceAverage - globalRenderData.luminanceTemporal) * (1 - ExpF(-globalDeltaTime * globalRenderData.exposureChangeRate));
      globalRenderData.luminanceTemporal = luminanceTemporal;
      //Calculate exposure value
      globalRenderData.exposure = Clamp(0.2, 0.1 / luminanceTemporal, 10.0);
    }
  setFloatUniform(globalRenderData.frameBufferShader, "exposure", globalRenderData.exposure);

  glDrawArrays(GL_TRIANGLES, 0, 6); //Draw

  //Reset to wireframes if it was enabled
  if (globalRenderData.wireFrameMode)
    {
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);  
    }

  //Draw debug console without post processing
  startGLTimer(&GPUImGUITimer);
  if (debugConsoleEnabled)
    {
      drawDebugConsole();
    }
  endGLTimer(&GPUImGUITimer);
  glfwSwapBuffers(mainWindow.glWindow);  

  //Calculate gpu time for debug info
  //TODO: put under preprocessor
  f32 meshTime = getGLTimerResult(&GPUMeshTimer);
  f32 UITime = getGLTimerResult(&GPUUITimer);
  f32 ImGuiTime = getGLTimerResult(&GPUImGUITimer);
  GPUTotalTime = meshTime + UITime + ImGuiTime;

  
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  //Switch back to frameBuffer
  glBindFramebuffer(GL_FRAMEBUFFER, globalRenderData.frontBuffer);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // we're not using the stencil buffer now

   glEnable(GL_DEPTH_TEST);
  errCheck();
}


