#include "Mesh.h"
#include "EntityRegistry.h"
/*************************************

Ray collisions

**************************************/
u32 rayPlaneCollision(Ray* ray, Plane* plane, f32* dist, vec3* location)
{
  f32 rayTolerance = 0.0001;
  f32 minHitDistance = 0.0001;
  
  f32 denom = Dot(plane->normal, ray->direction);
  u32 toleranceMask = (denom > rayTolerance) | (denom < -rayTolerance);

  f32 pDist = (-plane->dist - Dot(plane->normal, ray->origin)) / denom;
  u32 distMask = (pDist > minHitDistance);
  if (location)
    *location = ray->origin + pDist * ray->direction;
  if (dist)
    *dist = pDist;
  return toleranceMask & distMask; 
}
u32 rayCylinderCollision(Ray* ray, Cylinder* cyl, f32* outDist, vec3* location)
{
  vec3 perpDir = Cross(ray->direction, cyl->direction);

  vec3 n2 = Cross(cyl->direction, perpDir);
  vec3 c1 = ray->origin + ray->direction * (Dot(cyl->origin - ray->origin, n2) / Dot(ray->direction, n2));

  vec3 n1 = Cross(ray->direction, perpDir);
  vec3 c2 = cyl->origin + cyl->direction * (Dot(ray->origin - cyl->origin, n1) / Dot(cyl->direction, n1));

  f32 dist = Length(c2 - c1);
  vec3 rayOriginToPoint = c1 - ray->origin;
  vec3 cylOriginToPoint = c2 - cyl->origin;

  u32 distMask = dist <  (cyl->width);
		
  u32 minDistMask = Dot(ray->direction, rayOriginToPoint) > (0.0f);
  u32 LengthMask = (Length(cylOriginToPoint) <  (cyl->length)) & (Dot(cyl->direction, cylOriginToPoint) >  (0.0f));
		
  u32 hitMask = distMask & minDistMask & LengthMask;

  if (location)
    *location = ray->origin + dist * ray->direction;
  if (dist)
    *outDist = dist;
  return hitMask; 
}
u32 rayTriangleCollision(Ray* ray, Triangle* triangle, f32* outDist, vec3* location)
{
  f32 rayTolerance = 0.0001;
  f32 minDist = 0.0001;
  vec3 v0 = triangle->v0;
  vec3 v1 = triangle->v1;
  vec3 v2 = triangle->v2;
  vec3 normal = Normalize(Cross(v1-v0, v2-v0));


  f32 denom = Dot(normal, ray->direction);
  u32 toleranceMask = (denom > rayTolerance) | (denom < -rayTolerance);


  f32 triangleOffset; //like the planeDist but for the triangle
  triangleOffset = -Dot(normal, v0);
  f32 triangleDist;
  triangleDist = -(Dot(normal, ray->origin) + triangleOffset) / denom; 
		
  u32 planeHitMask;
  planeHitMask = (triangleDist > minDist);
  if (planeHitMask)
    {
      u32 triangleHitMask;
      triangleHitMask = 0x1;
		  
      vec3 planePoint;
      planePoint = (ray->direction * triangleDist) + ray->origin;

      vec3 edgePerp;
		  
      vec3 edge0 = v1 - v0;
      edgePerp = Cross(edge0, planePoint - v0);
      triangleHitMask &= Dot(normal, edgePerp) > 0.0f;

      vec3 edge1 = v2 - v1;
      edgePerp = Cross(edge1, planePoint - v1);
      triangleHitMask &= Dot(normal, edgePerp) > 0.0f;

      vec3 edge2 = v0 - v2;
      edgePerp = Cross(edge2, planePoint - v2);
      triangleHitMask &= Dot(normal, edgePerp) > 0.0f;
      
      u32 hitMask = triangleHitMask && planeHitMask;
      if (hitMask)
	{
	  if (outDist)
	    *outDist = triangleDist;
	  if (location)
	    *location = ray->origin + ray->direction * triangleDist;
	  return hitMask;
	}
    }
  return 0;
}

u32 rayPlaneCollision(Ray* ray, Plane* plane)
{
  return rayPlaneCollision(ray, plane, NULL, NULL);
}
u32 rayPlaneCollision(Ray* ray, Plane* plane, f32* dist)
{
  return rayPlaneCollision(ray, plane, dist, NULL);
}
u32 rayPlaneCollision(Ray* ray, Plane* plane, vec3* location)
{
  return rayPlaneCollision(ray, plane, NULL, location);
}
u32 rayCylinderCollision(Ray* ray, Cylinder* cyl)
{
  return rayCylinderCollision(ray, cyl, NULL, NULL);
}
u32 rayCylinderCollision(Ray* ray, Cylinder* cyl, f32* dist)
{
  return rayCylinderCollision(ray, cyl, dist, NULL);
}
u32 rayCylinderCollision(Ray* ray, Cylinder* cyl, vec3* location)
{
  return rayCylinderCollision(ray, cyl, NULL, location);
}
u32 rayTriangleCollision(Ray* ray, Triangle* tri)
{
  return rayTriangleCollision(ray, tri, NULL, NULL);
}
u32 rayTriangleCollision(Ray* ray, Triangle* tri, f32* dist)
{
  return rayTriangleCollision(ray, tri, dist, NULL);
}
u32 rayTriangleCollision(Ray* ray, Triangle* tri, vec3* location)
{
  return rayTriangleCollision(ray, tri, NULL, location);
}


