void writeVec3Line(FILE* fileHandle, const char* tagName, vec3 v)
{
  char tagBuffer[256];
  sprintf(tagBuffer, "%s %f %f %f", tagName, v.x, v.y, v.z); 
  fprintf(fileHandle, "%s\n", tagBuffer);
}

void writeFloatLine(FILE* fileHandle, const char* tagName, float v)
{
  char tagBuffer[256];
  sprintf(tagBuffer, "%s %f", tagName, v); 
  fprintf(fileHandle, "%s\n", tagBuffer);
}
