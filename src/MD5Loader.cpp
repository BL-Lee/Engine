/*
MD5Version <int:version>
commandline <string:commandline>
 
numJoints <int:numJoints>
numMeshes <int:numMeshes>
 
joints {
<string:name> <int:parentIndex> ( <vec3:position> ) ( <vec3:orientation> )
...
}
 
mesh {
shader <string:texture>
 
numverts <int:numVerts>
vert <int:vertexIndex> ( <vec2:texCoords> ) <int:startWeight> <int:weightCount>
...
 
numtris <int:numTriangles>
tri <int:triangleIndex> <int:vertIndex0> <int:vertIndex1> <int:vertIndex2>
...
 
numweights <int:numWeights>
weight <int:weightIndex> <int:jointIndex> <float:weightBias> ( <vec3:weightPosition> )
...
 
}
...
*/

/*

  T_w is the placement of the bone in world space. Origin of world to where you want the bone to be
  T_r is the rest position of the bone. Origin of modelling space to that position of the bone.
  T_r-1 is inverse of T_r. Puts vertices relative to that bone to relative to model origin
  T_c is composite matrix. T_w * T_r-1. This puts vertices relative to bone, to relative to model, then into world space
  T_c * Point would put a vertex relative to a bone into world space
  w_0 is the first weight for this vertex
  NewPoint = w_0 * T_c0 * Point + w_1 * T_c1 * Point + ... would be the full mesh skinning


 */

vec3 rotateByOrientation(vec3 v, hmm_quaternion q)
{

  vec4 convert = {v.x, v.y, v.z, 0};
  hmm_quaternion mid = QuaternionV4(convert);
  hmm_quaternion conjugate = InverseQuaternion(q);
  hmm_quaternion r = q * mid * conjugate;

  vec3 result = {r.X, r.Y, r.Z};
  return result;
  
}
void printMat4(mat4 in)
{
  for (int i = 0; i < 4; i++)
    {
      for (int j = 0; j < 4; j ++)
	{
	  printf("%f ", in[i][j]);
	}
      printf("\n");
    }
}


