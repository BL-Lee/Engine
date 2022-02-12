vec3 loadVec3Line(const char* tagName, const char* buffer)
{
  vec3 values;
  char tagBuffer[128];
  sprintf(tagBuffer, "%s %%f %%f %%f",tagName);
  u32 ret = sscanf(buffer, tagBuffer,
		   &values.x,
		   &values.y,
		   &values.z);
  printf("%s: %f %f %f\n", tagName, values.x,values.y,values.z);
  if (ret != 3)
    {
      fprintf(stderr,"WARNING: invalid %s tag\n", tagName);
    }
  return values;
}