u32 rayMeshCollisionNaive(Ray* ray, Mesh* mesh, mat4* transMatrix, f32* hit, vec3* loc)
{
  //CAUTION: DOES NTO WORK WITH SKINNED MESHES YET
  u32 hasHit = 0;
  for (int index = 0; index < mesh->rendererData.indexCount; index+=3)
    {
      //THESE ARE NOT IN WORLD SPACE YET
      vec3 positions[3];
      Vertex* vertices = (Vertex*)mesh->vertices;
      if (mesh->indices)
	{
	  positions[0] =  vertices[mesh->indices[index + 0]].pos;
	  positions[1] =  vertices[mesh->indices[index + 1]].pos;
	  positions[2] =  vertices[mesh->indices[index + 2]].pos;
	}
      else
	{
	  positions[0] = vertices[index + 0].pos;
	  positions[1] = vertices[index + 1].pos;
	  positions[2] = vertices[index + 2].pos;
	}
      vec4 worldPositions[3] = 
	{
	  {positions[0].x,positions[0].y,positions[0].z,1.0},
	  {positions[1].x,positions[1].y,positions[1].z,1.0},
	  {positions[2].x,positions[2].y,positions[2].z,1.0}
	};
      for (int b = 0; b < 3; b ++)
	{
	  worldPositions[b] = *transMatrix * worldPositions[b];
	  worldPositions[b] /= worldPositions[b].w;
	}
		      
      Triangle tri;
      tri.v0 = worldPositions[0].xyz;
      tri.v1 = worldPositions[1].xyz;
      tri.v2 = worldPositions[2].xyz;

      f32 tHit = FLT_MAX;
      vec3 tLoc;
      if (rayTriangleCollision(ray, &tri, &tHit, &tLoc))
	{
	  //Bit unintuitive, but this will make it so if you pass in a minimum distance in hit
	  //It will discard any hits that are above that length
	  if (tHit < *hit && tHit > 0.0f)
	    {
	      *hit = tHit;
	      *loc = tLoc;
	      hasHit = 1;
	    }
	}
    }
  return hasHit;
}

u32 rayEntityCollisionNaive(Ray* ray, Entity* e, f32* hit, vec3* loc)
{

  u32 hasHit = 0;
  
  mat4 modelMatrix = transformationMatrixFromComponents(e->position, e->scale, e->rotation);

  for (int me = 0; me < e->meshCount; me++)
    {
      Mesh* mesh = e->meshes[me];
      f32 meshHit = FLT_MAX;
      vec3 meshLoc;
      if (rayMeshCollisionNaive(ray, mesh, &modelMatrix, &meshHit, &meshLoc))
	{
	  //Bit unintuitive, but this will make it so if you pass in a minimum distance in hit
	  //It will discard any hits that are above that length
	  if (meshHit < *hit && meshHit > 0.0f)
	    {

	      *hit = meshHit;
	      *loc = meshLoc;
	      hasHit = 1;
	    }
	}
    }
  return hasHit;
}

//Actually so fucking bad lmao, goes from 200fps to 70fps when calling 4 times a frame
u32 rayCastAllNaive(Ray* ray, f32* hit, vec3* loc, Entity** eHit)
{
  f32 hitDist = FLT_MAX;
  vec3 hitLoc;
  u32 hasHit = 0;

  for (int i = 0; i < MAX_REGISTRY_SIZE; i++)
    {
      if (globalEntityRegistry->occupiedIndices[i] && globalEntityRegistry->entities[i].meshes)
	{
	  Entity* e = globalEntityRegistry->entities + i;
	  f32 entityHit = FLT_MAX;
	  vec3 entityLoc;
	  if (e->visible)
	    {
	      if (rayEntityCollisionNaive(ray, e, &entityHit, &entityLoc))
		{
		  if (entityHit < *hit && entityHit > 0.0f)
		    {
		      *eHit = e;
		      *hit = entityHit;
		      *loc = entityLoc;
		      hasHit = 1;
		    }
		}
	    }
	}
    }
  return hasHit;
}

u32 rayCastAllNaive(Ray* ray, f32* hit, vec3* loc)
{
  Entity* e;
  return rayCastAllNaive(ray, hit, loc, &e);
}

#if 0

/*****************************

GJK

 *****************************/

//Generic support function. Gives the support value for a certain direction
vec3 GJKSupport(Mesh* A, Mesh* B, vec3 direction)
{

}

//Builds simplex. Returns false if the two objects are not colliding
#define GJK_TOWARDS_ORIGIN(A,B) (Dot(B - A, -A) > 0)
bool GJKSimplex(vec3* points, u32 pointCount, vec3* direction)
{
  switch (pointCount)
    {
    case 2:
      {
	//Line
	//A is one we just added
	vec3 A = points[1];
	vec3 B = points[0];
	points[pointCount + 1] = Cross(Cross(B - A, -A), B - A);
	return true;
      } break;
    case 3:
      {
	//Triangle
      } break;
    case 4:
      {
	//Tetrahedron
      } break;
    default:
      {
	Assert(0);
      } break;
    }
  return false;
}


//Returns true if First and Second are colliding
//This is the function to call
bool checkGJKCollision(Mesh* First, Mesh* Second)
{
  //TODO: convex the concave objects
  /*
https://doc.cgal.org/latest/Convex_decomposition_3/index.html
http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.123.5307&rep=rep1&type=pdf
https://www.cs.princeton.edu/~chazelle/pubs/OptimalConvexDecomp.pdf
   */

  Mesh* A = First;
  Mesh* B = Second;
  
  vec3 points[4];
  u32 pointsAdded = 1;
  vec3 direction = {0.0,1.0,0.0};
  points[0] = GJKSupport(A, B, direction);
  direction = -points[0];

  do
    {
      vec3 newPoint = GJKSupport(A,B, direction);
      if (Dot(newPoint, direction) < 0)
	{
	  return false;	  
	}
      points[pointsAdded] = newPoint;
      pointsAdded++;
      
    } while (GJKSimplex(points, pointsAdded, &direction));
  return false;  
}
#endif
