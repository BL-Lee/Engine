
void addDebugGizmo(vec3 location)
{
  Entity* dodec = deserializeEntity("res/entities/LightGizmo.entity");
  dodec->position = location;
}

void debugTranslateEntity()
{
  //X axis
  Entity* e = getEntityById(globalDebugData.selectedEntityId);
  vec2 mouseCoords = pollCursorPos();
  Ray ray = rayFromScreenPoint(mouseCoords);

  if (globalDebugData.selectedEntityId == globalDebugData.translationArrowIds[0])
    {
      Plane intersectionPlane = {
	{0.0, 0.0, 1.0}, //normal
	e->parent->position.z //Dist
      };
      vec3 hitLocation;
      
      if (rayPlaneCollision(&ray, &intersectionPlane, &hitLocation))
	{
	  //addDebugLineBox({0.0,0.0,e->parent->position.z}, hitLocation);
	  e->parent->position.x = hitLocation.x;
	}
    }
  else if (globalDebugData.selectedEntityId == globalDebugData.translationArrowIds[1])
    {
      Plane intersectionPlane = {
	{0.0, 0.0, 1.0}, //normal
	e->parent->position.z //Dist
      };

      vec3 hitLocation;
      if (rayPlaneCollision(&ray, &intersectionPlane, &hitLocation))
	{
	  //addDebugLineBox({0.0,0.0,e->parent->position.z}, hitLocation);
	  e->parent->position.y = hitLocation.y;
	}
    }
  else if (globalDebugData.selectedEntityId == globalDebugData.translationArrowIds[2])
    {
      Plane intersectionPlane = {
	{1.0, 0.0, 0.0}, //normal
	e->parent->position.x //Dist
      };
      vec3 hitLocation;
      if (rayPlaneCollision(&ray, &intersectionPlane, &hitLocation))
	{
	  //addDebugLineBox({0.0,0.0,e->parent->position.x}, hitLocation);
	  e->parent->position.z = hitLocation.z;
	}
    }

  
}

