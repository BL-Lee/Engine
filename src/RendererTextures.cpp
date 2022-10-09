#include "Renderer.h"
//Generates a texture width x and height y and returns the openGL key
u32 genTexture(void* data, u32 x, u32 y, u32 channelType, GLenum wrapType, GLenum filterType)
{
  u32 textureKey;
  //Gen texture  
  glGenTextures(1, &textureKey);
  glBindTexture(GL_TEXTURE_2D, textureKey);
  errCheck()  
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterType);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterType);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapType);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapType);
  errCheck();
  
  //set texture data
  // texture_data is the source data of your texture, in this case
  // its size is sizeof(unsigned char) * texture_width * texture_height * 4
  glTexImage2D(GL_TEXTURE_2D, 0, channelType, x, y, 0, channelType, GL_UNSIGNED_BYTE, data);
  errCheck();
  return textureKey;
}
//Generates a texture width x and height y and returns the openGL key
u32 genTexture(void* data, u32 x, u32 y, u32 channelType)
{
  return genTexture(data, x, y, channelType, GL_CLAMP_TO_BORDER, GL_LINEAR);
}

//Loads a texture from a file. Places the opengl Key in textureKey and returns the texture data
u8* loadTexture(u32* textureKey, const char* textureFile, GLenum wrapType, GLenum filterType)
{
  //load image
  int x, y, n;
  u8* data = stbi_load(textureFile, &x, &y, &n, 0);

  if (!data)
    {
      char texPath[512] = "res/textures/";
      strcpy(texPath + 9, textureFile);
      u8* data = stbi_load(texPath, &x, &y, &n, 0);
      if (!data)
	{
	  printf("WARNING: FAILED TO LOAD TEXTURE:%s\n", textureFile);
	  printf("STBI ERROR: %s\n",stbi_failure_reason());
	  return NULL;
	}
    }
  GLenum channelType;
  switch (n)
    {
    case 1:
      channelType = GL_RED;
      break;
    case 2:
      channelType = GL_RG;
      break;
    case 3:
      channelType = GL_RGB;
      break;
    case 4:
      channelType = GL_RGBA;
      break;
    default:
      assert(0);
      break;
    }
  *textureKey = genTexture(data, x, y, channelType, wrapType, filterType);
  return data;  
}
u8* loadTexture(u32* textureKey, const char* textureFile)
{
  return loadTexture(textureKey, textureFile, GL_CLAMP_TO_BORDER, GL_LINEAR);
}

u8* loadRawImage(const char* textureFile, s32* width, s32* height, s32* channelCount )
{
    //load image
  int x, y, n;
  u8* data = stbi_load(textureFile, width, height, channelCount, 0);

  if (!data)
    {
      char texPath[512] = "res/textures/";
      strcpy(texPath + strlen(texPath), textureFile);
      data = stbi_load(texPath, width, height, channelCount, 0);
      if (!data)
	{
	  printf("WARNING: FAILED TO LOAD TEXTURE:%s\n", textureFile);
	  printf("STBI ERROR: %s\n",stbi_failure_reason());
	  return NULL;
	}
    }
  return data;
}
//requesst a key to be put in the renderer's texture buffer pool. returns the opengl Key
u32 requestTextureKey(const char* textureName)
{  
  Assert(globalRenderData.textureCount < 64);
  Assert(globalRenderData.initialized);
  //printf("Requesting texture: %s... ", textureName);
  for (int i = 0; i < globalRenderData.textureBufferSize; i++)
    {
      if (globalRenderData.totalUsingTexture[i] &&
	  strcmp(textureName, globalRenderData.textureNames[i]) == 0)
	{
	  //printf("Exists! Total using this texture = %d\n", globalRenderData.totalUsingTexture[i] + 1);
	  globalRenderData.totalUsingTexture[i]++;
	  return globalRenderData.textureKeys[i];
	}
    }
  //printf("Placing in map.\n");
  u32 key;
  u8* data = loadTexture(&key, textureName);
  if (data)
    {
      globalRenderData.textureNames[globalRenderData.textureCount] = (char*)malloc(strlen(textureName) + 1);
      strcpy(globalRenderData.textureNames[globalRenderData.textureCount], textureName);
      
      globalRenderData.textureKeys[globalRenderData.textureCount] = key;
      globalRenderData.totalUsingTexture[globalRenderData.textureCount]++;
      
      globalRenderData.textureCount++;
      stbi_image_free(data);
      return key;
    }
  else
    {
      return (u32)-1;
    }
}

//Requests a texture key from a raw image width x and height y.
u32 requestTextureKey(const char* textureName, void* data, u32 x, u32 y, u32 channelType)
{  
  Assert(globalRenderData.textureCount < 64);
  Assert(globalRenderData.initialized);
  for (int i = 0; i < globalRenderData.textureBufferSize; i++)
    {
      if (globalRenderData.totalUsingTexture[i] &&
	  strcmp(textureName, globalRenderData.textureNames[i]) == 0)
	{
	  globalRenderData.totalUsingTexture[i]++;
	  return globalRenderData.textureKeys[i];
	}
    }

  if (data)
    {
      u32 key = genTexture(data, x, y, channelType);
      globalRenderData.textureNames[globalRenderData.textureCount] = (char*)malloc(strlen(textureName) + 1);
      strcpy(globalRenderData.textureNames[globalRenderData.textureCount], textureName);
      
      globalRenderData.textureKeys[globalRenderData.textureCount] = key;
      globalRenderData.totalUsingTexture[globalRenderData.textureCount]++;
      
      globalRenderData.textureCount++;

      return key;

    }
  else
    {
      return (u32)-1;
    }
}

//Removes a texture from the pool. If no other entities are using this texture then the texture's memory is freed
void deleteTexture(const char* textureName)
{
  for (int i = 0; i < globalRenderData.textureBufferSize; i++)
    {
      if (globalRenderData.totalUsingTexture[i] && strcmp(textureName, globalRenderData.textureNames[i]) == 0)
	{
	  //Decrement the # things using this texture
	  globalRenderData.totalUsingTexture[i]--;
	  if (globalRenderData.totalUsingTexture[i] == 0)
	    {
	      //If nothing else using it then delete it
	      glDeleteTextures(1, &globalRenderData.textureKeys[i]);
	      free(globalRenderData.textureNames[i]);
	      return;
	    }
	  return;
	}
    }
}

