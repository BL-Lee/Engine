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
#include "Renderer.cpp"
#include "STL.cpp"
#include "OBJLoader.cpp"
#include "Camera.cpp"
#include "EntityRegistry.cpp"
#include "Random.cpp"
#include "Timer.h"
#include "Bitmap.cpp"
#include "UI.cpp"
#include "Font.cpp"
#include "Scene.cpp"

static u32 translationArrows;
#include "DebugConsole.cpp"

static Font* mainFont;

static TextElement globalPopupText;


//Game related
#include "CaveGeneration.cpp"
/*
Mesh* selectMesh(double mouse_x, double mouse_y)
{
  f32 hitDist = FLT_MAX;
  vec3 hitLoc;
  Ray ray;
  vec4 zero = {0.0,0.0,0.0,1.0};
  vec4 rayOrigin = (mainCamera.viewMatrix * zero);
  ray.origin = rayOrigin.xyz / rayOrigin.w;

  vec4 one = {0.0,0.0,-1.0,0.0};
  ray.direction = (mainCamera.viewMatrix * one).xyz;

  Entity* entityHit = NULL;
  Mesh* meshHit = NULL;
  
  printf("origin: %f %f %f\n", ray.origin.x, ray.origin.y, ray.origin.z);
  printf("direction: %f %f %f\n", ray.direction.x, ray.direction.y, ray.direction.z);
    
    for (int i = 0; i < MAX_REGISTRY_SIZE; i++)
    {
      if (globalEntityRegistry->occupiedIndices[i] && globalEntityRegistry->entities[i].meshes)
	{
	  Entity* e = globalEntityRegistry->entities + i;
	  if (e->visible)
	    {
	      for (int me = 0; me < e->meshCount; me++)
		{
		  Mesh* mesh = e->meshes[me];
		  for (int index = 0; index < mesh->rendererData.indexCount; index+=3)
		    {
		      Vertex vertices[3];
		      if (mesh->indices)
			{
			  vertices[0] =  mesh->vertices[mesh->indices[index + 2]];
			  vertices[1] =  mesh->vertices[mesh->indices[index + 1]];
			  vertices[2] =  mesh->vertices[mesh->indices[index + 0]];
			}
		      else
			{
			  vertices[0] = mesh->vertices[index + 0];
			  vertices[1] = mesh->vertices[index + 1];
			  vertices[2] = mesh->vertices[index + 2];
			}

		      Triangle tri;
		      tri.v0 = vertices[0].pos;
		      tri.v1 = vertices[1].pos;
		      tri.v2 = vertices[2].pos;
		      f32 hit;
		      vec3 loc;
		      if (rayTriangleCollision(&ray, &tri, &hit, &loc))
			{
			  printf("HIT! Dist: %f before: %f\n", hit, hitDist);
			  
			  if (hit < hitDist && hit > 0.0f)
			    {
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
	//meshHit->visible = false;
	entitySelected = entityHit->id;
      }
    return NULL;
}

void selectMeshCallback(GLFWwindow* window, int button, int action, int mods)
{
}
*/

void renderWindow()
{
  /* Render here */
  // Start the Dear ImGui frame
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();

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
	//selectMesh(xpos, ypos);
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
  openWindow(400,600);
  
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
	     0.001f,100.0f,
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

  //TEMP: Generate cave
  vec3i dims = {16,16,16};
  float* grid = generateCaveGrid(dims);
  Entity* cave = marchCubes(grid, dims);
  setEntityName(cave, "Cave");
  free(grid);
  loadMaterial("res/materials/defaultMaterial.mat", &cave->meshes[0]->material);

  loadScene("res/scenes/testScene.scene");
  
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