hmm_quaternion calculateWQuaternion(hmm_quaternion in)
{
  hmm_quaternion q = in;
  float t = 1.0f - (q.X * q.X) - (q.Y * q.Y) - (q.Z * q.Z);

  if (t < 0.0f)
    {
      q.W = 0.0f;
    }
  else
    {
      q.W = -sqrt (t);
    }
  return q;
}
SkinnedMesh* loadMD5Mesh(const char* fileName)
{
  FILE* fileHandle = fopen(fileName, "r");
  if (!fileHandle)
    {
      printf("WARNING: CANNOT LOAD MD5Mesh FILE: %s\n", fileName);
      return NULL;
    }

  SkinnedMesh* sm = (SkinnedMesh*)malloc(sizeof(SkinnedMesh));
  sm->meshCount = -1;
  sm->jointCount = -1;

  int meshIndex = 0; //mesh index
  
  char buffer[512];
  while (fgets(buffer, 512, fileHandle))
    {
      if (strstr(buffer, "numJoints")){
	sscanf(buffer,"numJoints %d", &sm->jointCount);
	sm->joints = (SkinnedJoint*)malloc(sizeof(SkinnedJoint) * sm->jointCount);
	sm->restInverses = (mat4*)malloc(sizeof(mat4) * sm->jointCount);
      }
      else if (strstr(buffer, "numMeshes")) {
	sscanf(buffer,"numMeshes %d", &sm->meshCount);
	sm->meshes = (Mesh**)malloc(sizeof(Mesh*) * sm->meshCount);
	//TEMP
	sm->tempPositions = (vec3**)malloc(sizeof(vec3*) * sm->meshCount);
	sm->weights = (SkinnedWeight**)malloc(sizeof(SkinnedWeight*) * sm->meshCount);
	sm->weightCounts = (u32*)malloc(sizeof(u32) * sm->meshCount);
	sm->vertexWeights = (SkinnedVertexWeight**)malloc(sizeof(SkinnedVertexWeight*) * sm->meshCount);
      }
      else if (strstr(buffer, "joints")) {
	Assert(sm->jointCount != -1);
	for (int j = 0; j < sm->jointCount; j++)
	  {
	    Assert(fgets(buffer, 512, fileHandle));
	    char nameBuffer[256];
	    SkinnedJoint* thisJoint = &sm->joints[j];
	    sscanf(buffer, "%s %d ( %f %f %f ) ( %f %f %f )",
		   nameBuffer,
		   &thisJoint->parentIndex,
		   &thisJoint->position.x, &thisJoint->position.y, &thisJoint->position.z,
		   &thisJoint->orientation.X, &thisJoint->orientation.Y, &thisJoint->orientation.Z);

	    thisJoint->orientation = calculateWQuaternion(thisJoint->orientation);
	    
	    thisJoint->transform = Translate(thisJoint->position) *
	      QuaternionToMat4(thisJoint->orientation);
	    
	    vec3 scale, pos;
	    hmm_quaternion orientation;
	    decomposeTransform(thisJoint->transform,
			       &pos, &scale, &orientation);	    
	    sm->restInverses[j] = Transpose(QuaternionToMat4(thisJoint->orientation)) *
	      Translate(-thisJoint->position);
	    
	    thisJoint->name = (char*)malloc(strlen(nameBuffer) + 1);
	    strcpy(thisJoint->name, nameBuffer);
	  }
	Assert(fgets(buffer, 512, fileHandle));
      }
      else if (strstr(buffer, "mesh")) {
	Assert(sm->meshCount != -1);
	Assert(buffer[0] != '\r');
	  {
	    sm->meshes[meshIndex] = (Mesh*)malloc(sizeof(Mesh));	    
	    Assert(fgets(buffer, 512, fileHandle));//shader line
	    char nameBuffer[256];
	    sscanf(buffer, "shader %s", nameBuffer);
	    
	    sm->meshes[meshIndex]->shaderName = (char*)malloc(strlen(nameBuffer) + 1);
	    strcpy(sm->meshes[meshIndex]->shaderName, nameBuffer);

	    Assert(fgets(buffer, 512, fileHandle));//blank line
	    Assert(fgets(buffer, 512, fileHandle));//numverts
	    sscanf(buffer, "\tnumverts %d", &sm->meshes[meshIndex]->vertexCount);
	    sm->meshes[meshIndex]->vertices = (Vertex*)malloc(sizeof(Vertex) * sm->meshes[meshIndex]->vertexCount);
	    //TEMP
	    sm->vertexWeights[meshIndex] = (SkinnedVertexWeight*)malloc(sizeof(SkinnedVertexWeight) * sm->meshes[meshIndex]->vertexCount);
	    	    //Vertices
	    for (int v = 0; v < sm->meshes[meshIndex]->vertexCount; v++)
	      {
		Assert(fgets(buffer, 512, fileHandle));//vertex
		int index;
		SkinnedVertexWeight sw;
		vec2 uv;
		sscanf(buffer, "\tvert %d ( %f %f ) %d %d",
		       &index,
		       &uv.x, &uv.y,
		       &sw.startIndex, &sw.count);

		sm->meshes[meshIndex]->vertices[index].texCoord = uv;
		sm->vertexWeights[meshIndex][index] = sw;		       
	      }
	    
	    Assert(fgets(buffer, 512, fileHandle));//Blank line

	    
	    //Triangles
	    Assert(fgets(buffer, 512, fileHandle));
	    int triCount;
	    sscanf(buffer, "\tnumtris %d", &triCount);
	    sm->meshes[meshIndex]->rendererData.indexCount = triCount * 3;
	    sm->meshes[meshIndex]->indices = (u32*)malloc(sizeof(u32) * triCount * 3);
	    
	    for (int v = 0; v < triCount; v++)
	      {
		Assert(fgets(buffer, 512, fileHandle));//vertex
		int index;
		int indices[3];
		sscanf(buffer, "\ttri %d %d %d %d",
		       &index,
		       &indices[0], &indices[1], &indices[2]);
		sm->meshes[meshIndex]->indices[index * 3 + 0] = indices[0];
		sm->meshes[meshIndex]->indices[index * 3 + 1] = indices[1];
		sm->meshes[meshIndex]->indices[index * 3 + 2] = indices[2];		
	      }

	    Assert(fgets(buffer, 512, fileHandle));//Blank line


	    
	    //Weights
	    Assert(fgets(buffer, 512, fileHandle));
	    int weightCount;
	    sscanf(buffer, "\tnumweights %d", &sm->weightCounts[meshIndex]);
	    sm->weights[meshIndex] = (SkinnedWeight*)malloc(sizeof(SkinnedWeight) * sm->weightCounts[meshIndex]);
	    vec3 tempPositions[sm->weightCounts[meshIndex]];
	    
	    for (int v = 0; v < sm->weightCounts[meshIndex]; v++)
	      {
		Assert(fgets(buffer, 512, fileHandle));//vertex
		int index;
		SkinnedWeight sw;		
		sscanf(buffer, "\tweight %d %d %f ( %f %f %f )",
		       &index,
		       &sw.jointIndex,
		       &sw.bias,
		       &tempPositions[v].x, &tempPositions[v].y, &tempPositions[v].z); 
		sm->weights[meshIndex][index] = sw;
	      }

	    sm->tempPositions[meshIndex] = (vec3*)malloc(sizeof(vec3) * sm->meshes[meshIndex]->vertexCount);
	    /* Calculate final vertex to draw with weights */
	    for (int j = 0; j < sm->meshes[meshIndex]->vertexCount; j++)
	      {
		vec4 finalVertex = { 0.0, 0.0, 0.0 };

		SkinnedVertexWeight svw = sm->vertexWeights[meshIndex][j];
		for (int i = 0; i < svw.count; i++)
		  {
		    SkinnedWeight weight = sm->weights[meshIndex][svw.startIndex + i];
		    SkinnedJoint joint = sm->joints[weight.jointIndex];
		    
		    vec4 zero = {tempPositions[svw.startIndex + i],1.0};
		    finalVertex += joint.transform * zero * weight.bias;
		  }
		sm->meshes[meshIndex]->vertices[j].pos = finalVertex.xyz;
		sm->meshes[meshIndex]->visible = 1;
		sm->tempPositions[meshIndex][j] = finalVertex.xyz;
	      }	  
	    meshIndex++;
	  }
      }
    }

  fclose(fileHandle);

  return sm;

}

