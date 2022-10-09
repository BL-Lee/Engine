#define PALETTE_LUT_DIMS 64
#define PALETTE_LUT_BYTES_PER_PIXEL (3)
void writePaletteLUTFile(const char* filePath, u8* data)
{
  FILE* fileHandle = fopen(filePath, "wb");
  if (!fileHandle)
    {
      printf("WARNING: CANNOT LOAD Palette FILE: %s\n", filePath);
      return;
    }

  u32 fileSize = PALETTE_LUT_DIMS * PALETTE_LUT_DIMS * PALETTE_LUT_DIMS * PALETTE_LUT_BYTES_PER_PIXEL;

  fwrite((void*)data, fileSize, 1, fileHandle);
  fclose(fileHandle);
}
u8* generateColourLUTFromImage(const char* filePath)
{
  s32 width, height, channelCount;
  u8* palette = loadRawImage(filePath, &width, &height, &channelCount);
  if (!palette) return NULL;  
  u8* colourData = (u8*)malloc(PALETTE_LUT_DIMS * PALETTE_LUT_DIMS * PALETTE_LUT_DIMS * 3);
  Assert(channelCount == 3);//Temp, enforce RGB

  printf("Generating colour LUT....\n");
  //using euclidian distance to find closest colour
  u8* colourPtr = colourData;
  for (int x = 0; x < PALETTE_LUT_DIMS; x++)
    {
      printf("\rGenerating... %d%%", (x * 100) / PALETTE_LUT_DIMS);
      fflush(stdout);
      for (int y = 0; y < PALETTE_LUT_DIMS; y++)
	{
	  for (int z = 0; z < PALETTE_LUT_DIMS; z++)
	    {
	      f32 minDist = FLT_MAX;
	      u8 colour[3];
	      for (int s = 0; s < width * height; s++)
		{
		  int r, g, b;
		  r = palette[s * 3 + 0];
		  g = palette[s * 3 + 1];
		  b = palette[s * 3 + 2];
		  //Scaled to 0-255 just in case the PALETTE_LUT_DIMS isnt 256x256
		  int sX = ((f32)x / PALETTE_LUT_DIMS * 255);
		  int sY = ((f32)y / PALETTE_LUT_DIMS * 255);
		  int sZ = ((f32)z / PALETTE_LUT_DIMS * 255);
		  f32 sqrdist = ((sX - r) * (sX - r)) + ((sY - g) * (sY - g)) + ((sZ - b) * (sZ - b));
		  if (sqrdist < minDist)
		    {
		      colour[0] = palette[s * 3 + 0];
		      colour[1] = palette[s * 3 + 1];
		      colour[2] = palette[s * 3 + 2];
		      minDist = sqrdist;
		    }
		}
	      
	      for (int i = 0; i < 3; i++)
		{
		  *colourPtr = colour[i];
		  colourPtr++;
		}

	    }
	}
    }
  printf("\n");
  
  stbi_write_bmp("out.bmp", PALETTE_LUT_DIMS, PALETTE_LUT_DIMS * PALETTE_LUT_DIMS, 3, colourData);
  //writePaletteLUTFile("basic.palette", colourData);
  free(palette);
  return colourData;

}
u8* readPaletteLUTFile(const char* filePath, bool forceRemake)
{
  FILE* fileHandle = fopen(filePath, "rb");
  if (!fileHandle || forceRemake)
    {
      fprintf(stderr,"WARNING: CANNOT LOAD Palette FILE: %s\n", filePath);
      fprintf(stderr, "TRYING TO CREATE PALETTE FROM NAME\n");
      const char* extensions[] = {".bmp", ".png"};
      char* name = (char*)malloc(strlen(filePath) + 10);
      
      for (int i = 0; i < 2; i++)
	{
	  strcpy(name, filePath);
	  char *end = name + strlen(name);

	  while (end > name && *end != '.')
	    {
	      --end;
	    }

	  if (end > name)
	    {
	      strcpy(end, extensions[i]);
	    }
	  u8* colourData = generateColourLUTFromImage(name);
	  if (colourData)
	    {
	      writePaletteLUTFile(filePath, colourData);	      
	      fileHandle = fopen(filePath, "rb");
	      fprintf(stderr, "SUCCESSFULLY WRITTEN %s PALETTE FILE\n", name);
	      break;
	    }
	}
      free(name);
    }
  if (!fileHandle)
    {
      fprintf(stderr, "FAILED TO WRITE NEW PALETTE FILE, %s DOESNT EXIST\n", filePath);
      return NULL;
    }
  u32 fileSize = PALETTE_LUT_DIMS * PALETTE_LUT_DIMS * PALETTE_LUT_DIMS * PALETTE_LUT_BYTES_PER_PIXEL;
  u8* data =(u8*) malloc(fileSize);

  fread((void*)data, fileSize, 1, fileHandle);
  fclose(fileHandle);
  return data;
}

u8* readPaletteLUTFile(const char* filePath)
{
  return readPaletteLUTFile(filePath, false);
}
