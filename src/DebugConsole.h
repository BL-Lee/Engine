#ifndef __DEBUG_CONSOLE_HEADER
#define __DEBUG_CONSOLE_HEADER

struct GLTimer
{
  u32 query;
  f32 elapsedMS;
};

void drawDebugConsole();
void startGLTimer(GLTimer* timer);
void endGLTimer(GLTimer* timer);
int queryGLTimerReady(GLTimer* timer);
f32 getGLTimerResult(GLTimer* timer);
void initGLTimer(GLTimer* timer);
#endif
