#include "Shader.h"

#include "Skinned.cpp"
#include "Renderer.h"
//#include "RendererTextures.cpp"
#include "RendererHelpers.cpp"
//#include "RendererPrimitives.cpp"
#include "RendererDebugGeometry.cpp"
#include "Camera.h"
//#include "Palette.cpp"
/* TODO


             Renderer
      /                   \                             
      Framebuffer        VAO info       
     /\                       /                               \ 
 Texture/renderbuffer      VAOs (for every vertex format)     Mesh Info, indices into Index/vertex, shader?
                               /         |
                             IndexBuffer VertexBuffer       



      Init renderer()
         Setup VAOs
	     1. Standard meshes
	     2. Skinned meshes
	     3. Full screen final quad
	     4. Debug geometry?
	 Set Framebuffers
	     1. For output (tex for colour, renderbuffer for depth)
	     2. Shadow map (tex)
	     3. Bloom (textures)
	     4. other post processing?


      Mesh pipeline
      
      addMesh(mesh, shaders, vertex layout)
         1. find what layout matches the VAO
	 2. Find spot in buffer for the data
	 3. validate shader
	 4. generate mapping in vao, return vao id, mesh id

      drawMesh(mesh, transform)
         1. find mesh in mesh info
	 2. probably add to list of ids to draw? w/ transform

      removeMesh(meshId)
         1. find info.
	 2. delete that info
              

      Renderer pipeline

      flushMeshesAndRender()
         1. calc light info
	 2. bind appropriate frame buffer
	 2. shadow map pass w/ list of mesh ids
	 3. Render pass w/ mesh ids
	 5. bind full screen quad
	 4. bind frame buffers / textures for postprocessing
	 5. post processing
	 6. Bind output frame buffer
	 7. draw, swap buffer
	 8. clear mesh ids to draw



 */

/*

  TODO: One VAO per vertex format
  
eg:

        Mesh VAO (big VBO buffer, big Index buffer)
	Skinned mesh VAO (big VBO buffer, big Index buffer)
	
	Mesh will then have pointers into the big buffers saying start and end indices
	    then glDrawElements on that portion of the buffers


*/

/* 

  Post processing Frame buffer functions

 */
/*
void initializePostProcessingFrameBufferShaders()
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

void setPostProcessingFrameBufferShader(int shader)
{
  globalRenderData.frameBufferShader = globalRenderData.postProcessingShaders[shader];
}
*/
void genFramebuffer(GLuint* buffer)
{
  glGenFramebuffers(1, buffer);
  glBindFramebuffer(GL_FRAMEBUFFER, *buffer);
}
void gen2DTexture(GLuint* tex)
{
   //generate texture
  glGenTextures(1, tex);
  glBindTexture(GL_TEXTURE_2D, *tex);
}
//Assumes to be bound already
void set2DTexture(GLenum type, u32 width, u32 height, GLenum internalType, GLenum sampleType, GLenum wrap)
{
  glTexImage2D(GL_TEXTURE_2D, 0, type,
	       width, height,
	       0, internalType, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, sampleType);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, sampleType);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
}

void genGenericBuffer(GLuint* buffer, GLenum type, u32 size, void* data, GLenum drawType)
{
  glGenBuffers(1, buffer);
  glBindBuffer(type, *buffer);
  glBufferData(type, size, data, drawType);  
}


void genRenderBuffer(GLuint* buffer, GLenum type, GLenum bindType, u32 width, u32 height)
{
  glGenRenderbuffers(1, buffer);
  glBindRenderbuffer(GL_RENDERBUFFER, *buffer); 
  glRenderbufferStorage(GL_RENDERBUFFER, type, width, height);  

  //Bind render buffer
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, bindType, GL_RENDERBUFFER, *buffer);

  //Validate
  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
      fprintf(stderr,"WARNING: FRONT BUFFER FAILED INIT\n");
      fprintf(stderr, "ERRCODE: %d\n", glCheckFramebufferStatus(GL_FRAMEBUFFER));
    }
  
}

