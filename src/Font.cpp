//Currently uses prerasterized fonts
//consider re-rasterizing when the scale changes?

struct CharData
{
  vec2 planeBounds[2];//Location of the character, in regards to baseline
  vec2 location[2]; //uv coordinates of char in texture from 0 to 1 in x and y
  f32 advance; //amount to advance for this character
  s32 leftSideBearing; //where this character starts from left side
};
struct Font
{
  char* name;
  u8* texture;
  u32 width;
  u32 height;
  u32 key;
  u32 pixelScale;
  f32 baseLine;
  f32 lineHeight;
  bool msdf;
  CharData charData[256]; 
};

//Load a signed distance field image and corresponding info file
//MSDFs are generated from https://github.com/Chlumsky/msdfgen
Font* initFontMSDF(const char* distFieldFile, const char* infoFile)
{
  //Load image
  int x, y, n;
  u8* data = stbi_load(distFieldFile, &x, &y, &n, 0);
  if (!data)
    {
      fprintf(stderr,"WARNING: CANNOT OPEN FONT FILE: %s\n", distFieldFile);
      return NULL;
    }

  //Initialize font  
  Font* font = (Font*)malloc(sizeof(Font));
  font->name = (char*)malloc(strlen(distFieldFile) + 1);
  strcpy(font->name, distFieldFile);

  font->texture = data;
  font->width = x;
  font->height = y;
  font->msdf = true;
  font->lineHeight = 0;
  
  FILE* infoFileHandle = fopen(infoFile,"r");
  if (!infoFileHandle)
    {
      fprintf(stderr,"WARNING: CANNOT OPEN FONT INFO FILE: %s\n", infoFile);
      free(font->name);
      free(font);
      return NULL;
    }
  vec2 size = {font->width, font->height};

  //Info file layout: decimal char value, plane bounds (left, bottom, right, top), atlas bounds (left bottom right top) (as pixel values, not scaled 0-1)
  for (char i = 32; i < 127; i++)
    {
      u32 c;
      float planeBounds[4];
      vec2 atlasBounds[2];
      fscanf(infoFileHandle, "%d, %f, %f, %f, %f, %f, %f, %f, %f, %f\n", &c,
	     &font->charData[i].advance,
	     &font->charData[i].planeBounds[0].x,
	     &font->charData[i].planeBounds[0].y,
	     &font->charData[i].planeBounds[1].x,
	     &font->charData[i].planeBounds[1].y,
	     &atlasBounds[0].x,
	     &atlasBounds[0].y,
	     &atlasBounds[1].x,
	     &atlasBounds[1].y);
      font->charData[i].location[0] = atlasBounds[0] / size;
      font->charData[i].location[1] = atlasBounds[1] / size;
      font->lineHeight = fmax(font->lineHeight, font->charData[i].planeBounds[1].y - font->charData[i].planeBounds[0].y);

    }
  //printf("l: %f\n", font->lineHeight);
  font->pixelScale = 20;
  fclose(infoFileHandle);
  font->key = requestTextureKey(font->name, font->texture, font->width, font->height, GL_RGB);
  return font;
}
//TODO: Square the font texture. OpenGL is bad at long and narrow textures
//TODO: Consider https://github.com/Chlumsky/msdfgen
Font* initFont(const char* fontfile, int heightInPixels)
{
  //Setup
  FILE* fileHandle = fopen(fontfile, "rb");
  if (!fileHandle)
    {
      fprintf(stderr,"WARNING: CANNOT OPEN FONT FILE: %s\n", fontfile);
      return NULL;
    }
  Font* font = (Font*)malloc(sizeof(Font));
  font->name = (char*)malloc(strlen(fontfile) + 1);
  strcpy(font->name, fontfile);

  //read file and init font
  stbtt_fontinfo fontInfo;
  u8* ttf_buffer = (u8*)malloc(1 << 25);
  fread(ttf_buffer, 1, 1 << 25, fileHandle);
  stbtt_InitFont(&fontInfo, ttf_buffer, 0);

  //pack texture
  u32 textureWidth = heightInPixels * ('~' -' ');
  u32 textureHeight = heightInPixels;
  font->width = textureWidth;
  font->height = textureHeight;
  font->pixelScale = heightInPixels;
  font->texture = (u8*)malloc(heightInPixels * heightInPixels * ('~' -' ')); //w is height * 260
  font->msdf = false;
  u32 x = 0;
  u32 y = 0;
  //Get data for each character
  for (char c = ' '; c <= '~'; c++)
    {
        int w,h,i,j;
	u8* bitmap;
	int xoff, yoff;

	bitmap = stbtt_GetCodepointBitmap(&fontInfo, 0,stbtt_ScaleForPixelHeight(&fontInfo, heightInPixels), c, &w, &h, &xoff, &yoff);
	if (h > heightInPixels)
	  {
	    fprintf(stderr,"EYYY WTF HEIGHT IS BIGGER THAN HEIGHT IN PIXELS : %c\n", c);
	    fflush(stdout);
	    h = heightInPixels;
	  }
	if (!bitmap)
	  {
	    fprintf(stderr,"WARNING: stbtt_GetCodePointBitmap Error.\n");
	    continue;
	  }
	
	//copy into tecture (but also flip vertically)
	for (int bitY = 0; bitY < h; bitY++)
	  {
	    for (int bitX = 0; bitX < w; bitX++)
	      {
		u32 offset = y * textureWidth + x;
		font->texture[x + bitY * textureWidth + bitX] = bitmap[(w * h) - (w * (bitY + 1)) + bitX];
	      }
	  }

	//cache data for each character
	font->charData[c].location[0].x = (f32)x / textureWidth;
	font->charData[c].location[0].y = (f32)y / textureHeight;
	font->charData[c].location[1].x = (f32)(x + w) / textureWidth;
	font->charData[c].location[1].y = (f32)(y + h) / textureHeight;
	stbtt_GetCodepointHMetrics(&fontInfo, c,
				   (s32*)&font->charData[c].advance,
			       &font->charData[c].leftSideBearing);
	font->charData[c].planeBounds[0].y = h + yoff;
	
	//printf("%c: %d %d\n", c, font->charData[c].advance, font->charData[c].leftSideBearing);
	
	//advance pointer in texture
	x += w;
	stbtt_FreeBitmap(bitmap, NULL);
    }
  s32 x0, y0, x1, y1;
  stbtt_GetFontBoundingBox(&fontInfo, &x0, &y0, &x1, &y1);
  font->baseLine = (f32)(font->pixelScale - y0) / mainWindow.height;

  /*
  stbi_flip_vertically_on_write(1);
  if(!stbi_write_bmp("res/fonts/times.bmp", textureWidth, textureHeight, 1, font->texture))
    {
      printf("WARNING: stbi_write_bmp Error.\n");
    }
  */
  
  font->key = requestTextureKey(font->name, font->texture, font->width, font->height, GL_RED);

  //close down
  fclose(fileHandle);
  free(ttf_buffer);
  return font;
}



