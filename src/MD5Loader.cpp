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
weight <int:weightIndex> <int:jointIndex> <float:weightBias> ( <vec3:weightPosition> %)
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

#if DEBUG_IMPORT
#define printBuffer() {printf("%s", buffer);fflush(stdout);}
#else
#define printBuffer()
#endif
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
  
  SkinnedWeight** tempWeights;
  u32* weightCounts;
  SkinnedVertexWeight** vertexWeights;
  
  char buffer[512];
  
  while (fgets(buffer, 512, fileHandle))
    {
      printBuffer();
      if (strstr(buffer, "numJoints")){
	sscanf(buffer,"numJoints %d", &sm->jointCount);
	sm->joints = (SkinnedJoint*)malloc(sizeof(SkinnedJoint) * sm->jointCount);
	sm->restInverses = (mat4*)malloc(sizeof(mat4) * sm->jointCount);
      }
      else if (strstr(buffer, "numMeshes")) {
	sscanf(buffer,"numMeshes %d", &sm->meshCount);
	sm->meshes = (Mesh**)malloc(sizeof(Mesh*) * sm->meshCount);
	
	//Temporary variables. TODO: get rid of tempPositions
	sm->tempPositions = (vec3**)malloc(sizeof(vec3*) * sm->meshCount);
	tempWeights = (SkinnedWeight**)malloc(sizeof(SkinnedWeight*) * sm->meshCount);
	weightCounts = (u32*)malloc(sizeof(u32) * sm->meshCount);
	vertexWeights = (SkinnedVertexWeight**)malloc(sizeof(SkinnedVertexWeight*) * sm->meshCount);
      }
      else if (strstr(buffer, "joints")) {
	Assert(sm->jointCount != -1);
	for (int j = 0; j < sm->jointCount; j++)
	  {
	    Assert(fgets(buffer, 512, fileHandle));
	    printBuffer();
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
	    sm->meshes[meshIndex] = (Mesh*)calloc(1, sizeof(Mesh));
	    sm->meshes[meshIndex]->visible = 1;
	    Assert(fgets(buffer, 512, fileHandle));//shader line
	    printBuffer();
	    char nameBuffer[256];
	    filterBlankLinesUntil(fileHandle, buffer, 512, "shader");
	    sscanf(buffer, "shader %s", nameBuffer);
	    
	    sm->meshes[meshIndex]->shaderName = (char*)malloc(strlen(nameBuffer) + 1);
	    strcpy(sm->meshes[meshIndex]->shaderName, nameBuffer);

	    filterBlankLinesUntil(fileHandle, buffer, 512, "numverts");
	    sscanf(buffer, "\tnumverts %d", &sm->meshes[meshIndex]->vertexCount);
	    sm->meshes[meshIndex]->vertices = calloc(sm->meshes[meshIndex]->vertexCount, sizeof(SkinnedVertex));
	    
	    //TEMP
	    vertexWeights[meshIndex] = (SkinnedVertexWeight*)malloc(sizeof(SkinnedVertexWeight) * sm->meshes[meshIndex]->vertexCount);
	    	    //SkinnedVertices
	    for (int v = 0; v < sm->meshes[meshIndex]->vertexCount; v++)
	      {
		Assert(fgets(buffer, 512, fileHandle));//vertex
		printBuffer();
		int index;
		SkinnedVertexWeight sw;
		vec2 uv;
		sscanf(buffer, "\tvert %d ( %f %f ) %d %d",
		       &index,
		       &uv.x, &uv.y,
		       &sw.startIndex, &sw.count);

		((SkinnedVertex*)sm->meshes[meshIndex]->vertices)[index].texCoord = uv;
		vertexWeights[meshIndex][index] = sw;		       
	      }
	    
	    //Triangles
	    int triCount;
	    filterBlankLinesUntil(fileHandle, buffer, 512, "numtris");
	    sscanf(buffer, "\tnumtris %d", &triCount);
	    sm->meshes[meshIndex]->rendererData.indexCount = triCount * 3;
	    sm->meshes[meshIndex]->indices = (u32*)malloc(sizeof(u32) * triCount * 3);
	    
	    for (int v = 0; v < triCount; v++)
	      {
		Assert(fgets(buffer, 512, fileHandle));//vertex
		      printBuffer();
		int index;
		int indices[3];
		sscanf(buffer, "\ttri %d %d %d %d",
		       &index,
		       &indices[0], &indices[1], &indices[2]);
		sm->meshes[meshIndex]->indices[index * 3 + 0] = indices[0];
		sm->meshes[meshIndex]->indices[index * 3 + 1] = indices[1];
		sm->meshes[meshIndex]->indices[index * 3 + 2] = indices[2];		
	      }

	    //Weights
	          printBuffer();
	    int weightCount;
	    filterBlankLinesUntil(fileHandle, buffer, 512, "numweights");
	    sscanf(buffer, "\tnumweights %d", &weightCounts[meshIndex]);
	    tempWeights[meshIndex] = (SkinnedWeight*)calloc(weightCounts[meshIndex],sizeof(SkinnedWeight));
	    Assert(weightCounts[meshIndex] < 5000);
	    vec3 tempPositions[weightCounts[meshIndex]];
	    
	    for (int v = 0; v < weightCounts[meshIndex]; v++)
	      {
		Assert(fgets(buffer, 512, fileHandle));//vertex
		      printBuffer();
		int index;
		SkinnedWeight sw;		
		sscanf(buffer, "\tweight %d %d %f ( %f %f %f )",
		       &index,
		       &sw.jointIndex,
		       &sw.bias,
		       &tempPositions[v].x, &tempPositions[v].y, &tempPositions[v].z); 
		tempWeights[meshIndex][index] = sw;
	      }

	    sm->tempPositions[meshIndex] = (vec3*)malloc(sizeof(vec3) * sm->meshes[meshIndex]->vertexCount);
	
	    for (int j = 0; j < sm->meshes[meshIndex]->vertexCount; j++)
	      {
		vec4 finalVertex = { 0.0, 0.0, 0.0 };
		
		float remainingWeight = 0.0;
		SkinnedVertex* vertex = &((SkinnedVertex*)sm->meshes[meshIndex]->vertices)[j];
		SkinnedVertexWeight svw = vertexWeights[meshIndex][j];
		SkinnedWeight weights[svw.count];
		for (int i = 0; i < svw.count; i++)
		  {
		    weights[i] = tempWeights[meshIndex][svw.startIndex + i];
		  }
		//TODO: sort helper function
		for (int i = 0; i < svw.count - 1; i++)
		  {
		    for (int k = 0; k < svw.count - i - 1; k++)
		      {
			if (weights[k].bias < weights[k + 1].bias)
			  {
			    SkinnedWeight temp = weights[k+1];
			    weights[k + 1] = weights[k];
			    weights[k] = temp;

			  }
 		      }
		  }
				
		for (int i = 0; i < svw.count; i++)
		  {
		    SkinnedWeight weight = weights[i];
		    SkinnedJoint joint = sm->joints[weight.jointIndex];

		    if (i < 4)
		      {
			vertex->jointWeights[i] = weight.bias;
			vertex->jointIndices[i] = (float)weight.jointIndex;
		      }
		    else
		      {
			remainingWeight += weight.bias;
		      }
		    
		    vec4 zero = {tempPositions[svw.startIndex + i],1.0};
		    finalVertex += joint.transform * zero * weight.bias;
		    
		  }
		for (int i = 0; i < 4; i++)
		  {
		    vertex->jointWeights[i] += remainingWeight * vertex->jointWeights[i];
		  }
		for (int i = 1; i < 4; i ++)
		  {		    
		    Assert(vertex->jointWeights[i] <= vertex->jointWeights[i-1]);
		  }				
		((SkinnedVertex*)sm->meshes[meshIndex]->vertices)[j].pos = finalVertex.xyz;
		sm->meshes[meshIndex]->visible = 1;
		sm->tempPositions[meshIndex][j] = finalVertex.xyz;
		sm->meshes[meshIndex]->skinnedMesh = sm;
	      }	  
	    meshIndex++;
	  }
      }
    }
  for (int i = 0; i < sm->meshCount; i++)
    {
      free(tempWeights[i]);
      free(vertexWeights[i]);
    }
  free(tempWeights);
  free(weightCounts);
  free(vertexWeights);
  fclose(fileHandle);

  return sm;

}