void initOutputFBO()
{
  if (globalRenderData.outputFBO.width == -1 || globalRenderData.outputFBO.height == -1)
    {
      globalRenderData.outputFBO.width = globalRenderData.viewportWidth;
      globalRenderData.outputFBO.height = globalRenderData.viewportWidth;
    }

  //Gen frame buffer
  genFramebuffer(&globalRenderData.outputFBO.key);
  gen2DTexture(&globalRenderData.outputFBO.textureKey);
  set2DTexture(GL_R11F_G11F_B10F, globalRenderData.outputFBO.width, globalRenderData.outputFBO.height, GL_RGB, GL_NEAREST, GL_REPEAT);
  //Bind texture
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, globalRenderData.outputFBO.textureKey, 0);

  u32 rbo;
  genRenderBuffer(&rbo, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT, globalRenderData.outputFBO.width, globalRenderData.outputFBO.height);

  glGenerateMipmap(GL_TEXTURE_2D);
}

void initShadowMapInfo()
{
  //FBO 
  globalRenderData.shadowMapFBO.width = 1024;
  globalRenderData.shadowMapFBO.height = 1024;

  genFramebuffer(&globalRenderData.shadowMapFBO.key);
  gen2DTexture(&globalRenderData.shadowMapFBO.textureKey);
  set2DTexture(GL_DEPTH_COMPONENT, globalRenderData.shadowMapFBO.width, globalRenderData.shadowMapFBO.height, GL_DEPTH_COMPONENT, GL_LINEAR, GL_REPEAT);
  
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, globalRenderData.shadowMapFBO.textureKey, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);

  
  globalRenderData.depthShader = loadAndValidateShaderPair("res/shaders/screenShadervert.glsl",
							   "res/shaders/depthFrag.glsl");
}


void initVAO(RendererVAOInfo* info, u32 vertexSize, VertexLayoutComponent* layout, u32 layoutCount, u32 shadowMapKey)
{

    //Generate gpu buffers
  //Array
  glGenVertexArrays(1, &info->key);
  glBindVertexArray(info->key);
    errCheck();
  //VertexBuffer
  genGenericBuffer(&info->VBOKey, GL_ARRAY_BUFFER,
		   vertexSize * info->maxVertexCount, NULL,
		   GL_DYNAMIC_DRAW);
  //IndexBuffer
  genGenericBuffer(&info->IBKey, GL_ELEMENT_ARRAY_BUFFER,
		   sizeof(u32) * info->maxIndexCount, NULL,
		   GL_DYNAMIC_DRAW);
  //generatez gpu vertex layout
   u64 currentOffset = 0;
  for (int i = 0; i < layoutCount; i++)
    {
      glEnableVertexAttribArray(layout[i].location);
          errCheck();
	  glVertexAttribPointer(layout[i].location, layout[i].count, layout[i].type, GL_FALSE, vertexSize, (const void*)layout[i].offset);
	  
          errCheck();
      currentOffset += layout[i].size;
    }
    errCheck();
  info->vertexSize = vertexSize;


  //SHADERS
  //Should probably be in the mesh info
  info->shadowMapProgramKey = shadowMapKey;

  info->meshesToDrawCount = 0;
  info->meshInfoCount = 0;
  info->layout = layout;
  info->layoutCount = layoutCount;
  errCheck();
}

