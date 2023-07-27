bool loadScene(const char* filename)
{
  FILE* fileHandle = fopen(filename, "r");
  if (!fileHandle)
    {
      fprintf(stderr,"WARNING: Cannot open scene file: %s\n", filename);
      return false;
    }
  Entity* currentEntity = 0;
  char buffer[512];
  while (fgets(buffer, 512, fileHandle))
    {
      if (strstr(buffer, "#entity"))
	{
	  vec3 values;
	  
	  u32 ret = sscanf(buffer, "#entity %s", buffer);
	  currentEntity = deserializeEntity(buffer);
	}
      else if (strstr(buffer, "#eposition"))
	{
	  Assert(currentEntity);
	  currentEntity->position = loadVec3Line("#eposition", buffer);
	}
      else if (strstr(buffer, "#erotation"))
	{
	  Assert(currentEntity);
	  currentEntity->rotation = loadVec3Line("#erotation", buffer);
	}
      else if (strstr(buffer, "#escale"))
	{
	  Assert(currentEntity);
	  currentEntity->scale = loadVec3Line("#escale", buffer);
	}
      else if (strstr(buffer, "#newDirLight"))
	{
	  globalRenderData.dirLightCount++;
	  Assert(globalRenderData.dirLightCount < RENDERER_DIRECTIONAL_LIGHT_COUNT);
	}
      else if (strstr(buffer, "#dirLightDir"))
	{
	  globalRenderData.dirLights[globalRenderData.dirLightCount].direction
	    = loadVec3Line("#dirLightDir", buffer);
	}
      else if (strstr(buffer, "#dirLightAmbient"))
	{
	  globalRenderData.dirLights[globalRenderData.dirLightCount].ambientColour
	    = loadVec3Line("#dirLightAmbient", buffer);
	}
      else if (strstr(buffer, "#dirLightDiffuse"))
	{
	  globalRenderData.dirLights[globalRenderData.dirLightCount].diffuseColour
	    = loadVec3Line("#dirLightDiffuse", buffer);
	}
      else if (strstr(buffer, "#dirLightSpecular"))
	{
	  globalRenderData.dirLights[globalRenderData.dirLightCount].specularColour
	    = loadVec3Line("#dirLightSpecular", buffer);
	}
      else if (strstr(buffer, "#newPointLight"))
	{
	  globalRenderData.pointLightCount++;
	  Assert(globalRenderData.dirLightCount < RENDERER_POINT_LIGHT_COUNT);
	}
      else if (strstr(buffer, "#pointLightAmbient"))
	{
	  globalRenderData.pointLights[globalRenderData.pointLightCount].ambientColour
	    = loadVec3Line("#pointLightAmbient", buffer);
	}
      else if (strstr(buffer, "#pointLightDiffuse"))
	{
	  globalRenderData.pointLights[globalRenderData.pointLightCount].diffuseColour
	    = loadVec3Line("#pointLightDiffuse", buffer);
	}
      else if (strstr(buffer, "#pointLightSpecular"))
	{
	  globalRenderData.pointLights[globalRenderData.pointLightCount].specularColour
	    = loadVec3Line("#pointLightSpecular", buffer);
	}
      else if (strstr(buffer, "#pointLightIntensity"))
	{
	  u32 ret = sscanf(buffer, "#pointLightIntensity %f",
			   &globalRenderData.pointLights[globalRenderData.pointLightCount].intensity);
	}
      
    }
  fclose(fileHandle);
  return true;
}
