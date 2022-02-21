#include "Mesh.h"

void calculateSkinnedCompositeMatrices(SkinnedMesh* skinnedMesh)
{
  if (skinnedMesh->currentAnimation == -1) return;
  SkinnedAnimation* anim = skinnedMesh->animations + skinnedMesh->currentAnimation;
  for (int i = 0; i < anim->jointCount; i++)
    {
      vec3 position = {0.0,0.0,0.0};
      quaternion orientation = {0.0,0.0,0.0,1.0};
      mat4 temp = Mat4d(0.0);
      for (int j = 0; j < 2; j++) //TODO multiple animations?
	{
	  /*
	  position += anim->frames[anim->currentFrames[j]].joints[i].position *
	    anim->currentInterps[j];
	  orientation += anim->frames[anim->currentFrames[j]].joints[i].orientation *
	  anim->currentInterps[j];*/
	  //TODO: slerp orientations for better rotation-interpolation
	  temp += anim->frames[anim->currentFrames[j]].joints[i].transform *  skinnedMesh->restInverses[i] * anim->currentInterps[j];
	}
      anim->compositeMatrices[i] = temp;
    }
}