void resizeVAO(RendererVAOInfo* info, f32 scale)
{

  //Generate gpu buffers
  //Array
  glBindVertexArray(info->key);


  errCheck();
  //VertexBuffer
  u32 newVBOKey;
  genGenericBuffer(&newVBOKey, GL_ARRAY_BUFFER,
		   (u32)(info->vertexSize * info->maxVertexCount * scale), NULL,
		   GL_DYNAMIC_DRAW);
  u32 newIBKey;
  //IndexBuffer
  genGenericBuffer(&newIBKey, GL_ELEMENT_ARRAY_BUFFER,
		   (u32)(sizeof(u32) * info->maxIndexCount * scale), NULL,
		   GL_DYNAMIC_DRAW);
  
  glBindBuffer           (GL_COPY_READ_BUFFER, info->VBOKey);
  glBindBuffer           (GL_COPY_WRITE_BUFFER, newVBOKey);  
  glCopyBufferSubData    (GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0,
			  info->vertexSize * info->maxVertexCount);

  glBindBuffer           (GL_COPY_READ_BUFFER, info->IBKey);
  glBindBuffer           (GL_COPY_WRITE_BUFFER, newIBKey);  
  glCopyBufferSubData    (GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0,
			 sizeof(u32) * info->maxIndexCount);

			  
  info->maxVertexCount = (u32)(scale * info->maxVertexCount);
  info->maxIndexCount = (u32)(scale * info->maxIndexCount);

  printf("Vert: %llu, Ind: %llu\n", info->maxVertexCount, info->maxIndexCount);
			  
  glDeleteBuffers(1, &info->IBKey);
  glDeleteBuffers(1, &info->VBOKey);

  glBindBuffer(GL_ARRAY_BUFFER, newVBOKey);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, newIBKey);

  //Attrib location is bound along with the buffer, not the vao. So need to rebind this
  VertexLayoutComponent* layout = info->layout;  
  u64 currentOffset = 0;
  for (int i = 0; i < info->layoutCount; i++)
    {
      glEnableVertexAttribArray(layout[i].location);
      errCheck();
      glVertexAttribPointer(layout[i].location, layout[i].count, layout[i].type, GL_FALSE, info->vertexSize, (const void*)layout[i].offset);
      
      errCheck();
      currentOffset += layout[i].size;
    }

  
  errCheck();
  info->IBKey = newIBKey;
  info->VBOKey = newVBOKey;
}

void initRenderer()
{
  stbi_set_flip_vertically_on_load(1);
  
  globalRenderData.wireFrameMode = 0;
  globalRenderData.initialized = 1;
  
  //globalRenderData.exposure = 0.2f;
  //globalRenderData.luminanceTemporal = 1.0f;
  //globalRenderData.exposureChangeRate = 1.0f;

  globalRenderData.dirLightCount = -1;
  globalRenderData.pointLightCount = -1;

  
  glfwGetFramebufferSize(mainWindow.glWindow, &globalRenderData.viewportWidth, &globalRenderData.viewportHeight);

#if DEBUG_INIT_STATS
  printf("VIEWPORT WIDTH/HEIGHT: %d %d\n", globalRenderData.viewportHeight, globalRenderData.viewportWidth);
#endif

  //FBOs --------------------
  initOutputFBO();
  initShadowMapInfo();

  errCheck();

  //VAOs ---------------------
  globalRenderData.standardVAO.maxVertexCount = RENDERER_STANDARD_VAO_VBO_MAX_COUNT;
  globalRenderData.standardVAO.maxIndexCount = RENDERER_STANDARD_VAO_VBO_MAX_COUNT;
  u32 shadowMapKey = loadAndValidateShaderPair("res/shaders/shadowMapVert.glsl", "res/shaders/shadowMapFrag.glsl");
  initVAO(&globalRenderData.standardVAO, sizeof(Vertex), defaultLayout, 4, shadowMapKey);
  //globalRenderData.standardVAO.meshesToDraw = (RendererMeshVAOInfo*)calloc(sizeof(RendererMeshVAOInfo), RENDERER_STANDARD_VAO_MESH_MAX_COUNT);
  globalRenderData.standardVAO.meshInfoCount = 0;

  
    errCheck();
  globalRenderData.skinnedVAO.maxVertexCount = RENDERER_SKINNED_VAO_VBO_MAX_COUNT;
  globalRenderData.skinnedVAO.maxIndexCount = RENDERER_SKINNED_VAO_VBO_MAX_COUNT;
  shadowMapKey = loadAndValidateShaderPair("res/shaders/skinnedShadowMapVert.glsl", "res/shaders/shadowMapFrag.glsl");
  initVAO(&globalRenderData.skinnedVAO, sizeof(SkinnedVertex), skinnedDefaultLayout, 6, shadowMapKey);
  //globalRenderData.skinnedVAO.meshesToDraw = (RendererMeshVAOInfo*)calloc(sizeof(RendererMeshVAOInfo), RENDERER_SKINNED_VAO_MESH_MAX_COUNT);
  globalRenderData.skinnedVAO.meshInfoCount = 0;

  
    errCheck();
  globalRenderData.screenQuadVAO.maxVertexCount = 6;
  globalRenderData.screenQuadVAO.maxIndexCount = 6;
  initVAO(&globalRenderData.screenQuadVAO, sizeof(vec2) * 2, screenDefaultLayout, 2, 0);
  errCheck();
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

  //Debug Geometry -----------------------------
  initDebugGeometryMesh();
  
  errCheck();
  globalRenderData.screenShaderProgramKey =
    loadAndValidateShaderPair( "res/shaders/screenShadervert.glsl",
			       "res/shaders/screen_basic.glsl");
    errCheck();
  errCheck();

}

