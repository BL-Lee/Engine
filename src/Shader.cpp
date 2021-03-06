#include "Shader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SHADER_BUFFER_SIZE 8192
u32 loadShader(const char* file, u32 type) {
  //Definitely is a better way of doing this without two buffers
  FILE* fileHandle = fopen(file, "rb");
  if (!fileHandle)
    {
      //Log when logging is made
      fprintf(stderr,"WARNING: Could not open shader file: %s\n",file);
      return (u32)-1;
    }
  char* buffer = (char*)malloc(sizeof(char) * MAX_SHADER_BUFFER_SIZE);
  char* lineBuffer = (char*)malloc(sizeof(char) * MAX_SHADER_BUFFER_SIZE);
  char* bufferPointer = buffer;
  //Load Shader data from file
  while( fgets(lineBuffer, MAX_SHADER_BUFFER_SIZE, fileHandle) )
    {
      strcpy(bufferPointer, lineBuffer);
      bufferPointer += strlen(lineBuffer);
    }
  fclose(fileHandle);

  //Load data into Shader struct
  Shader* s = (Shader*)malloc(sizeof(Shader));
  s->data   = (char*)malloc(strlen(buffer) + 1);
  strcpy(s->data, buffer);

  //Cleanup
  free(buffer);
  free(lineBuffer);

  //Create shaders with GL
  s->key = glCreateShader(type);
  glShaderSource(s->key, 1, &s->data, NULL);
  glCompileShader(s->key);
  
  return s->key;
}

void deleteShader(Shader* S) {
  free(S->data);
  free(S);
}

void compileShader(Shader* S) {
  glCompileShader(S->key);
  //Print to page, log later when logging is made
  #ifdef DEBUG_GL
    s32 status;
    glGetShaderiv(S->key, GL_COMPILE_STATUS, &status);
    char buffer[512];
    glGetShaderInfoLog(S->key, 2048, NULL, buffer);
    printf("%s",buffer);
  #endif  
}
