
#if 0
#include "miniz.c"
#include "ofbx.cpp"
SkinnedMesh* loadFBX(const char* fileName)
{
  FILE* fp = fopen(fileName, "rb");

  if(!fp) {
    fprintf(stderr, "CANNOT LOAD FBX FILE: %s", fileName);
  }

  fseek(fp, 0, SEEK_END);
  long file_size = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  ofbx::u8* content = new ofbx::u8[file_size];
  fread(content, 1, file_size, fp);
  ofbx::IScene* scene = ofbx::load((ofbx::u8*)content, file_size, (ofbx::u64)ofbx::LoadFlags::TRIANGULATE);
  if(!scene) {
    fprintf(stderr, "IMPROPERLY FORMATTED FBX SCENE: %s", fileName);
  }

  delete[] content;
  fclose(fp);

  SkinnedMesh* skinnedMesh = (SkinnedMesh*)calloc(1, sizeof(SkinnedMesh));
  int meshCount = scene->getMeshCount();
  skinnedMesh->meshes = (Mesh**)malloc(sizeof(Mesh*) * meshCount);
  skinnedMesh->meshCount = meshCount;
  for (int i = 0; i < meshCount; i++)
    {
      Mesh* mesh = (Mesh*)calloc(1, sizeof(Mesh));
      mesh->visible = 1;
      mesh->skinnedMesh = skinnedMesh;
      skinnedMesh->meshes[i] = mesh;
      
      const ofbx::Mesh& ofbxMesh = *scene->getMesh(i);      
      const ofbx::Geometry& geom = *ofbxMesh.getGeometry();
      int vertexCount = geom.getVertexCount();
      mesh->vertexCount = vertexCount;
      mesh->vertices = calloc(vertexCount, sizeof(SkinnedVertex));
      const ofbx::Vec3* vertices = geom.getVertices();
      for (int j = 0; j < vertexCount; j++)
	{
	  SkinnedVertex* vert = &((SkinnedVertex*)mesh->vertices)[j];
	  ofbx::Vec3 v = vertices[j];
	  printf("v %f %f %f\n", v.x, v.y, v.z);
	  vert->pos.x = v.x;
	  vert->pos.y = v.y;
	  vert->pos.z = v.z;
	}

      const int* faceIndices = geom.getFaceIndices();
      int index_count = geom.getIndexCount();
      mesh->indices = (u32*)malloc(sizeof(u32) * index_count);
      mesh->rendererData.indexCount = index_count;
      for (int j = 0; j < index_count; j++)
	{
	  //idx is 1 to n in obj style
	  int idx = (faceIndices[j] < 0) ? -faceIndices[j] : (faceIndices[j] + 1);
	  //reset to index, 0 to n-1
	  idx--; 
	  mesh->indices[j] = idx;
	}

      bool has_normals = geom.getNormals() != nullptr;
      if (has_normals)
	{
	  const ofbx::Vec3* normals = geom.getNormals();
	  int count = geom.getIndexCount();

	  for (int j = 0; j < count; j++)
	    {
	      ofbx::Vec3 n = normals[j];
	      //printf("vn %f %f %f\n", n.x, n.y, n.z);
	      SkinnedVertex* v = &((SkinnedVertex*)mesh->vertices)[mesh->indices[j]];
	      Assert(mesh->indices[j] < vertexCount);
	      v->normal.x += n.x;
	      v->normal.y += n.y;
	      v->normal.z += n.z;
	    }
	  for (int j = 0; j < vertexCount; j++)
	    {
	      SkinnedVertex* v = &((SkinnedVertex*)mesh->vertices)[mesh->indices[j]];
	      v->normal = Normalize(v->normal);
	    }	  
	}
      /*
      bool has_uvs = geom.getUVs() != nullptr;
      if (has_uvs)
	{
	  const ofbx::Vec2* uvs = geom.getUVs();
	  int count = geom.getIndexCount();

	  for (int i = 0; i < count; ++i)
	    {
	      ofbx::Vec2 uv = uvs[i];
	      fprintf(fp, "vt %f %f\n", uv.x, uv.y);
	    }
	}
      */

      //indices_offset += vertexCount;
      //normals_offset += index_count;
    }
  scene->destroy();
  return skinnedMesh;
}
#endif
