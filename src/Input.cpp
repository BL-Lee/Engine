#include "Window.h"
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
  return glfwGetKey(mainWindow.glWindow, key);
}
void pollCursorPos(float* x, float* y)
{
  double xpos, ypos;
  glfwGetCursorPos(mainWindow.glWindow, &xpos, &ypos);
  *x = (float)xpos;
  *y = (float)ypos;
}


