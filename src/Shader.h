#ifndef __SHADER_HEADER
#define __SHADER_HEADER

struct Shader {
  u32 key;
  char* data;
};

Shader* loadShader(const char* file, u32 type);
void deleteShader(Shader*);
void compileShader(Shader*);

#endif
