
/*

  Shader functions

 */
void validateShaderCompilation( Shader* shader )
{
#if DEBUG_GL
  char log[8000];
  int logLength;
  int status;
  glGetShaderiv(shader->key, GL_COMPILE_STATUS, &status);
  if (status != GL_TRUE)
    {
      glGetShaderInfoLog( shader->key,
			  8000,
			  &logLength,
			  log);
      fprintf(stderr, "SHADER: %s\n", shader->data);
      fprintf(stderr, "ERROR: SHADER COMPILATION FAILED\n");
      fprintf(stderr, "Shader log length: %d\n", logLength);
      fprintf(stderr, "%s\n", log);
      Assert(0);
    }
#endif
}
void validateShaderLink(u32 programID)
{
#if DEBUG_GL
  char log[8000];
  int logLength;
  int status;
  glGetProgramiv(programID, GL_LINK_STATUS, &status);
  if (status != GL_TRUE)
    {
      glGetProgramInfoLog( programID,
			  8000,
			  &logLength,
			  log);
      fprintf(stderr, "ERROR: SHADER LINKING FAILED");
      printf("Shader log length: %d\n", logLength);
      printf("%s\n", log);
      Assert(0);

    }
#endif
}

u32 loadAndValidateShaderPair(const char* vert, const char* frag)
{
  u32 program = glCreateProgram();
  Shader* vs = loadShader(vert, GL_VERTEX_SHADER);
  validateShaderCompilation(vs);
  Shader* fs = loadShader(frag, GL_FRAGMENT_SHADER);
  validateShaderCompilation(fs);
      

  //link program
  glAttachShader(program, vs->key);
  glAttachShader(program, fs->key);
  glLinkProgram(program);           
  glValidateProgram(program);
      
  validateShaderLink(program);

  deleteShader(vs);
  deleteShader(fs);  
  
  return program;
}


/* UNIFORMS */
void setVec3Uniform(s32 program, const char* name, vec3 values)
{
  s32 location = glGetUniformLocation(program, name);
  glUniform3f(location, values.x, values.y, values.z );  
}

void setFloatUniform(s32 program, const char* name, float value)
{
  s32 location = glGetUniformLocation(program, name);
  glUniform1f(location, value );  
}

void setLightUniform(s32 program)
{
  //Set lights
  
  //Holy moly, jeepers creepers....
  //You cannot GetUniformLocation on a struct, only members of the struct
  //IE: getloc(program, "light") would NOT work because it is not a member of the struct
  s32 location = glGetUniformLocation(program, "pointLights[0].ambientColour");
  if (location != -1)
    {
      //printf("doing the thing\n");
      char uniformBuffer[64];
      for (int i = 0; i < RENDERER_POINT_LIGHT_COUNT; i++)
	{
	  sprintf(uniformBuffer, "pointLights[%d].ambientColour", i);
	  setVec3Uniform(program, uniformBuffer, globalRenderData.pointLights[i].ambientColour
			 * globalRenderData.pointLights[i].intensity);
	  sprintf(uniformBuffer, "pointLights[%d].diffuseColour", i);	  
	  setVec3Uniform(program, uniformBuffer, globalRenderData.pointLights[i].diffuseColour
			 * globalRenderData.pointLights[i].intensity);
	  sprintf(uniformBuffer, "pointLights[%d].specularColour", i);
	  setVec3Uniform(program, uniformBuffer, globalRenderData.pointLights[i].specularColour
			 * globalRenderData.pointLights[i].intensity);
	  sprintf(uniformBuffer, "pointLights[%d].position", i);
	  setVec3Uniform(program, uniformBuffer, globalRenderData.pointLights[i].position);
	  sprintf(uniformBuffer, "pointLights[%d].linearFalloff", i);	  
	  setFloatUniform(program, uniformBuffer, 1 / globalRenderData.pointLights[i].intensity);
	}
      errCheck();
    }
  location = glGetUniformLocation(program, "dirLights[0].ambientColour");
  if (location != -1)
    {
      //printf("doing the thing\n");
      char uniformBuffer[64];
      for (int i = 0; i < RENDERER_DIRECTIONAL_LIGHT_COUNT; i++)
	{
	  vec3 dir = NormalizeVec3(globalRenderData.dirLights[i].direction);
	  sprintf(uniformBuffer, "dirLights[%d].ambientColour", i);
	  setVec3Uniform(program, uniformBuffer, globalRenderData.dirLights[i].ambientColour);
	  sprintf(uniformBuffer, "dirLights[%d].diffuseColour", i);	  
	  setVec3Uniform(program, uniformBuffer, globalRenderData.dirLights[i].diffuseColour);
	  sprintf(uniformBuffer, "dirLights[%d].specularColour", i);
	  setVec3Uniform(program, uniformBuffer, globalRenderData.dirLights[i].specularColour);
	  sprintf(uniformBuffer, "dirLights[%d].direction", i);
	  setVec3Uniform(program, uniformBuffer, dir);
	}
      errCheck();
    }
  
}

void setMaterialUniform(s32 program, Material* mat)
{
  s32 location = glGetUniformLocation(program, "material.ambient");
  if (location != -1)
    {
      char uniformBuffer[64];
      setVec3Uniform(program, "material.ambient", mat->ambient);
      setVec3Uniform(program, "material.diffuse", mat->diffuse);
      setVec3Uniform(program, "material.specular", mat->specular);
      setFloatUniform(program, "material.shininess", mat->specularExponent);        
    }
}


/* MATRICES */
void glSetModelViewProjectionMatrices(u32 program, mat4* projection, mat4* view, mat4* model)
{
  s32 location = glGetUniformLocation(program, "mvpMatrix");
  if (location == -1)
    {      
      location = glGetUniformLocation(program, "vpMatrix");
      Assert(location != -1);
      mat4 vpMatrix = *projection * *view;
      glUniformMatrix4fv(location, 1, GL_FALSE, (float*)&vpMatrix);
      location = glGetUniformLocation(program, "mMatrix");
      Assert(location != -1);
      glUniformMatrix4fv(location, 1, GL_FALSE, (float*)model);
    }  
  else
    {
      mat4 mvp =  *projection * *view * *model;
      glUniformMatrix4fv(location, 1, GL_FALSE, (float*)&mvp);
    }
}

//Takes the model matrix. NOT the normal itself
void glSetNormalMatrix(u32 program, mat4* modelMatrix)
{
  s32 location = glGetUniformLocation(program, "normalMatrix");
  if (location != -1)
    {
      mat4 normalMatrix;
      gluInvertMatrix((float*)modelMatrix, (float*)&normalMatrix);
      normalMatrix = Transpose(normalMatrix);
      glUniformMatrix4fv(location, 1, GL_FALSE, (float*)&normalMatrix);
    }
}


