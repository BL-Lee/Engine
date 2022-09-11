#include "Renderer.h"
#include "RendererDebugGeometry.h"
#define DEBUG_LINE_TO_VERTEX_COUNT 6
void initDebugGeometryMesh()
{
  Mesh* m = (Mesh*)calloc(1, sizeof(Mesh));
  globalRenderData.meshesToDraw[0] = m;
  globalRenderData.debugGeometryMesh = globalRenderData.meshesToDraw[0];
  globalRenderData.meshesToDrawCount = 1;
  m->visible = true;
  u32 totalVertexCount = DEBUG_LINE_TO_VERTEX_COUNT * RENDERER_MAX_DEBUG_LINE_COUNT;
  m->vertices = malloc(sizeof(Vertex) * totalVertexCount);
  globalRenderData.meshTransforms[0] = {0.0,0.0,0.0};
  globalRenderData.meshTransforms[1] = {0.0,0.0,0.0};
  globalRenderData.meshTransforms[2] = {1.0,1.0,1.0};  

  //m->indices = (u32*) malloc(sizeof(u32) * totalVertexCount);

  m->vertexCount = totalVertexCount;
  addMesh(m, "res/shaders/normalVisVertex.glsl", "res/shaders/normalVisFrag.glsl");
  //addMesh(m, "res/shaders/basicLightVertex.glsl", "res/shaders/basicLightFrag.glsl");
  m->vertexCount = 0;
  m->rendererData.indexCount = 0;
}
void initDebugLineBuffer() {
  for (int i = 0; i < RENDERER_MAX_DEBUG_LINE_COUNT; i++)
    {
      globalRenderData.debugLines[i].active = false;
    }
  globalRenderData.debugLineIndex = 0;
}

void drawDebugGeometry()
{
  for (int i = 0; i < RENDERER_MAX_DEBUG_LINE_COUNT; i++)
    {
      DebugLine* line = &globalRenderData.debugLines[i];
      if (line->active)
	{

	  _drawDebugLine(line->min, line->max, line->colour);

	  line->lifeTime -= globalDeltaTime;
	  if (line->lifeTime < 0)
	    {
	      line->active = false;
	    }
	}      
    }
  _updateDebugLineBuffers();
}


void deleteDebugGeometryWithID(u32 id)
{
  for (int i = 0; i < RENDERER_MAX_DEBUG_LINE_COUNT; i++)
    {
      if (globalRenderData.debugLines[i].shapeID == id)
	{
	  globalRenderData.debugLines[i].active = false;
	}
    }
}

u32 addDebugLine(vec3 min, vec3 max, vec3 colour, f32 lifeTime)
{
  return _addDebugLine(min, max, colour, lifeTime, globalRenderData.debugLineIndex);

}
u32 addDebugLine(vec3 min, vec3 max, f32 lifeTime)
{
  vec3 colour = {0.0, 1.0, 0.0};
  return addDebugLine(min, max, colour, lifeTime);
}
u32 addDebugLineBox(vec3 min, vec3 max, vec3 colour, f32 lifeTime)
{
  vec3 corners[8];
  corners[0] = {min.x, min.y, min.z};
  corners[1] = {min.x, min.y, max.z};
  corners[2] = {min.x, max.y, min.z};
  corners[3] = {min.x, max.y, max.z};

  corners[4] = {max.x, min.y, min.z};
  corners[5] = {max.x, min.y, max.z};
  corners[6] = {max.x, max.y, min.z};
  corners[7] = {max.x, max.y, max.z};

  u32 id = globalRenderData.debugLineIndex;
  
  _addDebugLine(corners[0], corners[1], colour, lifeTime, id);
  _addDebugLine(corners[0], corners[2], colour, lifeTime, id);
  _addDebugLine(corners[3], corners[2], colour, lifeTime, id);
  _addDebugLine(corners[3], corners[1], colour, lifeTime, id);

  _addDebugLine(corners[4], corners[5], colour, lifeTime, id);
  _addDebugLine(corners[4], corners[6], colour, lifeTime, id);
  _addDebugLine(corners[7], corners[6], colour, lifeTime, id);
  _addDebugLine(corners[7], corners[5], colour, lifeTime, id);

  _addDebugLine(corners[0], corners[4], colour, lifeTime, id);
  _addDebugLine(corners[1], corners[5], colour, lifeTime, id);
  _addDebugLine(corners[2], corners[6], colour, lifeTime, id);
  _addDebugLine(corners[3], corners[7], colour, lifeTime, id);

  return id;
}

u32 addDebugLineBox(vec3 min, vec3 max, f32 lifeTime)
{
  vec3 colour = {0.0, 1.0, 0.0};
  return addDebugLineBox(min, max, colour, lifeTime);
}



void _updateDebugLineBuffers()
{
  glBindBuffer(GL_ARRAY_BUFFER, globalRenderData.debugGeometryMesh->rendererData.vertexBufferKey);
  glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * globalRenderData.debugGeometryMesh->vertexCount, globalRenderData.debugGeometryMesh->vertices, GL_DYNAMIC_DRAW);
}


void _drawDebugRect(vec3 c0, vec3 c1, vec3 c2, vec3 c3, vec3 colour)
{

  Vertex* vertices = (Vertex*)globalRenderData.debugGeometryMesh->vertices;
  Mesh* m = globalRenderData.debugGeometryMesh;
  
  //If assertion hits, run out of debug lines, try increasing the buffer in renderer.h or using less
  Assert(m->vertexCount + 6 < DEBUG_LINE_TO_VERTEX_COUNT * RENDERER_MAX_DEBUG_LINE_COUNT);
  
  //TODO: use indices
  vertices[m->vertexCount + 0].pos = c0;
  vertices[m->vertexCount + 1].pos = c1;
  vertices[m->vertexCount + 2].pos = c2;
  
  vertices[m->vertexCount + 3].pos = c0;
  vertices[m->vertexCount + 4].pos = c2;
  vertices[m->vertexCount + 5].pos = c3;

  for (int i =0 ; i< 6; i++)
    {
      vertices[m->vertexCount + i].normal = colour; //Uses normal as colour, since we're not doing shading anyways
      //m->indices[m->vertexCount + i] = m->vertexCount + i;
    }
  m->rendererData.indexCount += 6;
  m->vertexCount += 6;
}

void _drawDebugLine(vec3 min, vec3 max, vec3 colour)
{
  vec3 dir = min - max;
  vec3 orth = Normalize(Cross(dir, mainCamera.pos - max));
  vec3 corners[4];
  float t = 0.01f;
  
  corners[0] = min - orth * t;
  corners[1] = min + orth * t;
  corners[2] = max + orth * t;
  corners[3] = max - orth * t;

  _drawDebugRect(corners[0], corners[1], corners[2], corners[3], colour );
}
//id is used for indefinitely lasting lines so you can delete them later
u32 _addDebugLine(vec3 min, vec3 max, vec3 colour, f32 lifeTime, u32 id)
{
  s32 index = -1;
  for (int i = 0; i < RENDERER_MAX_DEBUG_LINE_COUNT; i++)
    {
      globalRenderData.debugLineIndex++;
      if (!globalRenderData.debugLines[
		globalRenderData.debugLineIndex % RENDERER_MAX_DEBUG_LINE_COUNT
				       ].active)
	{
	  index = globalRenderData.debugLineIndex % RENDERER_MAX_DEBUG_LINE_COUNT;
	  break;
	}

    }
  Assert(index != -1); //Too many debug lines, try increasing buffer size or using less

  DebugLine line = {true, min, max, colour, lifeTime, id};
  globalRenderData.debugLines[index] = line;
  return id;
}
