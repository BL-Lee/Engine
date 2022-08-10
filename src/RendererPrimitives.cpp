
void addScreenSpaceTriangle(Vertex* v0, Vertex* v1, Vertex* v2)
{
  if (*globalRenderData.indexCount >= RENDERER_MAX_INDICES - 3)
    {
      rendererEndScene();
      rendererBeginScene();
    }
  v0->texUnit = 0.0f;
  v1->texUnit = 0.0f;
  v2->texUnit = 0.0f;
  *(globalRenderData.vertexBufferPtr  + 0) = *v0;
  *(globalRenderData.vertexBufferPtr  + 1) = *v1;
  *(globalRenderData.vertexBufferPtr  + 2) = *v2;
  globalRenderData.vertexBufferPtr += 3;
  globalRenderData.indexBufferBase[*globalRenderData.indexCount++] = (*globalRenderData.indexOffset)++;
  globalRenderData.indexBufferBase[*globalRenderData.indexCount++] = (*globalRenderData.indexOffset)++;
  globalRenderData.indexBufferBase[*globalRenderData.indexCount++] = (*globalRenderData.indexOffset)++;  
}
void addScreenSpaceQuad(Vertex* v0, Vertex* v1, Vertex* v2, Vertex* v3, u32 texKey)
{
  if (*globalRenderData.indexCount >= RENDERER_MAX_INDICES - 6 ||
      *globalRenderData.texturesToBindCount >= 16)
    {
      rendererEndScene();
      rendererBeginScene();
    }
  u32 texUnit = getTextureUnitByKey(texKey);
  v0->texUnit = texUnit;
  v1->texUnit = texUnit;
  v2->texUnit = texUnit;
  v3->texUnit = texUnit;
  *(globalRenderData.vertexBufferPtr  + 0) = *v0;
  *(globalRenderData.vertexBufferPtr  + 1) = *v1;
  *(globalRenderData.vertexBufferPtr  + 2) = *v2;
  *(globalRenderData.vertexBufferPtr  + 3) = *v3;
  globalRenderData.vertexBufferPtr += 4; 
  globalRenderData.indexBufferBase[(*globalRenderData.indexCount)++] = *globalRenderData.indexOffset + 0;
  globalRenderData.indexBufferBase[(*globalRenderData.indexCount)++] = *globalRenderData.indexOffset + 1;
  globalRenderData.indexBufferBase[(*globalRenderData.indexCount)++] = *globalRenderData.indexOffset + 2;
  globalRenderData.indexBufferBase[(*globalRenderData.indexCount)++] = *globalRenderData.indexOffset + 0;
  globalRenderData.indexBufferBase[(*globalRenderData.indexCount)++] = *globalRenderData.indexOffset + 2;
  globalRenderData.indexBufferBase[(*globalRenderData.indexCount)++] = *globalRenderData.indexOffset + 3;
  *globalRenderData.indexOffset += 4;
}
void addScreenSpaceQuad(Vertex* v0, Vertex* v1, Vertex* v2, Vertex* v3)  
{
  addScreenSpaceQuad(v0, v1, v2, v3, defaultTexture);
}
void addScreenSpaceQuad(vec2 center, vec2 scale)
{
    Vertex a = {
    {center.x - scale.x / 2,center.y - scale.y / 2,0.0},
    {0.0,1.0,0.0},
    {0.0, 0.0}
  };
  Vertex b = {
    {center.x - scale.x / 2,center.y + scale.y / 2,0.0},
    {0.0,1.0,0.0},
    {0.0, 1.0}
  };
  Vertex c = {
    {center.x + scale.x / 2,center.y + scale.y / 2,0.0},
    {0.0,1.0,0.0},
    {1.0, 1.0}
  };
  Vertex d = {
    {center.x + scale.x / 2,center.y - scale.y / 2,0},
    {0.0,1.0,0.0},
    {1.0, 0.0}
  };
  addScreenSpaceQuad(&a, &b, &c, &d, defaultTexture);
}


void addWorldSpaceQuad(Vertex* v0, Vertex* v1, Vertex* v2, Vertex* v3, u32 texKey)
{
  mat4 matrix = mainCamera.projectionMatrix * mainCamera.viewMatrix;
  vec4 a = {v0->pos, 1.0};
  a = matrix * a;
  a /= a.w;
  v0->pos = a.xyz;
  vec4 b = {v1->pos, 1.0};
  b = matrix * b;
  b /= b.w;
  v1->pos = b.xyz;
  vec4 c = {v2->pos, 1.0};
  c = matrix * c;
  c /= c.w;
  v2->pos = c.xyz;
  vec4 d = {v3->pos, 1.0};
  d = matrix * d;
  d /= d.w;
  v3->pos = d.xyz;
  addScreenSpaceQuad(v0,v1,v2,v3,texKey);
}
void addWorldSpaceQuad(Vertex* v0, Vertex* v1, Vertex* v2, Vertex* v3)
{
  addWorldSpaceQuad(v0,v1,v2,v3,defaultTexture);
}


void addWorldSpaceTriangle(Vertex* v0, Vertex* v1, Vertex* v2)
{
  mat4 matrix = mainCamera.projectionMatrix * mainCamera.viewMatrix;
  vec4 a = {v0->pos, 1.0};
  a = matrix * a;
  a /= a.w;
  v0->pos = a.xyz;
  vec4 b = {v1->pos, 1.0};
  b = matrix * b;
  b /= b.w;
  v1->pos = b.xyz;
  vec4 c = {v2->pos, 1.0};
  c = matrix * c;
  c /= c.w;
  v2->pos = c.xyz;
  addScreenSpaceTriangle(v0,v1,v2);
}

