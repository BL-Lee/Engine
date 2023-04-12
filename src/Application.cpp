//GL Includes
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

//OBJ includes
#include <OBJ_Loader.h>

//STB includes
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

//#include <msdfgen.h>

//IMGUI includes
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

//IMPLOT includes
#include <implot.h>

//CRT Includes
#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <string.h>

//Debug defines
#define DEBUG_GL 1
#define DEBUG_INIT_STATS 1
#define DEBUG_IMPORT 0

//Typedefs
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   s8;
typedef int16_t  s16; 
typedef int32_t  s32;
typedef int64_t  s64;
typedef float    f32;
typedef double   f64; 

//Handmade includes
#define HMM_PREFIX
#define HANDMADE_MATH_IMPLEMENTATION
#include "HandmadeMath.h"

//Handmade typedefs
typedef hmm_vec2 vec2;
typedef hmm_vec3 vec3;
typedef hmm_vec4 vec4;
typedef hmm_quaternion quaternion;
typedef hmm_mat4 mat4;

//Global Variables
static f64 globalStopWatch;
static f64 globalDeltaTime;
static f64 globalTimeScale = 1.0;
static u32 defaultTexture;
static u32 globalEntropy = 0x31415926;

//Debug profiling
#include "DebugConsole.h"
static GLTimer GPUMeshTimer;
static GLTimer GPUUITimer;
static GLTimer GPUImGUITimer;
static f32 GPUTotalTime;

//Temp Global
static f32 globalKerning = 1.0;
static f32 fontSize = 1.0f;


//Custom includes
#include "Timer.h"
//#include "Bitmap.cpp"
#include "UI.h"

#include "ImportUtils.cpp"
#include "DebugLogging.cpp"
#include "LinkedList.cpp"

#include "Window.cpp"
#include "Shader.cpp"
#include "Collision.cpp"
#include "Mesh.cpp"
#include "MD5Loader.cpp"
#include "Renderer.cpp"
#include "STL.cpp"
#include "OBJLoader.cpp"
#include "FBX.cpp"
#include "Camera.cpp"
#include "EntityRegistry.cpp"
#include "Random.cpp"
#include "Font.cpp"
static TextElement globalPopupText;
#include "Input.cpp"
#include "Scene.cpp"
#include "DebugSelection.cpp"
#include "ScreenSelect.cpp"
static Entity* bob;
static int bobFrame = 0;
static SkinnedAnimation* anim;
static f32 frameElapsed = 0.0f;
static SkinnedMesh* bobMesh;
static Entity* debugGizmoEntity;
#include "DebugConsole.cpp"

static Font* mainFont;



//Game related
#include "CaveGeneration.cpp"





void drawEntity(Entity* e, mat4* transform)
{
  if (!e->visible)
    {
      return;
    }
  for (int j = 0; j < e->meshCount; j++)
    {		  
      Mesh* mesh = e->meshes[j];
      if (mesh->visible)
	drawMesh(mesh, transform);
    }

  if (e->children)
    {
      for (int i = 0; i < e->childCount; i++)
	{
	  //compounded transform
	  Entity* child = e->children[i];
	  mat4 childTransform = *transform * transformationMatrixFromComponents(child->position, child->scale, child->rotation);
	  drawEntity(e->children[i], &childTransform);
	}
    }
}

void drawEntity(Entity* e)
{
  mat4 transform = transformationMatrixFromComponents(e->position, e->scale, e->rotation);
  drawEntity(e, &transform);
}

