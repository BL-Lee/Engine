#ifndef I_STRING_HEADER
#define I_STRING_HEADER
struct I_String
{
  char* buffer;
  u32 curBufferLen;
  u32 bufferLen;
  
  I_String &operator+=(char* b);
  
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

I_String operator+(I_String& a, char* b)
{
  u32 bLen = strnlen(b, 2048);
  if (bLen == 2048 || bLen == 2047)
    {
      fprintf(stderr, "WARNING: Might not have read full string. String.h\n");
    }

  while (a.curBufferLen + bLen > a.bufferLen)
    {
      a.buffer = (char*)realloc(a.buffer, a.bufferLen * 2);
      a.bufferLen *= 2;	      
    }

  sprintf(a.buffer, "%s%s", a.buffer, b);
  
  return a;
}

I_String &I_String::operator+=(char* b)
{
  *this = *this + b;
  return *this;
}

#endif
