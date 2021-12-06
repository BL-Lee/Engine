#include <stdio.h>

static FILE* DEBUG_LOG;

//Debug Macros
#ifdef DEBUG_ASSERT
 #define Assert(x) if(!(x)) { printf("ASSERTION FAILED FILE:%s LINE #%d\n", __FILE__, __LINE__); abort(); }
#else
 #define Assert(x) 
#endif

#ifdef DEBUG_GL
 #define errCheck() { GLenum err; while((err = glGetError()) != GL_NO_ERROR) { printf("GL ERROR: %x ON: FILE: %s, LINE: %d\n", err, __FILE__, __LINE__); abort(); } }
#else
 #define errCheck()
#endif

  