RendererMeshVAOInfo* findMeshVAOInfoSlot(RendererVAOInfo* info, u32 vertexCount, u32 indexCount)
{

  if (info->meshInfoTail == NULL)
    {
      RendererMeshVAOInfo* meshInfo = (RendererMeshVAOInfo*)calloc(sizeof(RendererMeshVAOInfo), 1);
      info->meshInfoHead = meshInfo;
      info->meshInfoTail = meshInfo;
      meshInfo->valid = 1;
      meshInfo->id = 0;
      meshInfo->startVBOIndex = 0; meshInfo->endVBOIndex = vertexCount;
      meshInfo->startIBIndex = 0; meshInfo->endIBIndex = indexCount;
      return meshInfo;
    }
  else
    {
      RendererMeshVAOInfo* meshInfo = info->meshInfoTail;
      RendererMeshVAOInfo* result = (RendererMeshVAOInfo*)calloc(sizeof(RendererMeshVAOInfo), 1);
      meshInfo->next = result;
      result->prev = meshInfo;
      result->valid = 1;
      result->id = meshInfo->id + 1;
      result->startVBOIndex = meshInfo->endVBOIndex;
      result->endVBOIndex = result->startVBOIndex + vertexCount;
      result->startIBIndex = meshInfo->endIBIndex;
      result->endIBIndex = result->startIBIndex + indexCount;
      info->meshInfoTail = result;
      if (result->endVBOIndex >= info->maxVertexCount || result->endIBIndex >= info->maxIndexCount)
	{
	  //Ran out of space in buffer. Consider adding the smart fitting soon.
	  fprintf(stderr, "\n\nWARNING. RAN OUT OF SPACE IN BUFFERS. Renderer.cpp:findMeshInfoSlot RESIZING....\n\n");
	  resizeVAO(info, 2.0f);

	}
      return result;
    }
}

void addMesh(Mesh* mesh, const char* vertexShader, const char* fragmentShader, VertexLayoutComponent* layout, RendererVAOInfo* vao)
{
  glBindVertexArray(vao->key);
  RendererMeshVAOInfo* meshInfo = findMeshVAOInfoSlot(vao, mesh->vertexCount, mesh->indexCount);
  errCheck();
  //bind vertex buffer
  glBindBuffer(GL_ARRAY_BUFFER, vao->VBOKey);
  errCheck();
  //Set vertex data
  glBufferSubData(GL_ARRAY_BUFFER,
		  meshInfo->startVBOIndex * vao->vertexSize,
		  mesh->vertexCount * vao->vertexSize,
		  mesh->vertices);
  errCheck();
  //set index data
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vao->IBKey);
  errCheck();
  //Set vertex data
  glBufferSubData(GL_ELEMENT_ARRAY_BUFFER,
		  meshInfo->startIBIndex * sizeof(u32),
		  mesh->indexCount * sizeof(u32),
		  (void*)mesh->indices);

  mesh->vao = vao;
  mesh->vaoMeshInfo = meshInfo;
  meshInfo->mesh = mesh;
  errCheck();
  //Shader
  u32 program = glCreateProgram();
  Shader* vs = loadShader(vertexShader, GL_VERTEX_SHADER);
  Shader* fs = loadShader(fragmentShader, GL_FRAGMENT_SHADER);
  glAttachShader(program, vs->key);
  glAttachShader(program, fs->key);
  
  validateShaderCompilation(vs);
  validateShaderCompilation(fs);

  glLinkProgram(program);
  glValidateProgram(program);
  validateShaderLink(program);

  glDetachShader(program, vs->key);
  glDetachShader(program, fs->key);
  deleteShader(vs);
  deleteShader(fs);
  errCheck();
  meshInfo->programKey = program;
  glBindVertexArray(0);
  
}
void addMesh(Mesh* mesh, const char* vertexShader, const char* fragmentShader, VertexLayoutComponent* layout)
{
  printf("Adding mesh!\n");
  RendererVAOInfo* vao;
  if (layout == &defaultLayout[0])
      {
	vao = &globalRenderData.standardVAO;
      }
  else if (layout == &skinnedDefaultLayout[0])
      {
	vao = &globalRenderData.skinnedVAO;
      }
  else
      {
	Assert(0);
      }
  addMesh(mesh, vertexShader, fragmentShader, layout, vao);
}
void addMesh(Mesh* mesh, const char* vertexShader, const char* fragmentShader)
{
  addMesh(mesh, vertexShader, fragmentShader, defaultLayout, &globalRenderData.standardVAO);
}


