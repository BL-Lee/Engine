#ifndef __DEBUG_CONSOLE_HEADER
#define __DEBUG_CONSOLE_HEADER

struct GLTimer
{
  u32 query;
  f32 elapsedMS;
};

#define DEBUG_TIMING_HISTORY_LENGTH (5 * 60) //5 seconds of history at an assumed 60fps
struct GlobalDebugData
{
  bool showConsole;
  bool showAABB;
  
  f32 msHistory[DEBUG_TIMING_HISTORY_LENGTH];
  f32 fpsHistory[DEBUG_TIMING_HISTORY_LENGTH];
  u32 wrappedGraphIndex;
  f32 weightedMS;
  f32 weightedFPS;
  f32 xAxis[DEBUG_TIMING_HISTORY_LENGTH];
  u32 selectedEntityId;
  u32 translationArrowIds[3];
};

static GlobalDebugData globalDebugData;

void initDebugInfo();
void drawDebugConsole();
void startGLTimer(GLTimer* timer);
void endGLTimer(GLTimer* timer);
int queryGLTimerReady(GLTimer* timer);
f32 getGLTimerResult(GLTimer* timer);
void initGLTimer(GLTimer* timer);
bool isHoveringDebugConsole();
#endif
