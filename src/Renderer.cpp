#include "Shader.h"

#include "Skinned.cpp"
#include "Renderer.h"
#include "RendererTextures.cpp"
#include "RendererHelpers.cpp"
#include "RendererPrimitives.cpp"
#include "RendererDebugGeometry.cpp"
#include "Camera.h"
#include "Palette.cpp"
/* TODO

   Consider having one big buffer that it copies data into, 
that way the binding wouldnt have to switch as much? idk,
Also that way it would just be 1 draw call for shadow maps.
Look into this

 --Update: probably one bigger one for static meshes, and keep track
 of where they are in the buffer, and small ones for dhnamic meshes
 */


//Switch the shader type for future renderer calls
//UHHHH not being set back properly
//eg: vertexBufferPtr not equaling _vertexBufferPtr[i];
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
			
void initializeFrameBuffers(u32 framebufferWidth, u32 framebufferHeight)
{
  if (globalRenderData.frameBufferHeight == -1 || globalRenderData.frameBufferWidth == -1)
    {
      globalRenderData.frameBufferHeight = framebufferHeight;
      globalRenderData.frameBufferWidth = framebufferWidth;
    }

  //Gen frame buffer
  glGenFramebuffers(1, &globalRenderData.colorHDRFrameBuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, globalRenderData.colorHDRFrameBuffer);
  
  // generate texture
  glGenTextures(1, &globalRenderData.colorHDRFrameBufferTexture);
  glBindTexture(GL_TEXTURE_2D, globalRenderData.colorHDRFrameBufferTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F,
	       globalRenderData.frameBufferWidth, globalRenderData.frameBufferHeight,
	       0, GL_RGB, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);      

  //Bind texture
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, globalRenderData.colorHDRFrameBufferTexture, 0);

  //Generate renderbuffer
  unsigned int rbo;
  glGenRenderbuffers(1, &rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo); 
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, globalRenderData.frameBufferWidth, globalRenderData.frameBufferHeight);  

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
  globalRenderData.frameBufferShader = globalRenderData.postProcessingShaders[2];


  //DISPLAY BUFFER
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

void initShadowMapInfo()
{
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
}

void initRendererColourPaletteLUT()
{

  u8* colourData = readPaletteLUTFile("res/palettes/toxicPalette.palette");
  //colour palettization
  //generate texture
  glGenTextures(1, &globalRenderData.colourPaletteLUT);
  glBindTexture(GL_TEXTURE_3D, globalRenderData.colourPaletteLUT);
  glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB8,
	       PALETTE_LUT_DIMS, PALETTE_LUT_DIMS, PALETTE_LUT_DIMS, 
	       0, GL_RGB, GL_UNSIGNED_BYTE, colourData);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  free(colourData);
}

u32 loadBloomShader(const char* fragmentShader)
{
  u32 programValue = loadAndValidateShaderPair("res/shaders/bloom/bloomVertex.glsl", fragmentShader );
  glUseProgram(programValue);
  setIntUniform(programValue, "srcTexture", 0);
  return programValue;
}