void updateVertexPositionsManually(SkinnedMesh* skinnedMesh, int frameNumber)
{
  SkinnedAnimationFrame* frame = &skinnedMesh->animations->frames[frameNumber];
  for (int i = 0; i < skinnedMesh->meshCount; i++)
    {
      Mesh* mesh = skinnedMesh->meshes[i];
      for (int v = 0; v < mesh->vertexCount; v++)
	{
	  vec4 finalVertex = { 0.0, 0.0, 0.0 };

	  SkinnedVertexWeight svw = skinnedMesh->vertexWeights[i][v];
	  for (int j = 0; j < svw.count; j++)
	    {
	      SkinnedWeight weight = skinnedMesh->weights[i][svw.startIndex + j];
	      SkinnedJoint joint = frame->joints[weight.jointIndex];

	      vec4 zero = {skinnedMesh->tempPositions[i][v],1.0};
	      finalVertex += joint.transform * skinnedMesh->restInverses[weight.jointIndex] * zero * weight.bias;
	    }
	  mesh->vertices[v].pos = finalVertex.xyz;
	  
	}
      glBindBuffer(GL_ARRAY_BUFFER, mesh->rendererData.vertexBufferKey);
      glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * mesh->vertexCount, mesh->vertices, GL_DYNAMIC_DRAW);
    }
}