void deleteFont(Font* font)
{
  deleteTexture(font->name);
  free(font->name);
  free(font->texture);
  free(font);
}
void resizeFont(Font** font, int heightInPixels)
{
  Font* newlyLoaded = initFont((*font)->name, heightInPixels);
  deleteFont(*font);
  *font = newlyLoaded;
}

f32 printLetterScreenSpaceMSDF(Font* font, char ch, vec2 location, f32 layer, float pixelHeight, float kerning)
{
  //Currently using 4 because it seems to line up with the pixel height: tested with size = 720 on a 720 tall window
  f32 scale = (font->lineHeight / mainWindow.height) * pixelHeight * 4;
  //TODO: custom space with so no branching?
  if (ch == ' ')
    {
      return font->charData[ch].advance * kerning * scale;      
    }


  vec2 bl = font->charData[ch].location[0];
  vec2 tr = font->charData[ch].location[1];
  vec2 blV = location + font->charData[ch].planeBounds[0]*scale; //
  vec2 trV = location + font->charData[ch].planeBounds[1]*scale;//location + (tr - bl)*pixelHeight;
  Vertex a = {
    {blV.x,blV.y,layer},
    {0.0,1.0,0.0},
    {bl.x, bl.y}
  };
  Vertex b = {
    {blV.x,trV.y,layer},
    {0.0,1.0,0.0},
    {bl.x, tr.y}
  };
  Vertex c = {
    {trV.x,trV.y,layer},
    {0.0,1.0,0.0},
    {tr.x, tr.y}
  };
  Vertex d = {
    {trV.x,blV.y,layer},
    {0.0,1.0,0.0},
    {tr.x, bl.y}
  };

  addScreenSpaceQuad(&a,&b,&c,&d,font->key);  
  return font->charData[ch].advance * kerning * scale;
}
f32 printLetterScreenSpaceMSDF(Font* font, char ch, vec2 location, f32 layer, float pixelHeight)
{
  return printLetterScreenSpaceMSDF(font, ch, location, layer, pixelHeight, 1.0f);
}
			       
