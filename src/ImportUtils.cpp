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
#import "Renderer.h"
#import "Window.h"

typedef u16 INIEnum;
#define INI_TYPE_U32 0
#define INI_TYPE_S32 1
#define INI_TYPE_STRING 2

#define INI_KEY_MAX_LENGTH (32)

struct INIEntry {
  char* key;
  INIEnum type;
  void* value;
  void* defaultValue;
  void* valueLocation;
};
  
struct INI {
  INIEntry* entries;
};

struct INIU32DefaultEntry {
  char key[INI_KEY_MAX_LENGTH];
  u32 defaultValue;
  void* valueLocation;
};

static INIU32DefaultEntry IniU32DefaultEntries[] = {						    
  //Window
  "window_width",  1080, &mainWindow.width,
  "window_height", 720, &mainWindow.height,
  "vsync", 1, &mainWindow.vSyncOn,
  "refresh_rate",  60, &mainWindow.refreshRate,

};
struct INIS32DefaultEntry {
  char key[INI_KEY_MAX_LENGTH];
  s32 defaultValue;
  void* valueLocation;
};

static INIS32DefaultEntry IniS32DefaultEntries[] = {						    
  //Window
  "frameBuffer_width", 540, &globalRenderData.frameBufferWidth, //TODO add logic to default to get width because retina dumb shit
  "frameBuffer_height",  360, &globalRenderData.frameBufferHeight,
};
struct INIStringDefaultEntry {
  char key[INI_KEY_MAX_LENGTH];
  char* defaultValue;
  void* valueLocation;
};

static INIStringDefaultEntry IniStringDefaultEntries[] = {						    
  //Renderer
};

static const char* iniTypeToFormatMappings[] = { "%d", "%d", "%s" };
static u32 iniTypeToSizeMappings[] = { sizeof(u32), sizeof(s32), sizeof(char*) };

static INI globalINI;

#define ARRAY_SIZE(x)  (sizeof(x) / sizeof((x)[0]))

/*

Fancy way would be having all the values like

void* -> {all u32 values, all string values, all s16 values... blah balh}

but sinec this is used once i feel like the malloc count doesnt really matter a whole lot

*/
void initGlobalIni(const char* fileName)
{

  /*
***********************
INIT INI struct and defaults
***********************
   */
  //  u32 totalEntries = ARRAY_SIZE(IniU32DefaultEntries);
  u32 totalEntries = 6;
  globalINI.entries = (INIEntry*)malloc(sizeof(INIEntry) * totalEntries);
  u32 currentIndex = 0;
  
  for (int i = 0; i < 4; i++)
    {
      
      INIU32DefaultEntry entry = IniU32DefaultEntries[i];
      globalINI.entries[currentIndex].key = (char*)malloc(strlen(entry.key));
      strcpy(globalINI.entries[currentIndex].key, entry.key);
      globalINI.entries[currentIndex].type = INI_TYPE_U32;
      globalINI.entries[currentIndex].defaultValue = (u32*)malloc(sizeof(u32));
      *((u32*)globalINI.entries[currentIndex].defaultValue) = entry.defaultValue;
      globalINI.entries[currentIndex].valueLocation = entry.valueLocation;
      *(u32*)entry.valueLocation = entry.defaultValue;
      globalINI.entries[currentIndex].value = (void*)malloc(sizeof(u32));
      currentIndex++;
    }

  for (int i = 0; i < 2; i++)
    {
      
      INIS32DefaultEntry entry = IniS32DefaultEntries[i];
      globalINI.entries[currentIndex].key = (char*)malloc(strlen(entry.key));
      strcpy(globalINI.entries[currentIndex].key, entry.key);
      globalINI.entries[currentIndex].type = INI_TYPE_U32;
      globalINI.entries[currentIndex].defaultValue = (s32*)malloc(sizeof(s32));
      *((s32*)globalINI.entries[currentIndex].defaultValue) = entry.defaultValue;
      globalINI.entries[currentIndex].valueLocation = entry.valueLocation;
      *(s32*)entry.valueLocation = entry.defaultValue;
      globalINI.entries[currentIndex].value = (void*)malloc(sizeof(s32));
      currentIndex++;
    }

  //Repeat for other types

  
  /***********************

LOAD INI DATA

  ************************/
  FILE* fileHandle = fopen(fileName, "r");
  if (!fileHandle)
    {
      printf("WARNING: CANNOT LOAD INI FILE: %s\n", fileName);
      return;
    }

  char* buffer = (char*)malloc(512);
  char valueString[128];  
  while (fgets(buffer, 512, fileHandle))
    {
      if (buffer[0] == '#' || buffer[0] == '[' || buffer[0] == '\n') continue;

      char* value = buffer;
      char* key = strsep(&value, "=");
      //value in buffer
      if (!value || !key) { fprintf(stderr, "WARNING: MALFORMED LINE IN INI FILE: %s\n", key); continue; }

      INIEntry* entry = NULL;
      for (int i = 0; i < totalEntries; i++)
	{
	  if (strcmp(key, globalINI.entries[i].key) == 0)
	    {
	      entry = &globalINI.entries[i];
	      break;
	    }
	}
      if (!entry)
	{
	  fprintf(stderr, "WARNING: UNKNOWN KEY IN INI FILE: %s\n", key);
	  continue;
	}


     entry->value = (void*)malloc(iniTypeToSizeMappings[entry->type]);
     switch (entry->type)
	{
	case INI_TYPE_U32:
	  {
	    if (sscanf(value, "%d", (u32*)entry->value))
	      {
		*(u32*)entry->valueLocation = *(u32*)entry->value;
	      }

	  } break;
	case INI_TYPE_S32:
	  {
	    if (sscanf(value, "%d", (s32*)entry->value))
	      {
		*(s32*)entry->valueLocation = *(s32*)entry->value;      
	      }
	  } break;
	default: break;
	}
    }
  free(buffer);  
  fclose(fileHandle);

}


void printMat4(mat4 in)
{
  for (int i = 0; i < 4; i++)
    {
      for (int j = 0; j < 4; j ++)
	{
	  printf("%f ", in[i][j]);
	}
      printf("\n");
    }
}

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
