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

//-5.3 8.5 4.65

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
static bool debugConsoleEnabled = false;

//Debug profiling
#include "DebugConsole.h"
static GLTimer GPUMeshTimer;
static GLTimer GPUUITimer;
static GLTimer GPUImGUITimer;
static f32 GPUTotalTime;

//TODO: entity selection in debug
static s32 entitySelected = -1;


//Temp Global
static f32 globalKerning = 1.0;
static f32 fontSize = 1.0f;


//Custom includes
#include "ImportUtils.cpp"
#include "DebugLogging.cpp"
#include "LinkedList.cpp"
#include "Input.cpp"
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
#include "Timer.h"
//#include "Bitmap.cpp"
#include "UI.cpp"
#include "Font.cpp"
#include "Scene.cpp"

static u32 translationArrows;
static Entity* bob;
static int bobFrame = 0;
static SkinnedAnimation* anim;
static f32 frameElapsed = 0.0f;
static SkinnedMesh* bobMesh;
static Entity* debugGizmoEntity;
#include "DebugConsole.cpp"

static Font* mainFont;

static TextElement globalPopupText;
static vec3 testRayDir;
static vec3 testRayOrigin;

//Game related
#include "CaveGeneration.cpp"

Mesh* selectMesh(double mouse_x, double mouse_y)
{
  f32 hitDist = FLT_MAX;
  vec3 hitLoc;
  Ray ray;
  vec4 zero = {0.0,0.0,0.0,1.0};
  ray.origin = getCameraPos(&mainCamera);
  
  vec4 one = {0.0,0.0,-1.0,0.0};
  ray.direction = (mainCamera.invViewMatrix * one).xyz;

  Entity* entityHit = NULL;
  
  if (rayCastAllNaive(&ray, &hitDist, &hitLoc, &entityHit))
    {
      entitySelected = entityHit->id;
      globalRenderData.pointLights[1].position = hitLoc;
      Entity* pLight = getEntityById(globalRenderData.pointLights[1].entityGizmoID);
      pLight->position = hitLoc;
    }
  return NULL;
}

//void selectMeshCallback(GLFWwindow* window, int button, int action, int mods)
//{
  //  selectMesh(
