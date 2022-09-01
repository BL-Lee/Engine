#ifndef __COLLISION_HEADER
#define __COLLISION_HEADER

struct Ray
{
  vec3 origin;
  vec3 direction;
};
struct Plane
{
  vec3 normal;
  f32 dist;
};
struct Cylinder
{
  vec3 origin;
  vec3 direction;
  f32 length;
  f32 width;  
};
struct Triangle
{
  vec3 v0, v1, v2;
  vec3 normal;
};
struct AABBCollider
{
  //vec3 center;
  //vec3 size; //each component is "radius"
  vec3 min, max;
};
struct Collider
{
  u32 type;
  AABBCollider aabb;
  Cylinder cylCollider;
};

#endif
