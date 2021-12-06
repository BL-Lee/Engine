#include "Window.h"
#include <stdio.h>


s32 getMonitorRefreshRateHz() {
    const GLFWvidmode * mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    return mode->refreshRate;
}
vec2 getMonitorResolution() {
    const GLFWvidmode * mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    vec2 resolution  = {mode->width,mode->height};
    return resolution;
}
void setVSync(int setting)
{
  glfwSwapInterval(setting);
  mainWindow.vSyncOn = setting;
}
void openWindow(u32 height, u32 width)
{  
  mainWindow.width = width;
  mainWindow.height = height;

  /* Initialize the library */
  if (!glfwInit())
    printf("GLFW Init Error\n");

  /* Settings */
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

  /* Create a windowed mode window and its OpenGL context */
  mainWindow.glWindow = glfwCreateWindow(width, height, "Window Title", NULL, NULL);
  if (!mainWindow.glWindow)
    {
      glfwTerminate();
      return;
    }
  //glfwSetWindowUserPointer(m_Window, &m_Data);
  glewExperimental = GL_TRUE;

  /* Make the window's context current */
  glfwMakeContextCurrent(mainWindow.glWindow);


  setVSync(1);
  mainWindow.vSyncOn = 1;
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL); //LEQUAL to allow text elements on the same Z plane to not overlap
  glewExperimental = GL_TRUE;
  if (glewInit() != GLEW_OK)
    {
      printf("GLEW Error\n");
    }

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable( GL_BLEND );
  glClearColor(0.0,0.0,0.3,0.0);

  mainWindow.refreshRate = getMonitorRefreshRateHz();

  #ifdef DEBUG_GL
  //glEnable(GL_DEBUG_OUTPUT);    
  //glDebugMessageCallback(MessageCallback, 0); //unavailable on OpenGL 4.1, which is what OSX gives, why do they stop?
  #endif
}
