

void outlineSelectedMesh()
{
  Entity* e = getEntityById(globalDebugData.selectedEntityId);
  if (e)
    {
      AABBCollider aabb = getWorldSpaceAABBCollider(e);
      addDebugLineBox(aabb.min, aabb.max);
    }
}

Mesh* selectMesh(vec2 mouseCoords)
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
      globalDebugData.selectedEntityId = entityHit->id;
    }
  return NULL;
}

