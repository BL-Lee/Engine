/*
//Preprocessor Defer taken from https://www.reddit.com/r/ProgrammerTIL/comments/58c6dx/til_how_to_defer_in_c/
template <typename F>
struct saucy_defer {
	F f;
	saucy_defer(F f) : f(f) {}
	~saucy_defer() { f(); }
};

template <typename F>
saucy_defer<F> defer_func(F f) {
	return saucy_defer<F>(f);
}

#define DEFER_1(x, y) x##y
#define DEFER_2(x, y) DEFER_1(x, y)
#define DEFER_3(x)    DEFER_2(x, __COUNTER__)
#define defer(code)   auto DEFER_3(_defer_) =     defer_func([&](){code;})
*/
vec3 loadVec3Line(const char* tagName, const char* buffer)
{
  vec3 values;
  char tagBuffer[128];
  sprintf(tagBuffer, "%s %%f %%f %%f",tagName);
  u32 ret = sscanf(buffer, tagBuffer,
		   &values.x,
		   &values.y,
		   &values.z);
  printf("%s: %.3f %.3f %.3f\n", tagName, values.x,values.y,values.z);
  if (ret != 3)
    {
      fprintf(stderr,"WARNING: invalid %s tag\n", tagName);
    }
  return values;
}

void filterBlankLinesUntil(FILE* fileHandle, char* buffer, u32 bufferSize, const char* target)
{
  while (!strstr(buffer, target))
    fgets(buffer, bufferSize, fileHandle);
}
