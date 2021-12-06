#include "Mesh.h"

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
