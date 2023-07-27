#include "Window.h"
#include "ScreenSelect.h"
#include "DebugSelection.h"
#define INPUT_BUFFER_SIZE 1024

//Define a key code for scroll. GLFW goes up to 348 as of August 21, 2021.
//Choose some random number in the hopes no one else chooses it
#define KEY_SCROLL 50102

//TODO: Gamepad, mouse position, joycon, path drop input,
struct InputInfo
{
  int key;
  int action;
  int mods;
  float xoffset;
  float yoffset;
};
struct InputBuffer
{
  InputInfo buffer[INPUT_BUFFER_SIZE];
  int size;
  bool mouseHeld;
  vec2 mouseDragStart;
  vec3 mouseDragStartWorld;
  vec3 mouseDragStartOffsetWorld;
};
static InputBuffer* globalInputBuffer;

//Input events
void clearInputBuffer()
{
  globalInputBuffer->size = 0;
}
void pushInput(InputInfo i)
{
  globalInputBuffer->buffer[globalInputBuffer->size] = i;
  globalInputBuffer->size++;
  Assert(globalInputBuffer->size < INPUT_BUFFER_SIZE);
}
//Callbacks for glfw to use when an input is sent
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  InputInfo info = {key, action, mods};
  pushInput(info);
}
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
  InputInfo info = {button, action, mods};
  pushInput(info);
}
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
  InputInfo info = {KEY_SCROLL, 0,0, xoffset, yoffset};
  pushInput(info);
}
void initInput()
{
  globalInputBuffer = (InputBuffer*)calloc(1, sizeof(InputBuffer));
  glfwSetKeyCallback(mainWindow.glWindow, keyCallback);
  glfwSetMouseButtonCallback(mainWindow.glWindow, mouseButtonCallback);
  glfwSetScrollCallback(mainWindow.glWindow, scrollCallback);
}
void deleteInputBuffer()
{
  Assert(globalInputBuffer);
  free(globalInputBuffer);
}

//Input polling
bool pollInputKey(int key)
{
  bool ret = glfwGetKey(mainWindow.glWindow, key);
  errCheck();
  return ret;
}
vec2 pollCursorPos()
{
  double xpos, ypos;
  glfwGetCursorPos(mainWindow.glWindow, &xpos, &ypos);
  return {xpos, ypos};
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
  errCheck();
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
  errCheck();
  //Event Handling
  for (int i = 0; i < globalInputBuffer->size; i++)
    {
      InputInfo info = globalInputBuffer->buffer[i];
      /*      //TEMP: Timescale buttons
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
	  }*/
      //Debug Console button
      if (info.key == GLFW_KEY_GRAVE_ACCENT && info.action == GLFW_PRESS)
	{
	  globalDebugData.showConsole = !globalDebugData.showConsole;
	}
      if (info.key == GLFW_MOUSE_BUTTON_LEFT)
	{
	  if (!isHoveringDebugConsole())
	    {
	      if (info.action == GLFW_PRESS)
		{
		  globalInputBuffer->mouseHeld = true;
		  globalInputBuffer->mouseDragStart = pollCursorPos();
		  selectMesh(globalInputBuffer->mouseDragStart);
		}

	      //Released Mouse
	      if (info.action == GLFW_RELEASE)
		{	
		  globalInputBuffer->mouseHeld = false;
		}
	    }
	}
    }
  clearInputBuffer();
  errCheck();
}
