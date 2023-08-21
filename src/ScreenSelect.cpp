

void outlineSelectedMesh()
{
  Entity* e = getEntityById(globalDebugData.selectedEntityId);
  if (e)
    {
      AABBCollider aabb = getWorldSpaceAABBCollider(e);
      //      addDebugLineBox(aabb.min, aabb.max);
    }
}

Entity* screenSelectEntity(vec2 mouseCoords)
{

  f32 hitDist = FLT_MAX;
  vec3 hitLoc;
  Ray ray = rayFromScreenPoint(mouseCoords);

  Entity* entityHit = NULL;
  
  if (rayCastAllNaive(&ray, &hitDist, &hitLoc, &entityHit))
    {
      globalRenderData.pointLights[1].position = hitLoc;
      Entity* pLight = getEntityById(globalRenderData.pointLights[1].entityGizmoID);
      pLight->position = hitLoc;

      return entityHit;
    }
  return NULL;
}

vec3 screenSelectMaxDist(vec2 mouseCoords, f32 maxDist)
{
  f32 hitDist = FLT_MAX;
  vec3 hitLoc;
  Ray ray = rayFromScreenPoint(mouseCoords);

  Entity* entityHit = NULL;
  
  if (rayCastAllNaive(&ray, &hitDist, &hitLoc, &entityHit))
    {
      return entityHit->position;
    }
  return ray.origin + ray.direction * maxDist;
}
