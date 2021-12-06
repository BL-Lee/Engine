#ifndef __TEXTURE_HEADER
#define __TEXTURE_HEADER

u32 requestTextureKey(const char* textureName);
u8* loadTexture(u32* textureKey, const char* textureFile);
  
#endif