//returns advance amount
f32 printLetterScreenSpace(Font* font, char ch, vec2 location, f32 layer)
{
  //space is temporarily set to i's width
  //TODO: custom space with so no branching?
  bool space = ch == ' ';
  if (space)
    {
      ch = 'm';
    }
  vec2 bl = font->charData[ch].location[0];
  vec2 tr = font->charData[ch].location[1];

  f32 ratio =  ((tr.x - bl.x) * font->width) / ((tr.y - bl.y) * font->height) ;
  f32 pixelHeight = ((f32)font->pixelScale * (tr.y - bl.y) / mainWindow.height);
  vec2 scale = {ratio * pixelHeight, pixelHeight };

  if (space)
    {
      return scale.x / 4;
    }

  vec2 baselineCorrection = {-font->charData[ch].planeBounds[0].y / mainWindow.height};
  
  //Drawn from bottom left
  vec2 blV = location + baselineCorrection;
  vec2 trV = location + scale + baselineCorrection;
  Vertex a = {
    {blV.x,blV.y,layer},
    {0.0,1.0,0.0},
    {bl.x, bl.y}
  };
  Vertex b = {
    {blV.x,trV.y,layer},
    {0.0,1.0,0.0},
    {bl.x, tr.y}
  };
  Vertex c = {
    {trV.x,trV.y,layer},
    {0.0,1.0,0.0},
    {tr.x, tr.y}
  };
  Vertex d = {
    {trV.x,blV.y,layer},
    {0.0,1.0,0.0},
    {tr.x, bl.y}
  };

    //advance + kern advance
  /*
    problem is right now that advance is unscaled. So we get values like 500 and 944
    in order to advance properly we need to scale these so that it matches with the size of
    the characters
   */
  //f32 advance = (f32)font->charData[ch].advance / font->width;
  f32 advance = scale.x;
  addScreenSpaceQuad(&a,&b,&c,&d,font->key);  
  return advance;
}

//TODO: newlines, line wrapping, text alignment
void printStringScreenSpace(Font* font, const char* str, vec2 location, f32 layer, f32 size)
{
  const char* c = str;
  vec2 offLocation = location;
  while (*c)
    {
      if (font->msdf)
	offLocation.x += printLetterScreenSpaceMSDF(font, *c, offLocation, layer,  size);
      else
	offLocation.x += printLetterScreenSpace(font, *c, offLocation, layer);
      c++;
    }

}
void printStringScreenSpace(Font* font, const char* str, vec2 location, f32 size)
{
  printStringScreenSpace(font, str, location, 0.0, size);
}

#if 0
unsigned char ttf_buffer[1<<20];
unsigned char temp_bitmap[512*512];

stbtt_bakedchar cdata[96]; // ASCII 32..126 is 95 glyphs
GLuint ftex;

void my_stbtt_initfont(void)
{
   fread(ttf_buffer, 1, 1<<20, fopen("res/fonts/TIMES.ttf", "rb"));
   stbtt_BakeFontBitmap(ttf_buffer,0, 32.0, temp_bitmap,512,512, 32,96, cdata); // no guarantee this fits!
   
   ftex = requestTextureKey("res/fonts/TIMES.ttf", temp_bitmap, 512, 512, GL_RED);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}


u8 ttf_buffer[1<<25];
stbtt_fontinfo font;
unsigned char *bitmap;


void my_stbtt_initfont(void)
{  
   FILE* times = fopen("res/fonts/TIMES.ttf","rb");
   fread(ttf_buffer, 1, 1<<25, times);
   fclose(times);
   stbtt_InitFont(&font, ttf_buffer, 0); 
}

void my_stbtt_printletter(char c, vec2 offset)
{

  int w,h,i,j;
  int s = 400;
  bitmap = stbtt_GetCodepointBitmap(&font, 0,stbtt_ScaleForPixelHeight(&font, s), c, &w, &h, 0,0);
  if (!bitmap)
    {
      fprintf(stderr,"WARNING: stbtt_GetCodePointBitmap Error.\n");
      return;
    }
  //flip
  
  for (int i = 0; i < (h / 2); i++)
    {
      for (int j = 0; j < w; j++)
	{
	  char tmp = bitmap[i * w + j];
	  bitmap[i * w + j] = bitmap[((h-1) * w - 1) - i * w + j];
	  bitmap[((h-1) * w - 1) - i * w + j] = tmp;
	}
    }
  u32 texKey = requestTextureKey("charA", bitmap, w, h, GL_RED);
  float width = (f32)(w / 2) / mainWindow.width;
  float height = (f32)(h / 2) / mainWindow.height;
  Vertex a = {
    {-width,-height,0.0},
    {0.0,1.0,0.0},
    {0,0}
  };
  Vertex b = {
    {-width,height,0.0},
    {0.0,1.0,0.0},
    {0.0,1.0}
  };
  Vertex cd = {
    {width,height,0.0},
    {0.0,1.0,0.0},
    {1.0,1.0}
  };
  Vertex d = {
    {width,-height,0},
    {0.0,1.0,0.0},
    {1.0,0.0}
  };
  a.pos.xy += offset;
  b.pos.xy += offset;
  cd.pos.xy += offset;
  d.pos.xy += offset;

  addScreenSpaceQuad(&a,&b,&cd,&d, texKey);

  stbtt_FreeBitmap(bitmap, NULL);

}

#endif