void drawAllEntities()
{
  for (int i = 0; i < MAX_REGISTRY_SIZE; i++)
    {
      if (globalEntityRegistry->occupiedIndices[i])
	{
	  Entity* e = globalEntityRegistry->entities + i;
	  if (e->parent == NULL)
	    {
	      drawEntity(e);
	    }
	}
    }
}
void renderWindow()
{
  /* Render here */
  // Start the Dear ImGui frame
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();

  //Timing info
  frameElapsed += globalDeltaTime;
  while (frameElapsed >= 1.0f / anim->frameRate) {
    anim->currentFrames[0] = bobMesh->animations->currentFrames[1];
    anim->currentFrames[1] = ( bobMesh->animations->currentFrames[1] + 1 ) % anim->frameCount;
    frameElapsed -= 1.0f / anim->frameRate;
  }

  //TEMP Animation info
  f32 interp = frameElapsed * anim->frameRate;
  anim->currentInterps[0] = 1 - interp;
  anim->currentInterps[1] = interp;

  if (globalInputBuffer->mouseHeld && globalDebugData.showConsole)
    {
      debugTranslateEntity();
    }

  //Render Entities
  startGLTimer(&GPUMeshTimer);
  drawAllEntities();
  endGLTimer(&GPUMeshTimer);
    
  //UI
  startGLTimer(&GPUUITimer);
  rendererBeginScene();
  vec2 loc1 = {-0.3,-1.0};
  setRendererShaderMode(RENDERER_UI_MODE);
  printStringScreenSpace(mainFont, "Immaculate \"Deez Nuts\" my dude. qjipyz", loc1, fontSize);

  if (globalPopupText.timeRemaining > 0)
    {
      globalPopupText.timeRemaining -= globalDeltaTime;
      printStringScreenSpace(mainFont, globalPopupText.string, globalPopupText.location, globalPopupText.size);
    }
  Vertex vertices[4] =
    {
      {0.0,0.0,0.0},
      {0.0,0.5,0.0},
      {0.5,0.5,0.0},
      {0.5,0.0,0.0}
    };
  addScreenSpaceQuad(vertices + 0, vertices + 1, vertices + 2, vertices + 3);
  rendererEndScene();
  endGLTimer(&GPUUITimer);

  //Draw origin axes
  vec3 originVertices[4] =
    {
      {0.0, 0.0, 0.0},
      {1.0, 0.0, 0.0},
      {0.0, 1.0, 0.0},
      {0.0, 0.0, 1.0},
    };
  addDebugLine(originVertices[0], originVertices[1], originVertices[1], 0.0);
  addDebugLine(originVertices[0], originVertices[2], originVertices[2], 0.0);
  addDebugLine(originVertices[0], originVertices[3], originVertices[3], 0.0);

  outlineSelectedMesh();
  //Draw with frame buffer
  swapToFrameBufferAndDraw();

  errCheck();

}

//TEMP: move light 0 in a circle
void idleFunc()
{
  globalRenderData.pointLights[0].position.x = sin(globalStopWatch);
  globalRenderData.pointLights[0].position.z = cos(globalStopWatch);
  
}

void checkCollisions(Entity* e, int startingIndex)
{
  for (int i = startingIndex; i < MAX_REGISTRY_SIZE; i++)
    {
      if (globalEntityRegistry->occupiedIndices[i] && globalEntityRegistry->entities[i].meshes)
	{
	  Entity* other = globalEntityRegistry->entities + i;	  
	  if (other != e && other->visible && other->physicsEnabled)
	    {
	      f32 hitDist;
	      if (AABBCollidingContinuous(e->collider.aabb, other->collider.aabb,
					  e->velocity, other->velocity,
					  &hitDist))
		{
		  vec3 relativeVelocity = e->velocity - other->velocity * globalDeltaTime;
		  vec3 tangent = Normalize(relativeVelocity);

		  //e->position += e->velocity * globalDeltaTime * hitDist;
		  //other->position += other->velocity * globalDeltaTime * hitDist;

		  e->velocity *= 0;
		  other->velocity *= -1;
		  //e->velocity = Dot(e->velocity, tangent) * tangent;
		  //other->velocity = Dot(other->velocity, tangent) * tangent;
		  //e->visible = false;
		  //other->visible = false;
		}

	    }
	}
    } 
  
}