void drawMesh(Mesh* mesh, vec3 position, vec3 rotation, vec3 scale)
{
  RendererVAOInfo* vao = mesh->vao;
  Assert(vao->meshesToDrawCount < RENDERER_VAO_MESH_DRAW_COUNT);
  vao->meshesToDraw[vao->meshesToDrawCount] = mesh->vaoMeshInfo;
  vao->meshModelMatrices[vao->meshesToDrawCount] = transformationMatrixFromComponents(position, scale, rotation);  
  vao->meshesToDrawCount++;
}
void drawMesh(Mesh* mesh, mat4* transform)
{
  RendererVAOInfo* vao = mesh->vao;
  Assert(vao->meshesToDrawCount < RENDERER_VAO_MESH_DRAW_COUNT);
  vao->meshesToDraw[vao->meshesToDrawCount] = mesh->vaoMeshInfo;
  vao->meshModelMatrices[vao->meshesToDrawCount] = *transform;
  vao->meshesToDrawCount++;
}

void renderPass(RendererVAOInfo* vao, mat4* lightMatrix, Camera* camera)
{
  glBindVertexArray(vao->key);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vao->IBKey);
  for (int i = 0; i < vao->meshesToDrawCount; i++)
    {
      RendererMeshVAOInfo* meshInfo = vao->meshesToDraw[i];
      glUseProgram(meshInfo->programKey);
      mat4 viewMatrix = camera->viewMatrix;
      
      glSetModelViewProjectionMatrices(meshInfo->programKey, &camera->projectionMatrix, &viewMatrix, &vao->meshModelMatrices[i]);
      glSetNormalMatrix(meshInfo->programKey, &vao->meshModelMatrices[i]);

      errCheck();
      vec3 cameraPos = getCameraPos(camera);
      setVec3Uniform(meshInfo->programKey, "ViewPos", cameraPos);

      Mesh* mesh = (Mesh*)meshInfo->mesh;
      
      setLightUniform(meshInfo->programKey);
      setMaterialUniform(meshInfo->programKey, &mesh->material);
      setLightSpaceMatrixUniform(meshInfo->programKey, lightMatrix);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, globalRenderData.shadowMapFBO.textureKey);

      if (mesh->skinnedMesh)
	{
	  setSkinnedMeshUniforms(mesh, meshInfo->programKey);
	}

      //glActiveTexture(GL_TEXTURE1);
      //glBindTexture(GL_TEXTURE_3D, globalRenderData.colourPaletteLUT);
      //glActiveTexture(GL_TEXTURE0);

      glDrawElementsBaseVertex(GL_TRIANGLES, mesh->indexCount,
			       GL_UNSIGNED_INT, (void*)(meshInfo->startIBIndex * sizeof(u32)),
			       meshInfo->startVBOIndex);
      errCheck();
    }
  vao->meshesToDrawCount = 0;
}
void shadowMapPass(RendererVAOInfo* vao, mat4* lightMatrix)
{
  //No shadows for this geometry
  if (!vao->shadowMapProgramKey){ return; }
  
  //Bind correct VAO
  glBindVertexArray(vao->key);
  glUseProgram(vao->shadowMapProgramKey);
  setLightUniform(vao->shadowMapProgramKey);
  for (int i = 0; i < vao->meshesToDrawCount; i++)
    {
      RendererMeshVAOInfo* meshInfo = vao->meshesToDraw[i];


      Mesh* mesh = (Mesh*)meshInfo->mesh;


      if (mesh->skinnedMesh)
	{
	  setSkinnedMeshUniforms(mesh, vao->shadowMapProgramKey);
	}
      
      //assumes orthographic
      s32 location = glGetUniformLocation(vao->shadowMapProgramKey, "mMatrix");
      glUniformMatrix4fv(location, 1, GL_FALSE, (float*)&vao->meshModelMatrices[i]);

      location = glGetUniformLocation(vao->shadowMapProgramKey, "lightMatrix");
      glUniformMatrix4fv(location, 1, GL_FALSE, (float*)lightMatrix);


      glDrawElementsBaseVertex(GL_TRIANGLES, mesh->indexCount,
			       GL_UNSIGNED_INT, (void*)(meshInfo->startIBIndex * sizeof(u32)),
			       meshInfo->startVBOIndex);
      
      errCheck();
    }

}

