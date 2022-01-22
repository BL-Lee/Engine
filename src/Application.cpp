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
  Mesh* meshHit = NULL;
  //testRayDir = ray.direction;
  //testRayOrigin = ray.origin;
  //printf("origin: %f %f %f\n", ray.origin.x, ray.origin.y, ray.origin.z);
  //printf("direction: %f %f %f\n", ray.direction.x, ray.direction.y, ray.direction.z);

    for (int i = 0; i < MAX_REGISTRY_SIZE; i++)
    {
      if (globalEntityRegistry->occupiedIndices[i] && globalEntityRegistry->entities[i].meshes)
	{
	  Entity* e = globalEntityRegistry->entities + i;
	  if (e->visible)
	    {
	      
	      mat4 modelMatrix = 
		{
		  e->scale.x,    0.0,              0.0,              0.0,
		  0.0,              e->scale.y,    0.0,              0.0,
		  0.0,              0.0,              e->scale.z,    0.0,
		  e->position.x, e->position.y, e->position.z, 1.0
		};
	      f32 a = e->rotation.x;
	      f32 b = e->rotation.y;
	      f32 c = e->rotation.z;
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
	      
	      for (int me = 0; me < e->meshCount; me++)
		{
		  Mesh* mesh = e->meshes[me];
		  for (int index = 0; index < mesh->rendererData.indexCount; index+=3)
		    {
		      //THESE ARE NOT IN WORLD SPACE YET
		      vec3 positions[3];
		      if (mesh->indices)
			{
			  positions[0] =  mesh->vertices[mesh->indices[index + 0]].pos;
			  positions[1] =  mesh->vertices[mesh->indices[index + 1]].pos;
			  positions[2] =  mesh->vertices[mesh->indices[index + 2]].pos;
			}
		      else
			{
			  positions[0] = mesh->vertices[index + 0].pos;
			  positions[1] = mesh->vertices[index + 1].pos;
			  positions[2] = mesh->vertices[index + 2].pos;
			}
		      vec4 worldPositions[3] = 
			{
			  {positions[0].x,positions[0].y,positions[0].z,1.0},
			  {positions[1].x,positions[1].y,positions[1].z,1.0},
			  {positions[2].x,positions[2].y,positions[2].z,1.0}
			};
		      for (int b = 0; b < 3; b ++)
			{
			  worldPositions[b] = modelMatrix * worldPositions[b];
			  worldPositions[b] /= worldPositions[b].w;
			}
		      
		      Triangle tri;
		      tri.v0 = worldPositions[0].xyz;
		      tri.v1 = worldPositions[1].xyz;
		      tri.v2 = worldPositions[2].xyz;
		      
		      f32 hit;
		      vec3 loc;
		      if (rayTriangleCollision(&ray, &tri, &hit, &loc))
			{

			  if (hit < hitDist && hit > 0.0f)
			    {
			      //printf("HIT! Dist: %f before: %f\n\tloc: %f %f %f\n", hit, hitDist, loc.x, loc.y, loc.z);
			      hitDist = hit;
			      hitLoc = loc;
			      entityHit = e;
			      meshHit = mesh;
			    }
			}
		    }
		}
	    }
	}
    }
    if (entityHit)
      {
	entitySelected = entityHit->id;
	globalRenderData.pointLights[1].position = entityHit->position;
	Entity* pLight = getEntityById(globalRenderData.pointLights[1].entityGizmoID);
	pLight->position = entityHit->position;
      }
    return NULL;
}

void selectMeshCallback(GLFWwindow* window, int button, int action, int mods)
{
  //  selectMesh(
}


void renderWindow()
{
  /* Render here */
  // Start the Dear ImGui frame
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();

  frameElapsed += globalDeltaTime;
  if (frameElapsed >= 1 / anim->frameRate) {
    bobFrame = ( bobFrame + 1 ) % anim->frameCount;
    frameElapsed = 0.0f;
    updateVertexPositionsManually(bobMesh, bobFrame);
  }
  
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
      vec3 offset = {0.0,180.0,0.0};      
      rotateCameraLocal(&mainCamera, offset * globalDeltaTime);
    }
  if (pollInputKey(GLFW_KEY_LEFT))
    {
      vec3 offset = {0.0,-180.0,0.0};
      rotateCameraLocal(&mainCamera, offset * globalDeltaTime);
    }
  if (pollInputKey(GLFW_KEY_UP))
    {	  
      vec3 offset = {-180.0,0.0,0.0};
      rotateCameraLocal(&mainCamera, offset * globalDeltaTime);
    }
  if (pollInputKey(GLFW_KEY_DOWN))
    {
      vec3 offset = {180.0,0.0,0.0};
      rotateCameraLocal(&mainCamera, offset * globalDeltaTime);
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

//Update Physics
void physicsUpdate()
{
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
		    {
		      e->position += e->velocity * globalDeltaTime;
		      e->rotation += e->angularVelocity * globalDeltaTime;
		    }
		}
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
	     0.001f,20.0f,
	     camPos, camDir);  
  setPerspectiveMatrix(&mainCamera);

  //Init Entities
  initEntityRegistry();

  //TEMP: Init scene
  //TODO: Scene file serialization
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

  bobMesh = loadMD5Mesh("res/models/bob_lamp_update.md5mesh");
  anim = loadMD5Anim("res/models/bob_lamp_update.md5anim", bobMesh);
  bob = requestNewEntity("bob");
  bob->position.y -= 4.0f;
  bob->rotation.x = -3.14 / 2;
  bob->meshes = bobMesh->meshes;
  bob->meshCount = bobMesh->meshCount;
  for (int i = 0; i < bobMesh->meshCount; i++)
    {      
      calculateNormals(bobMesh->meshes[i]);
      addMesh(bobMesh->meshes[i], "res/shaders/basicLightVertex.glsl", "res/shaders/basicLightFrag.glsl");
      loadMaterial("res/materials/defaultMaterial.mat", &bob->meshes[i]->material);
    }
  bob->scale.x = 1.0;
  bob->scale.y = 1.0;
  bob->scale.z = 1.0;
  /*
  for (int i = 0; i < anim->jointCount; i++)
    {
      vec4 pos = {0.0, 0.0, 0.0, 1.0};
      pos = anim->frames[0].joints[i].world * pos;
      Entity* dodec = deserializeEntity("res/entities/smallArrows.entity");
      pos /= -pos.w;
      dodec->position = pos.xyz;      
    }
  */
  /*
  //TEMP: Generate cave
  vec3i dims = {16,16,16};
  float* grid = generateCaveGrid(dims);
  Entity* cave = marchCubes(grid, dims);
  setEntityName(cave, "Cave");
  free(grid);
  loadMaterial("res/materials/defaultMaterial.mat", &cave->meshes[0]->material);
  */
  loadScene("res/scenes/blankScene.scene");
  
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