void initBloomInfo()
{
  BloomInfo* bloomInfo = &globalRenderData.bloomInfo;
  glGenFramebuffers(1, &bloomInfo->frameBuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, bloomInfo->frameBuffer);

  for (int i = 0; i < BLOOM_SAMPLE_COUNT; i++)
    {
      bloomInfo->sizes[i] = {globalRenderData.frameBufferWidth / (2 * (i + 1)), globalRenderData.frameBufferHeight / (2 * (i + 1))};
      
      glGenTextures(1, &bloomInfo->mipTextures[i]);
      glBindTexture(GL_TEXTURE_2D, bloomInfo->mipTextures[i]);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F,
		   (int)bloomInfo->sizes[i].x, (int)bloomInfo->sizes[i].y,
		   0, GL_RGB, GL_FLOAT, NULL);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);      
    }
  bloomInfo->downSampleProgram = loadBloomShader(   "res/shaders/bloom/bloomDownsample.glsl");
  bloomInfo->firstDownSampleProgram = loadBloomShader("res/shaders/bloom/bloomFirstDownsample.glsl");
  bloomInfo->upSampleProgram = loadBloomShader( "res/shaders/bloom/bloomUpsample.glsl");

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			 GL_TEXTURE_2D, bloomInfo->mipTextures[0], 0);
// setup attachments
  unsigned int attachments[1] = { GL_COLOR_ATTACHMENT0 };
  glDrawBuffers(1, attachments);

  bloomInfo->blendingProgram = loadAndValidateShaderPair("res/shaders/bloom/bloomVertex.glsl",
							 "res/shaders/bloom/bloomMix.glsl");
  glUseProgram(bloomInfo->blendingProgram);
  setIntUniform(bloomInfo->blendingProgram, "srcTexture", 0);
  setIntUniform(bloomInfo->blendingProgram, "bloomTexture", 1);

  bloomInfo->strength = 0.04f;
  bloomInfo->cutoff = 1.0f;
  
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

  globalRenderData.meshesToDrawCount = 1;

  //FRONT BUFFER
 
  glfwGetFramebufferSize(mainWindow.glWindow, &globalRenderData.viewportWidth, &globalRenderData.viewportHeight);
 

  #if DEBUG_INIT_STATS
  printf("VIEWPORT WIDTH/HEIGHT: %d %d\n", globalRenderData.viewportHeight, globalRenderData.viewportWidth);
  #endif

  globalRenderData.exposure = 0.2f;
  globalRenderData.enabledScreenShader = 0;
  setFrameBufferShader(2);
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
	      loadAndValidateShaderPair("res/shaders/text/textMSDFVert.glsl",
					"res/shaders/text/textMSDFFrag.glsl");
	  }break;
	case 1:
	  {
	    globalRenderData._shaderProgramKey[i] =
	      loadAndValidateShaderPair("res/shaders/text/textBasicVert.glsl",
					"res/shaders/text/textBasicFrag.glsl");
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

  //Decrease to upscale
  initializeFrameBuffers(globalRenderData.viewportWidth, globalRenderData.viewportHeight);
  glGenerateMipmap(GL_TEXTURE_2D);

  initRendererColourPaletteLUT();
  globalRenderData.palettize = false;

  initBloomInfo();

  initShadowMapInfo();
  u8* data = loadTexture(&globalRenderData.blueNoiseTex, "res/textures/LDR_RGB1_0.png", GL_REPEAT, GL_NEAREST);
  errCheck();
  free(data);

  initDebugGeometryMesh();
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

  glDeleteFramebuffers(1, &globalRenderData.colorHDRFrameBuffer);

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
  Shader* vs = loadShader(vertexShader, GL_VERTEX_SHADER);
  Shader* fs = loadShader(fragmentShader, GL_FRAGMENT_SHADER);

  //link program
  u32 totalLayoutSize = 0;
  glAttachShader(program, vs->key);
  for (int i = 0; i < layoutCount; i++)
    {
      glBindAttribLocation(program, layout[i].location, layout[i].name);
      totalLayoutSize += layout[i].size;
    }
  glAttachShader(program, fs->key);

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
  u64 currentOffset = 0;
  for (int i = 0; i < layoutCount; i++)
    {
      glEnableVertexAttribArray(layout[i].location);
      glVertexAttribPointer(layout[i].location, layout[i].count, layout[i].type, GL_FALSE, totalLayoutSize, (const void*)currentOffset);
      currentOffset += layout[i].size;
    }

  
  glDetachShader(program, vs->key);
  glDetachShader(program, fs->key);
  deleteShader(vs);
  deleteShader(fs);
 }

void addMesh(Mesh* mesh, const char* vertexShader, const char* fragmentShader)
{
  addMesh(mesh, vertexShader, fragmentShader, defaultLayout, 4);
}