//See note in Mesh.h about transform/joint storage
void updateVertexPositionsManually(SkinnedMesh* skinnedMesh)
{
  SkinnedAnimation* anim = skinnedMesh->animations;
  for (int i = 0; i < skinnedMesh->meshCount; i++)
    {
      Mesh* mesh = skinnedMesh->meshes[i];
      for (int v = 0; v < mesh->vertexCount; v++)
	{
	  SkinnedVertex* vertex = &((SkinnedVertex*)mesh->vertices)[v];
	  vec4 finalVertex = { 0.0, 0.0, 0.0, 0.0 };

	  for (int j = 0; j < 4; j++)
	    {
	      vec4 zero = { skinnedMesh->tempPositions[i][v], 1.0 };
	      vec4 newPos = (anim->compositeMatrices[(int)vertex->jointIndices[j]] * zero) * vertex->jointWeights[j];
	      finalVertex += newPos;
	      }
	  //vec3 oldPos = ((SkinnedVertex*)mesh->vertices)[v].pos;
	  //vec3 finalVert = finalVertex.xyz / finalVertex.w;
	  //vec3 diff = oldPos - finalVert;
	  //printf("oldPos: %f %f %f\n", oldPos.x, oldPos.y, oldPos.z);
	  //printf("finalVert: %f %f %f\n", finalVert.x, finalVert.y, finalVert.z );
	  //printf("DIFF: %f %f %f\n", diff.x, diff.y, diff.z);
	  ((SkinnedVertex*)mesh->vertices)[v].pos = finalVertex.xyz / finalVertex.w;
	  
	}
      glBindBuffer(GL_ARRAY_BUFFER, mesh->rendererData.vertexBufferKey);
      glBufferData(GL_ARRAY_BUFFER, sizeof(SkinnedVertex) * mesh->vertexCount, mesh->vertices, GL_DYNAMIC_DRAW);
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

  for (int i = 0; i < 4; i++)
    {
      animation->currentFrames[i] = 0;
      animation->currentInterps[i] = 0.0;
    }
    animation->currentInterps[0] = 1.0;
  u32* jointFlags;
  u32* startIndices;
  u32 numAnimatedComponents;
  int framesAdded = 0;
  char buffer[512];
  while (fgets(buffer, 512, fileHandle))
    {
      printBuffer();
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
	  animation->compositeMatrices = (mat4*)malloc(sizeof(mat4) * animation->jointCount);
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
	      printBuffer();
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
	      printBuffer();
	      sscanf(buffer, "\t( %f %f %f ) ( %f %f %f )",
		     &animation->frames[i].mins.x,
		     &animation->frames[i].mins.y,
		     &animation->frames[i].mins.z,
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
	      printBuffer();
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
	      	      printBuffer();
	      u32 ret = sscanf(buffer, "\t%f %f %f %f %f %f",
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