void flushMeshesAndRender(Camera* camera)
{
  vec3 lightPos = globalRenderData.dirLights[0].direction;
  mat4 lightProjection = Orthographic(
				      -8.0f, 8.0f,
				      -8.0f, 8.0f,
				      0.1, 20.0f
				      );
  vec3 target = {0.0,0.0,0.0};
  vec3 up = {0.0,1.0,0.0};  
  //calculateDirLightPositions(LookAt(lightPos, target, up));
  //mat4 lightProjection = globalRenderData.dirLights[0].shadowMatrix;
  mat4 lightMatrix = lightProjection * LookAt(lightPos, target, up);

  //---------Shadow map pass -----------
  //Bind shadowmap frame buffer
  glBindFramebuffer(GL_FRAMEBUFFER, globalRenderData.shadowMapFBO.key);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glViewport(0,0,globalRenderData.shadowMapFBO.width, globalRenderData.shadowMapFBO.height);
  glCullFace(GL_FRONT);  
  shadowMapPass(&globalRenderData.standardVAO, &lightMatrix);
  shadowMapPass(&globalRenderData.skinnedVAO, &lightMatrix);


  //---------Render pass---------------
  //Reset to other frame buffer ot draw geometry
  glCullFace(GL_BACK);
  glViewport(0,0,globalRenderData.outputFBO.width, globalRenderData.outputFBO.height);
  glBindFramebuffer(GL_FRAMEBUFFER, globalRenderData.outputFBO.key);
  errCheck();
  renderPass(&globalRenderData.standardVAO, &lightMatrix, camera);
  renderPass(&globalRenderData.skinnedVAO, &lightMatrix, camera);
  globalRenderData.skinnedVAO.meshesToDrawCount = 0;
  drawDebugGeometry();
  glViewport(0,0,globalRenderData.viewportWidth, globalRenderData.viewportHeight);

  
}

void swapToFrameBufferAndDraw(Camera* camera)
{
    //Switch back to frameBuffer
  glBindFramebuffer(GL_FRAMEBUFFER, globalRenderData.outputFBO.key);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // we're not using the stencil buffer now
  glEnable(GL_DEPTH_TEST);
  flushMeshesAndRender(camera);

  drawDebugGeometry();
  glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default
  
  glBindVertexArray(globalRenderData.screenQuadVAO.key);
  glBindTexture(GL_TEXTURE_2D, globalRenderData.outputFBO.textureKey);
  glUseProgram(globalRenderData.screenShaderProgramKey);  

  glViewport(camera->viewportMin.x,camera->viewportMin.y,
	     camera->viewportMax.x, camera->viewportMax.y);
  glDrawArrays(GL_TRIANGLES, 0, 6); //Draw
  
  //Draw debug console without post processing
  startGLTimer(&GPUImGUITimer);
  if (globalDebugData.showConsole)
    {
      drawDebugConsole();
    }
  endGLTimer(&GPUImGUITimer);
  glfwSwapBuffers(mainWindow.glWindow);  

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  errCheck();

    
}

void deleteRenderer()
{
}
u32 requestTextureKey(const char* c)
{return 0;}

void deleteTexture(const char* c)
{
}