SkinnedAnimation* loadMD5Anim(const char* fileName, SkinnedMesh* associatedMesh)
{
  FILE* fileHandle = fopen(fileName, "r");
  if (!fileHandle)
    {
      printf("WARNING: CANNOT LOAD MD5Anim FILE: %s\n", fileName);
      return NULL;
    }
  
  SkinnedAnimation* animation = (SkinnedAnimation*)malloc(sizeof(SkinnedAnimation));
  u32* jointFlags;
  u32* startIndices;
  u32 numAnimatedComponents;
  int framesAdded = 0;
  char buffer[512];
  while (fgets(buffer, 512, fileHandle))
    {
      if (strstr(buffer, "numFrames"))
	{
	  sscanf(buffer,"numFrames %d", &animation->frameCount);
	  animation->frames = (SkinnedAnimationFrame*)malloc(sizeof(SkinnedAnimationFrame) * animation->frameCount);
	}

      else if (strstr(buffer, "numJoints"))
	{
	  sscanf(buffer,"numJoints %d", &animation->jointCount);
	  jointFlags = (u32*)malloc(sizeof(u32) * animation->jointCount);
	  startIndices = (u32*) malloc(sizeof(u32) * animation->jointCount);
	}
      else if (strstr(buffer, "frameRate"))
	{
	  sscanf(buffer,"frameRate %d", &animation->frameRate);
	}
      else if (strstr(buffer, "numAnimatedComponents"))
	{
	  sscanf(buffer,"numAnimatedComponents %d", &numAnimatedComponents);
	}
      //TODO: heirarchy building
      else if (strstr(buffer, "hierarchy"))
	{
	  for (int i = 0; i < animation->jointCount; i++)
	    {
	      char nameBuffer[512];
	      int parent;
	      int flags;
	      int startIndex;
	      int jointIndex = -1;
	      fgets(buffer, 512, fileHandle);
	      sscanf(buffer, "%s %d %d %d", nameBuffer, &parent, &flags, &startIndex);
	      jointFlags[i] = flags;
	      startIndices[i] = startIndex;
	    }
	}
      else if (strstr(buffer, "bounds"))
	{
	  for (int i = 0; i < animation->frameCount; i++)
	    {
	      fgets(buffer, 512, fileHandle);
	      sscanf(buffer, "\t( %f %f %f ) ( %f %f %f )",
		     &animation->frames[i].mins.x,
		     &animation->frames[i].mins.y,
		     &animation->frames[i].mins.y,
		     &animation->frames[i].maxs.x,
		     &animation->frames[i].maxs.y,
		     &animation->frames[i].maxs.z);	      		
	    }
	}
      else if (strstr(buffer, "baseframe"))
	{
	  animation->baseFrame.joints = (SkinnedJoint*)malloc(sizeof(SkinnedJoint) * animation->jointCount);
	  //NOTE: this has the same pointer to the name as the mesh.
	  memcpy(animation->baseFrame.joints, associatedMesh->joints, sizeof(SkinnedJoint) * associatedMesh->jointCount);
	  for (int i = 0; i < animation->jointCount; i++)
	    {
	      fgets(buffer, 512, fileHandle);
	      vec3 position;
	      hmm_quaternion orientation;
	      sscanf(buffer, "\t( %f %f %f ) ( %f %f %f )",
		     &position.x, &position.y, &position.z,
		     &orientation.X, &orientation.Y, &orientation.Z);

	      orientation = calculateWQuaternion(orientation);

	      SkinnedJoint* thisJoint = &animation->baseFrame.joints[i];
	      thisJoint->transform = Translate(position) * QuaternionToMat4(orientation);
	      thisJoint->position = position;
	      thisJoint->orientation = orientation;
	    }	  
	}
      else if (strstr(buffer, "frame"))
	{
	  f32 values[numAnimatedComponents];
	  animation->frames[framesAdded].joints = (SkinnedJoint*)malloc(sizeof(SkinnedJoint) * animation->jointCount);

	  //NOTE: has same pointer to name
	  memcpy(animation->frames[framesAdded].joints, animation->baseFrame.joints, sizeof(SkinnedJoint) * animation->jointCount);	  
		 
	  //Seems to be divided in lines of 6 values?
	  for (int i = 0; i < numAnimatedComponents; i+=6)
	    {
	      fgets(buffer, 512, fileHandle);
	      sscanf(buffer, "\t%f %f %f %f %f %f",
		     &values[i + 0], &values[i + 1], &values[i + 2],
		     &values[i + 3], &values[i + 4], &values[i + 5]
		     );
	    }
	  //calculate baseframe for this frame
	  for (int i = 0; i < animation->jointCount; i++)
	    {
	      u32 flags = jointFlags[i];
	      u32 j = 0;
	      vec3 updatedPos = animation->baseFrame.joints[i].position;
	      hmm_quaternion updatedOrientation = animation->baseFrame.joints[i].orientation;;
	      if (flags & 0x1)
		{
		  updatedPos.x = values[startIndices[i] + j];
		  j++;
		}
	      if (flags & 0x2)
		{
		  updatedPos.y = values[startIndices[i] + j];
		  j++;
		}
	      if (flags & 0x4)
		{
		  updatedPos.z = values[startIndices[i] + j];
		  j++;
		}
	      if (flags & 0x8)
		{
		  updatedOrientation.x = values[startIndices[i] + j];
		  j++;
		}
	      if (flags & 0x10)
		{
		  updatedOrientation.y = values[startIndices[i] + j];
		  j++;
		}
	      if (flags & 0x20)
		{
		  updatedOrientation.z = values[startIndices[i] + j];
		  j++;
		}
	      updatedOrientation = calculateWQuaternion(updatedOrientation);	     
	      SkinnedJoint* thisJoint = &animation->frames[framesAdded].joints[i];
	      
	      //Has a parent?
	      if (thisJoint->parentIndex >= 0)
		{
		  SkinnedJoint parentJoint = animation->frames[framesAdded].joints[thisJoint->parentIndex];
		  thisJoint->transform = parentJoint.transform * Translate(updatedPos) * QuaternionToMat4(updatedOrientation);
		}
	      vec3 scale;
	      decomposeTransform(thisJoint->transform, &thisJoint->position, &scale, &thisJoint->orientation);
	    }
	  framesAdded++;
	}
    }
  associatedMesh->animations = animation;
  free(jointFlags);
  free(startIndices);
  fclose(fileHandle);
  return animation;
}
