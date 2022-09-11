#ifndef RENDERER_DEBUG_GEOMETRY_HEADER
#define RENDERER_DEBUG_GEOMETRY_HEADER

void initDebugGeometryMesh();

void deleteDebugGeometryWithID(u32 id);
//id is used for indefinitely lasting lines so you can delete them later
u32 _addDebugLine(vec3 min, vec3 max, vec3 colour, f32 lifeTime, u32 id);
u32 addDebugLine(vec3 min, vec3 max, vec3 colour, f32 lifeTime);
u32 addDebugLine(vec3 min, vec3 max, f32 lifeTime);
u32 addDebugLineBox(vec3 min, vec3 max, vec3 colour, f32 lifeTime);
u32 addDebugLineBox(vec3 min, vec3 max, f32 lifeTime);
void initDebugLineBuffer();
void drawDebugGeometry();
void _updateDebugLineBuffers();
void _drawDebugRect(vec3 c0, vec3 c1, vec3 c2, vec3 c3, vec3 colour);
void _drawDebugLine(vec3 min, vec3 max, vec3 colour);


#endif