//Update Physics
void physicsUpdate()
{
  for (int i = 0; i < MAX_REGISTRY_SIZE; i++)
    {
      if (globalEntityRegistry->occupiedIndices[i] && globalEntityRegistry->entities[i].meshes)
	{
	  Entity* e = globalEntityRegistry->entities + i;
	  if (globalDebugData.showAABB)
	    {
	      if (e->visible && e->physicsEnabled)
		{	      
		  setEntityAABBCollider(e);
		  addDebugLineBox(e->collider.aabb.min, e->collider.aabb.max, 0.0);
		}
	    }
	}
    }
  for (int i = 0; i < MAX_REGISTRY_SIZE; i++)
    {
      if (globalEntityRegistry->occupiedIndices[i] && globalEntityRegistry->entities[i].meshes)
	{
	  Entity* e = globalEntityRegistry->entities + i;

	  if (e->visible && e->physicsEnabled)
	    {
	      checkCollisions(e, i + 1);
	      if (e->gravityEnabled)
		{
		  e->velocity.y -= 9.80f * globalDeltaTime;
		}
	      e->position += e->velocity * globalDeltaTime;
	      e->rotation += e->angularVelocity * globalDeltaTime;
	    }
	}
    } 
}


int initEngine()
{
  initGlobalIni("engineInfo.ini");
  //Open window
   openWindow(mainWindow.height,
	      mainWindow.width);
  
  //Init input and renderer
  initInput();
  initRenderer();
  initGeneralDebugInfo();

  //Init font
  //mainFont = initFont("res/fonts/TIMES.ttf",80);
  mainFont = initFontMSDF("res/fonts/TIMES.bmp", "res/fonts/TIMES_INFO.csv");

  //Init GPU timers
  initGLTimer(&GPUMeshTimer);
  initGLTimer(&GPUUITimer);
  initGLTimer(&GPUImGUITimer);
    
  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  ImGui::StyleColorsDark();

  ImPlot::CreateContext();

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(mainWindow.glWindow, true);
  ImGui_ImplOpenGL3_Init("#version 150");

  //Init Camera
  vec3 camPos = {0.0, 0.0, 5.0};  
  vec3 camDir = -camPos;
  initCamera(&mainCamera,
	     (f32)mainWindow.width, (f32)mainWindow.height,
	     60.0f,
	     0.001f,30.0f,
	     camPos, camDir);  
  setPerspectiveMatrix(&mainCamera);

  //Init Entities
  initEntityRegistry();

  defaultTexture = requestTextureKey("textures/defaultTexture.png");

  for (int i = 0; i < RENDERER_POINT_LIGHT_COUNT; i++)
    {
      Entity* dodec = deserializeEntity("res/entities/LightGizmo.entity");
      dodec->position = globalRenderData.pointLights[i].position;
      globalRenderData.pointLights[i].entityGizmoID = dodec->id;
      dodec->visible = false;
    }

  //TEMP: Arrows for entity selection
  Entity* arrows = deserializeEntity("res/entities/translationArrows.entity");
  globalDebugData.translationArrowIds[0] = arrows->children[0]->id;
  globalDebugData.translationArrowIds[1] = arrows->children[1]->id;
  globalDebugData.translationArrowIds[2] = arrows->children[2]->id;

  Entity* parentDodec = deserializeEntity("res/entities/Dodecahedron.entity");
  Entity* childDodec = deserializeEntity("res/entities/Dodecahedron.entity");
  Entity* grandDodec = deserializeEntity("res/entities/Dodecahedron.entity");
  parentDodec->children = (Entity**)malloc(sizeof(Entity*));
  parentDodec->children[0] = childDodec;
  parentDodec->position.x = -3.0;
  parentDodec->childCount = 1;
  parentDodec->angularVelocity.x = 3.14;
  childDodec->children = (Entity**)malloc(sizeof(Entity*));
  childDodec->children[0] = grandDodec;
  childDodec->childCount = 1;
  childDodec->position.y = 1.0;
  childDodec->scale.x = 1.0;  childDodec->scale.y = 1.0;  childDodec->scale.z = 1.0;
  childDodec->parent = parentDodec;
  childDodec->angularVelocity.z = 3.14;
  grandDodec->position.x = 1.0;
  grandDodec->scale.x = 1.0;  grandDodec->scale.y = 1.0;  grandDodec->scale.z = 1.0;
  grandDodec->parent = childDodec;
  

  //SkinnedMesh* spiderMesh = loadFBX("res/models/Spider.fbx");

  Entity* spider = requestNewEntity("spider");
  Entity* debugGizmoEntity = requestNewEntity("debugGizmo");
  //debugGizmoEntity = gizmos.id;
  /*
  for (int i = 0; i < spiderMesh->meshCount; i++)
    {
      for (int j = 0; j < spiderMesh->meshes[i]->vertexCount; j += 70)
	{
	  SkinnedVertex v = ((SkinnedVertex*)spiderMesh->meshes[i]->vertices)[j];
	  Entity* dodec = deserializeEntity("res/entities/smallArrows.entity");
	  dodec->position = v.pos;
	}
    }
  */
  /*
  spider->meshes = spiderMesh->meshes;
  spider->meshCount = spiderMesh->meshCount;
  spiderMesh->currentAnimation = -1;
  
  for (int i = 0; i < spider->meshCount; i++)
    {
      addMesh(spider->meshes[i], "res/shaders/basicLightVertex.glsl", "res/shaders/basicLightFrag.glsl", skinnedDefaultlayout, 6);
      loadMaterial("res/materials/defaultMaterial.mat", &spider->meshes[i]->material);
      }*/
  //Entity* spider = deserializeEntity("res/entities/spiderOBJ.entity");

  bobMesh = loadMD5Mesh("res/models/bob_lamp_update.md5mesh");
  anim = loadMD5Anim("res/models/bob_lamp_update.md5anim", bobMesh);

  bob = requestNewEntity("bob");
  bob->position.y -= 4.0f;
  bob->position.x -= 4.0f;
  bob->rotation.x = -3.14 / 2;
  bob->meshes = bobMesh->meshes;
  bob->meshCount = bobMesh->meshCount;
  bobMesh->currentAnimation = 0;
  for (int i = 0; i < bobMesh->meshCount; i++)
    {      
      calculateNormals(bobMesh->meshes[i]);
       addMesh(bobMesh->meshes[i], "res/shaders/skinnedBasicLightVertex.glsl", "res/shaders/basicFlatFrag.glsl", skinnedDefaultlayout, 6);
      loadMaterial("res/materials/defaultMaterial.mat", &bob->meshes[i]->material);
    }
    setEntityAABBCollider(bob);
    /*

  //TEMP: Generate cave
  vec3i dims = {16,16,16};
  float* grid = generateCaveGrid(dims);
  Entity* cave = marchCubes(grid, dims);
  setEntityName(cave, "Cave");
  free(grid);
  loadMaterial("res/materials/defaultMaterial.mat", &cave->meshes[0]->material);
  */


    /*    mat4 inTest = {
      0.0, 1.0, 0.0, 0.0,
      -1.0, 0.0, 0.0, 0.0,
      0.0, 0.0, 0.0, 0.0,
      0.0, 0.0, 0.0, 1.0
    };
    mat4 outTest = {
      0.0, -1.0, 0.0, 2.0,
      1.0, 0.0, 0.0, 0.0,
      0.0, 0.0, 0.0, 0.0,
      0.0, 0.0, 0.0, 1.0
    };
    mat4 result = invertMat4(&inTest);
    for (int i = 0; i < 4; i++)
      {
	for (int j = 0; j < 4; j++)
	  {
	    Assert(result[i][j] == outTest[i][j]);
	  }
      }
    */
    
  loadScene("res/scenes/testImportModel.scene");
  //ploadScene("res/scenes/blankScene.scene");
  return 0;
}
void shutDownEngine()
{
  deleteInputBuffer();
  deleteEntityRegistry();
  deleteRenderer();
  deleteGLTimer(&GPUMeshTimer);
  deleteGLTimer(&GPUImGUITimer);
  deleteGLTimer(&GPUUITimer);
  ImPlot::DestroyContext();
   ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwTerminate();
}

int main(int argc, char** argv)
{
  initEngine();
  while (!glfwWindowShouldClose(mainWindow.glWindow))
    {
      processInputs();
      idleFunc();
      physicsUpdate();
      renderWindow();
    }
  shutDownEngine(); 
  return 0;
}

