#ifndef I_STRING_HEADER
#define I_STRING_HEADER
struct I_String
{
  char* buffer;
  u32 curBufferLen;
  u32 bufferLen;
  
  I_String operator+=(const char* b);
  I_String operator+=(vec3 b);
  
};

I_String init_I_String(const char* init)
{
  I_String result;
  result.bufferLen = strnlen(init, 1024);
  result.curBufferLen = 0;
  result.buffer = (char*)malloc(result.bufferLen);
  strcpy(result.buffer, init);
  return result;
}

void delete_I_String(I_String i)
{
  free(i.buffer);
  i.buffer = 0;
}

void resize_I_String(I_String* i)
{
  i->buffer = (char*)realloc(i->buffer, i->bufferLen * 2);
  i->bufferLen *= 2;	      
}

I_String operator+(I_String& a, const char* b)
{
  u32 bLen = strnlen(b, 2048);
  if (bLen == 2048 || bLen == 2047)
    {
      fprintf(stderr, "WARNING: Might not have read full string. String.h\n");
    }

  while (a.curBufferLen + bLen > a.bufferLen)
    {
      resize_I_String(&a);
    }

  sprintf(a.buffer, "%s%s", a.buffer, b);
  
  return a;
}

I_String operator+(I_String& a, vec3 b)
{
  char buffer[256];
  sprintf(buffer, "%f %f %f", b.x, b.y, b.z);
  while (a.curBufferLen + strlen(buffer) > a.bufferLen)
    {
      resize_I_String(&a);
    }

  sprintf(a.buffer, "%s%s", a.buffer, buffer);
  
  return a;
}

I_String I_String::operator+=(vec3 b)
{
  *this = *this + b;
  return *this;
}
I_String I_String::operator+=(const char* b)
{
  *this = *this + b;
  return *this;
}

#endif
