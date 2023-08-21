
void addDebugGizmo(vec3 location)
{
  Entity* dodec = deserializeEntity("res/entities/LightGizmo.entity");
  dodec->position = location;
}

void debugTranslateEntity(Entity* entity)
{
  if (!globalDebugData.selectedEntityId || !entity)
    {
      return;
    }
  //X axis
  Entity* e = getEntityById(globalDebugData.translationArrowId);
  vec2 mouseCoords = pollCursorPos();
  Ray ray = rayFromScreenPoint(mouseCoords);

  if (globalDebugData.arrowSelected == globalDebugData.translationArrowIds[0])
    {
      Plane intersectionPlane = {
	{0.0, 0.0, 1.0}, //normal
	e->position.z //Dist
      };
      vec3 hitLocation;
      
      if (rayPlaneCollision(&ray, &intersectionPlane, &hitLocation))
	{
	  //addDebugLineBox({0.0,0.0,e->parent->position.z}, hitLocation);
	  e->position.x = hitLocation.x;
	  entity->position.x = hitLocation.x;
	}
    }
  else if (globalDebugData.arrowSelected == globalDebugData.translationArrowIds[1])
    {
      Plane intersectionPlane = {
	{0.0, 0.0, 1.0}, //normal
	e->position.z //Dist
      };

      vec3 hitLocation;
      if (rayPlaneCollision(&ray, &intersectionPlane, &hitLocation))
	{
	  //addDebugLineBox({0.0,0.0,e->parent->position.z}, hitLocation);
	  e->position.y = hitLocation.y;
	  entity->position.y = hitLocation.y;
	}
    }
  else if (globalDebugData.arrowSelected == globalDebugData.translationArrowIds[2])
    {
      Plane intersectionPlane = {
	{1.0, 0.0, 0.0}, //normal
	e->position.x //Dist
      };
      vec3 hitLocation;
      if (rayPlaneCollision(&ray, &intersectionPlane, &hitLocation))
	{
	  //addDebugLineBox({0.0,0.0,e->parent->position.x}, hitLocation);
	  e->position.z = hitLocation.z;
	  entity->position.z = hitLocation.z;
	}
    }

  
}

