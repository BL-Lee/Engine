#ifndef __WINDOW_HEADER
#define __WINDOW_HEADER

#include <GL/glew.h>
#include <GLFW/glfw3.h>


typedef struct _Window
{
  GLFWwindow* glWindow;
  u32 width;
  u32 height;
  s32 refreshRate;
  u32 vSyncOn;
}Window;

Window mainWindow;

#endif