void drawMesh(Mesh* mesh, vec3 position, vec3 rotation, vec3 scale)
{
  Assert(globalRenderData.meshesToDrawCount < RENDERER_MESH_DRAW_COUNT);
  globalRenderData.meshesToDraw[globalRenderData.meshesToDrawCount] = mesh;
  globalRenderData.meshModelMatrices[globalRenderData.meshesToDrawCount] = transformationMatrixFromComponents(position, scale, rotation);  
  globalRenderData.meshesToDrawCount++;
}
void drawMesh(Mesh* mesh, mat4* transform)
{
  Assert(globalRenderData.meshesToDrawCount < RENDERER_MESH_DRAW_COUNT);
  globalRenderData.meshesToDraw[globalRenderData.meshesToDrawCount] = mesh;
  globalRenderData.meshModelMatrices[globalRenderData.meshesToDrawCount] = *transform;
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
      //calculate how much vertex data to send
      uint32_t dataSize = (u8*)globalRenderData._vertexBufferPtr[i] - (u8*)globalRenderData._vertexBufferBase[i];
      if (dataSize == 0) continue; //continue if no vertices to draw
      
      setRendererShaderMode(i);
      glUseProgram(globalRenderData.shaderProgramKey);
      glBindVertexArray(globalRenderData.vertexArrayKey);
      
      
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

//TODO: Calculate tight fit of directional light's orthographic matrix based on what can be seen by the main camera
void calculateDirLightPositions(mat4 lightViewMatrix)
{
  vec4 corners[8];//world space
  //NBL, NBR, NTL, NTR, FBL, FBR, FTL, FTR,
  DirectionalLight* d = globalRenderData.dirLights + 0;

  getFrustumCornersWorldSpace(corners, &mainCamera);  
  vec4 lightViewCoords[8];
  
  vec3 max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
  vec3 min = {  FLT_MAX,  FLT_MAX,  FLT_MAX };
  for (int i = 0; i < 8; i++)
    {
      lightViewCoords[i] = lightViewMatrix * corners[i];//light space
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
  
  d->shadowMatrix = Orthographic(min.x, max.x, min.y, max.y, 0.01, 30.0);
}


//Assumes correct frame buffer is selected
void shadowMapPass(mat4* lightMatrix)
{
    //shadow map
  glBindFramebuffer(GL_FRAMEBUFFER, globalRenderData.shadowMap);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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
      glUniformMatrix4fv(location, 1, GL_FALSE, (float*)lightMatrix);
      errCheck();      

      glDrawElements(GL_TRIANGLES, mesh->rendererData.indexCount, GL_UNSIGNED_INT, NULL);
      errCheck();      
    }

  
}

void renderPass(mat4* lightMatrix)
{
  //Geometry pass
  for (int i = 0; i < globalRenderData.meshesToDrawCount; i++)
    {
      Mesh* mesh = globalRenderData.meshesToDraw[i];
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
	  glUniformMatrix4fv(location, 1, GL_FALSE, (float*)lightMatrix);
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
      glBindTexture(GL_TEXTURE_3D, globalRenderData.colourPaletteLUT);
      glActiveTexture(GL_TEXTURE0);
      glDrawElements(GL_TRIANGLES, mesh->rendererData.indexCount, GL_UNSIGNED_INT, NULL);
      errCheck();
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

    //Calculate all the mesh matrices and stuff
  /*  for (int i = 0; i < globalRenderData.meshesToDrawCount; i++)
    {
      Mesh* mesh = globalRenderData.meshesToDraw[i];
      vec3 position = globalRenderData.meshTransforms[i * 3 + 0];
      vec3 rotation = globalRenderData.meshTransforms[i * 3 + 1];
      vec3 scale = globalRenderData.meshTransforms[i * 3 + 2];
      globalRenderData.meshModelMatrices[i] = transformationMatrixFromComponents(position, scale, rotation);  
      }*/
  vec3 lightPos = globalRenderData.dirLights[0].direction;
  /*mat4 lightProjection = Orthographic(
				      -8.0f, 8.0f,
				      -8.0f, 8.0f,
				      0.1, 20.0f
				      );*/
  vec3 target = {0.0,0.0,0.0};
  vec3 up = {0.0,1.0,0.0};  
  calculateDirLightPositions(LookAt(lightPos, target, up));
  mat4 lightProjection = globalRenderData.dirLights[0].shadowMatrix;
  mat4 lightMatrix = lightProjection * LookAt(lightPos, target, up);

  drawDebugGeometry();
  shadowMapPass(&lightMatrix);
  //Reset to other frame buffer ot draw geometry
  glCullFace(GL_BACK);
  glViewport(0,0,globalRenderData.frameBufferWidth, globalRenderData.frameBufferHeight);
  glBindFramebuffer(GL_FRAMEBUFFER, globalRenderData.colorHDRFrameBuffer);  
  renderPass(&lightMatrix);
  
  glViewport(0,0,globalRenderData.viewportWidth, globalRenderData.viewportHeight);
  
  globalRenderData.meshesToDrawCount = 1;
  globalRenderData.debugGeometryMesh->vertexCount = 0;
  globalRenderData.debugGeometryMesh->rendererData.indexCount = 0;
}


f32 getAverageLuminanceOfFrameBuffer()
{
  //Get mip map pixel
  glGenerateMipmap(GL_TEXTURE_2D);
  int level = 1 + floorf(log2f(fmax(globalRenderData.frameBufferWidth, globalRenderData.frameBufferHeight)));
  glGetTexImage(GL_TEXTURE_2D, level - 1, GL_RGBA, GL_FLOAT, &globalRenderData.averageColour);

  //Extract luminance from pixel
  vec4 pixel = globalRenderData.averageColour;
  f32 luminanceAverage = (0.2125 * pixel.x) + (0.7154 * pixel.y) + (0.0721 * pixel.z);
  f32 luminanceTemporal = globalRenderData.luminanceTemporal + (luminanceAverage - globalRenderData.luminanceTemporal) * (1 - ExpF(-globalDeltaTime * globalRenderData.exposureChangeRate));
  return luminanceTemporal;
  
}

//General post processing pipeline
/*

  1. bind frame buffer to render to
  2. bind shader and VAO
  3. bind textures
  4. post process

 */
void bloomPasses(u32 readBuffer)
{

  BloomInfo bloomInfo = globalRenderData.bloomInfo;
  glBindFramebuffer(GL_READ_FRAMEBUFFER, globalRenderData.colorHDRFrameBuffer);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, bloomInfo.frameBuffer); // write to default framebuffer
  glBlitFramebuffer(
		    0, 0, globalRenderData.frameBufferWidth, globalRenderData.frameBufferHeight,
		    0, 0, globalRenderData.frameBufferWidth, globalRenderData.frameBufferHeight,
		    GL_COLOR_BUFFER_BIT, GL_NEAREST
		    );
  glBindFramebuffer(GL_FRAMEBUFFER, bloomInfo.frameBuffer);
  //glDrawArrays(GL_TRIANGLES, 0, 6); //Draw
  
  glUseProgram(bloomInfo.firstDownSampleProgram);
  
  glBindVertexArray(globalRenderData.frameBufferQuadVAO);
  glBindTexture(GL_TEXTURE_2D, globalRenderData.colorHDRFrameBufferTexture);
  
  glViewport(0, 0, bloomInfo.sizes[0].x, bloomInfo.sizes[0].y);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			 GL_TEXTURE_2D, bloomInfo.mipTextures[0], 0);  

  setVec2Uniform(bloomInfo.firstDownSampleProgram, "srcResolution", bloomInfo.sizes[0]);
  setFloatUniform(bloomInfo.firstDownSampleProgram, "cutoff", bloomInfo.cutoff);
  
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glBindTexture(GL_TEXTURE_2D, bloomInfo.mipTextures[0]);
  glUseProgram(bloomInfo.downSampleProgram);

  for (int i = 1; i < BLOOM_SAMPLE_COUNT; i++)
    {
      glViewport(0, 0, bloomInfo.sizes[i].x, bloomInfo.sizes[i].y);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, bloomInfo.mipTextures[i], 0);

      setVec2Uniform(bloomInfo.downSampleProgram, "srcResolution", bloomInfo.sizes[i]);
      
      glDrawArrays(GL_TRIANGLES, 0, 6);
      glBindTexture(GL_TEXTURE_2D, bloomInfo.mipTextures[i]);
    }
  glUseProgram(bloomInfo.upSampleProgram);
  setFloatUniform(bloomInfo.upSampleProgram,"filterRadius", 0.005f);

  // Enable additive blending
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE);
  glBlendEquation(GL_FUNC_ADD);
  
  for (int i = BLOOM_SAMPLE_COUNT - 1; i > 0; i--)
    {

      glBindTexture(GL_TEXTURE_2D, bloomInfo.mipTextures[i]);

      glViewport(0, 0, bloomInfo.sizes[i-1].x, bloomInfo.sizes[i-1].y);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, bloomInfo.mipTextures[i-1], 0);

      glDrawArrays(GL_TRIANGLES, 0, 6);
      glBindTexture(GL_TEXTURE_2D, bloomInfo.mipTextures[i]);
    }

  glDisable(GL_BLEND);
  glViewport(0,0,globalRenderData.viewportWidth, globalRenderData.viewportHeight);  
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void bloomMixing()
{
  //glBindFramebuffer(GL_FRAMEBUFFER,
  glUseProgram(globalRenderData.bloomInfo.blendingProgram);
  glActiveTexture(GL_TEXTURE0 + 0);
  glBindTexture(GL_TEXTURE_2D, globalRenderData.colorHDRFrameBufferTexture);
  glActiveTexture(GL_TEXTURE0 + 1);
  glBindTexture(GL_TEXTURE_2D, globalRenderData.bloomInfo.mipTextures[0]);
  glActiveTexture(GL_TEXTURE0 + 2);
  glBindTexture(GL_TEXTURE_3D, globalRenderData.colourPaletteLUT);
  errCheck();
  setFloatUniform(globalRenderData.bloomInfo.blendingProgram, "bloomStrength", globalRenderData.bloomInfo.strength);
  errCheck();
  u32 location = glGetUniformLocation(globalRenderData.bloomInfo.blendingProgram, "blueNoise");
  if (location != -1)
    {
      glUniform1i(location, 2);
    }
  location = glGetUniformLocation(globalRenderData.bloomInfo.blendingProgram, "palettize");
  if (location != -1)
    {
      glUniform1i(location, globalRenderData.palettize);
      }
  errCheck();
    {
      //globalRenderData.luminanceTemporal = getAverageLuminanceOfFrameBuffer();
      //Calculate exposure value
      //globalRenderData.exposure = Clamp(0.2, 0.1 / globalRenderData.luminanceTemporal, 10.0);
    }
  setFloatUniform(globalRenderData.bloomInfo.blendingProgram, "exposure", 0.5);

    errCheck();

  glDrawArrays(GL_TRIANGLES, 0, 6); //Draw
    errCheck();

}