//}
void addDebugGizmo(vec3 location)
{
  Entity* dodec = deserializeEntity("res/entities/LightGizmo.entity");
  dodec->position = location;
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

  //Render Entities
  startGLTimer(&GPUMeshTimer);
  for (int i = 0; i < MAX_REGISTRY_SIZE; i++)
    {
      if (globalEntityRegistry->occupiedIndices[i] && globalEntityRegistry->entities[i].meshes)
	{
	  Entity* e = globalEntityRegistry->entities + i;
	  if (e->visible)
	    {
	      for (int j = 0; j < e->meshCount; j++)
		{		  
		  Mesh* mesh = e->meshes[j];
		  if (mesh->visible)
		    drawMesh(mesh, e->position, e->rotation, e->scale);
		}
	    }
	}
    }
  //drawMesh(galRenderData.debugGeometryMesh, {0.0,0.0,0.0},{0.0,0.0,0.0},{1.0,1.0,1.0});
  //addDebugRect({0.0,0.0,0.0},{1.0, 0.0, 1.0}, {0.0,1.0,1.0}, {0.0,1.0,0.0});
  addDebugLine({1.0,-1.0,0.0},{0.0, 10.0, 0.0});
  //glBindVertexArray(globalRenderData.debugGeometryMesh->rendererData.vertexArrayKey);
  //VertexBuffer

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
  rendererEndScene();
  endGLTimer(&GPUUITimer);


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

void processInputs()
{
  /* Poll for and process events */
  //glfwWaitEvents();
  glfwPollEvents();

  //Update timing info
  f64 deltaTime = glfwGetTime() - globalStopWatch;
  globalDeltaTime = deltaTime * globalTimeScale;
  globalStopWatch = glfwGetTime();

  //Camera Polling
  if (pollInputKey(GLFW_KEY_A))
    {	  
      vec3 offset = {-5.0,0.0,0.0};
      translateCameraLocal(&mainCamera, offset * globalDeltaTime);
    }
  if (pollInputKey(GLFW_KEY_D))
    {
      vec3 offset = {5.0,0.0,0.0};
      translateCameraLocal(&mainCamera, offset * globalDeltaTime);
    }
  if (pollInputKey(GLFW_KEY_W))
    {
      vec3 offset = {0.0,0.0,-5.0};
      translateCameraLocal(&mainCamera, offset * globalDeltaTime);
    }
  if (pollInputKey(GLFW_KEY_S))
    {
      vec3 offset = {0.0,0.0,5.0};
      translateCameraLocal(&mainCamera, offset * globalDeltaTime);
    }
  if (pollInputKey(GLFW_KEY_RIGHT))
    {	  
      vec3 offset = {0.0,5.0,0.0};      
      rotateCameraLocal(&mainCamera, offset * globalDeltaTime);
    }
  if (pollInputKey(GLFW_KEY_LEFT))
    {
      vec3 offset = {0.0,-5.0,0.0};
      rotateCameraLocal(&mainCamera, offset * globalDeltaTime);
    }
  if (pollInputKey(GLFW_KEY_UP))
    {	  
      vec3 offset = {5.0,0.0,0.0};
      rotateCameraLocal(&mainCamera, offset * globalDeltaTime);
    }
  if (pollInputKey(GLFW_KEY_DOWN))
    {
      vec3 offset = {-5.0,0.0,0.0};
      rotateCameraLocal(&mainCamera, offset * globalDeltaTime);
    }
  if (pollInputKey(GLFW_KEY_P))
    {
      vec4 corners[8];
      getFrustumCornersWorldSpace(corners, &mainCamera);
      for (int i = 0; i < 8; i++)
	{
	  addDebugGizmo(corners[i].xyz);
	}
    }

  //Keyboard Event Handling
  for (int i = 0; i < globalInputBuffer->size; i++)
    {
      InputInfo info = globalInputBuffer->buffer[i];
      //TEMP: Timescale buttons
      if (info.key == GLFW_KEY_1 && info.action == GLFW_PRESS)
	{
	  globalTimeScale *= 2;
	  snprintf(globalPopupText.string, 128, "Timescale: %.4f (%.3f ms)", globalTimeScale, globalTimeScale /  mainWindow.refreshRate);
	  globalPopupText.timeRemaining = 2.00;	  
	  globalPopupText.location.y = 0.3;
	  globalPopupText.location.x = -0.9;
	  globalPopupText.size = 20;

	}
      if (info.key == GLFW_KEY_2 && info.action == GLFW_PRESS)
	{
	  globalTimeScale /= 2;
	  snprintf(globalPopupText.string, 128, "Timescale: %.4f (%.3f ms)", globalTimeScale, globalTimeScale / mainWindow.refreshRate);
	  globalPopupText.timeRemaining = 2.00;	  
	  globalPopupText.location.y = 0.3;
	  globalPopupText.size = 20;
	}
      //Debug Console button
      if (info.key == GLFW_KEY_GRAVE_ACCENT && info.action == GLFW_PRESS)
	{
	  debugConsoleEnabled = !debugConsoleEnabled;
	}
      //TODO: Entity selection
      if (info.key == GLFW_MOUSE_BUTTON_LEFT && info.action == GLFW_PRESS)
      {
	double xpos, ypos;
	glfwGetCursorPos(mainWindow.glWindow, &xpos, &ypos);
	selectMesh(xpos, ypos);
      }	    
    }
  clearInputBuffer();
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
	      if (AABBColliding(e->collider.aabb, other->collider.aabb))
		{
		  e->velocity *= -1;
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
	  if (e->visible && e->physicsEnabled)
	    {
	      setEntityAABBCollider(e);
	      addDebugLineBox(e->collider.aabb.min, e->collider.aabb.max);
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
  //Open window
  openWindow(720,1080);
  
  //Init input and renderer
  initInput();
  initRenderer();

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
  Entity* arrows = deserializeEntity("res/entities/Arrows.entity");
  arrows->visible = false;
  translationArrows = arrows->id;


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
  //bobMesh = loadMD5Mesh("res/models/Spider.md5mesh");
  //anim = loadMD5Anim("res/models/Spider.md5anim", bobMesh);

  /*   bob = requestNewEntity("bob");
  bob->position.y -= 4.0f;
  bob->rotation.x = -3.14 / 2;
  bob->meshes = bobMesh->meshes;
  bob->meshCount = bobMesh->meshCount;
  bobMesh->currentAnimation = 0;
  for (int i = 0; i < bobMesh->meshCount; i++)
    {      
      calculateNormals(bobMesh->meshes[i]);
       addMesh(bobMesh->meshes[i], "res/shaders/skinnedBasicLightVertex.glsl", "res/shaders/basicLightFrag.glsl", skinnedDefaultlayout, 6);
      loadMaterial("res/materials/defaultMaterial.mat", &bob->meshes[i]->material);
    }
    setEntityAABBCollider(bob);*/
  /*

  //TEMP: Generate cave
  vec3i dims = {16,16,16};
  float* grid = generateCaveGrid(dims);
  Entity* cave = marchCubes(grid, dims);
  setEntityName(cave, "Cave");
  free(grid);
  loadMaterial("res/materials/defaultMaterial.mat", &cave->meshes[0]->material);
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