void generalPostProcessingPass(u32 readFrameBuffer, u32 drawFrameBuffer)
{
  glBindFramebuffer(GL_FRAMEBUFFER, drawFrameBuffer);
  // swap to screen frameBuffer
  //glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer); // back to default
  glUseProgram(globalRenderData.frameBufferShader);
  glBindVertexArray(globalRenderData.frameBufferQuadVAO);

  //If we have autoexposure enabled, we need average luminance which is found by
  //getting 1x1 mip map
  u32 location = glGetUniformLocation(globalRenderData.frameBufferShader, "blueNoise");
  if (location != -1)
    {
      glUniform1i(location, 1);
    }

  if (globalRenderData.enabledScreenShader == 4)
    {
      globalRenderData.luminanceTemporal = getAverageLuminanceOfFrameBuffer();
      //Calculate exposure value
      globalRenderData.exposure = Clamp(0.2, 0.1 / globalRenderData.luminanceTemporal, 10.0);
    }
  setFloatUniform(globalRenderData.frameBufferShader, "exposure", globalRenderData.exposure);
  location = glGetUniformLocation(globalRenderData.frameBufferShader, "palettize");
  if (location != -1)
    {
      glUniform1i(location, globalRenderData.palettize);
    }

  glDrawArrays(GL_TRIANGLES, 0, 6); //Draw
  glBindFramebuffer(GL_READ_FRAMEBUFFER, readFrameBuffer);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFrameBuffer); // write to default framebuffer
  glBlitFramebuffer(
		    0, 0, globalRenderData.viewportWidth, globalRenderData.viewportHeight,
		    0, 0, globalRenderData.viewportWidth, globalRenderData.viewportHeight,
		    GL_COLOR_BUFFER_BIT, GL_NEAREST
		    );
}

void swapToFrameBufferAndDraw()
{

  flushMeshesAndRender();

  
  errCheck();
  //Dont draw wireframes on the fullscreen quad
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  //generalPostProcessingPass(globalRenderData.colorHDRFrameBufferTexture, globalRenderData.bloomInfo.mipTextures[0]);
  //generalPostProcessingPass(globalRenderData.colorHDRFrameBufferTexture, 0);
  bloomPasses(0);
  bloomMixing();

  //Reset to wireframes if it was enabled
  if (globalRenderData.wireFrameMode)
    {
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);  
    }

  glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default  
  glDrawArrays(GL_TRIANGLES, 0, 6); //Draw

  
  //Draw debug console without post processing
  startGLTimer(&GPUImGUITimer);
  if (globalDebugData.showConsole)
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
  glBindFramebuffer(GL_FRAMEBUFFER, globalRenderData.colorHDRFrameBuffer);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // we're not using the stencil buffer now

   glEnable(GL_DEPTH_TEST);
  errCheck();
}


