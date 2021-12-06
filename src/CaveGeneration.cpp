#include "../res/tables/MarchingCubesTable.cpp"
#include "PerlinNoise.cpp"
float* generateCaveGrid(vec3i dims)
{
  int width = dims.x;
  int length = dims.z;
  int height = dims.y;
  /*
  float* grid = (float*)malloc(sizeof(float) * width * height * length);
    for (int x = 0; x < width; x++)
    {
      for (int y = 0; y < height; y++)
	{
	  for (int z = 0; z < length; z++)
	    {
	      grid[z + y * length + x * length * height] =
		//	(f32)x / width;
		sqrt((f32)((x - width/2) * (x - width / 2) + (y - height/2) * (y-height/2) + (z-length/2) * (z-length/2))) / (width - 2);
		//randomUnilateral32(&globalEntropy);
	    }
	}
	}*/
  float* grid = generatePerlinNoiseGrid(width, height, length);
  return grid;

}




vec2i getVerticesFromEdge(int edge)
{  
  switch (edge) {
  case 0: {
    vec2i result = { 1, 2 };
    return result;
  }
  case 1: {
    vec2i result = { 2, 3 };
    return result;
  }
  case 2: {
    vec2i result = { 3, 4 };
    return result;
  }
  case 3: {
    vec2i result = { 4, 1 };
    return result;
  }

  case 4: {
    vec2i result = { 5, 6 };
    return result;
  }
  case 5: {
    vec2i result = { 6, 7 };
    return result;
  }
  case 6: {
    vec2i result = { 7, 8 };
    return result;
  }
  case 7: {
    vec2i result = { 8, 5 };
    return result;
  }

  case 8: {
    vec2i result = { 1, 5 };
    return result;
  }
  case 9: {
    vec2i result = { 2, 6 };
    return result;
  }
  case 10: {
    vec2i result = { 3, 7 };
    return result;
  }
  case 11: {
    vec2i result = { 4, 8 };
    return result;
  }
  }
  vec2i result = { -100, -100 };
  return result;
}
vec3 midPointFromIndex(int edge)
{
  switch (edge) {
  case 0: {
    vec3 result = {0, 0, 0.5f};
    return result;
  }
  case 1: {
    vec3 result = {0.5f, 0, 1};
    return result;
  }
  case 2: {
    vec3 result = {1.0f, 0, 0.5f};
    return result;
  }
  case 3: {
    vec3 result = {0.5f, 0, 0};
    return result;
  }

  case 4: {
    vec3 result = {0, 1.0f, 0.5f};
    return result;
  }
  case 5: {
    vec3 result = {0.5f, 1.0f, 1.0f};
    return result;
  }
  case 6: {
    vec3 result = {1.0f, 1.0f, 0.5f};
    return result;
  }
  case 7: {
    vec3 result = {0.5f, 1.0f, 0.0f};
    return result;
  }

  case 8: {
    vec3 result = {0.0f, 0.5f, 0.0f};
    return result;
  }
  case 9: {
    vec3 result = {0.0f, 0.5f, 1.0f};
    return result;
  }
  case 10: {
    vec3 result = {1.0f, 0.5f, 1.0f};
    return result;
  }
  case 11: {
    vec3 result = {1.0f, 0.5f, 0.0f};
    return result;
  }
  }
  vec3 result = {-100000.0f, 100000.0f, 0.0f};
}
Entity* marchCubes(float* noiseGrid, vec3i dims)
{
  Vertex* vertices = (Vertex*)malloc(sizeof(Vertex) * 100);
  u32 vertexSize = 100;
  u32 vertexCount = 0;
  u32 indexCount = 0;
  for (int x = 0; x < dims.x - 1; x++)
    {
      for (int y = 0; y < dims.y - 1; y++)
	{
	  for (int z = 0; z < dims.z - 1; z++)
	    {

	      float vertValues[8] = {
		noiseGrid[(x * dims.z * dims.y) + (y * dims.z) + z],
		noiseGrid[(x * dims.z * dims.y) + (y * dims.z) + (z + 1)],
		noiseGrid[((x + 1) * dims.z * dims.y) + (y * dims.z) + (z + 1)],
		noiseGrid[((x + 1) * dims.z * dims.y) + (y * dims.z) + z],
		noiseGrid[(x * dims.z * dims.y) + ((y + 1) * dims.z) + z],
		noiseGrid[(x * dims.z * dims.y) + ((y + 1) * dims.z) + (z + 1)],
		noiseGrid[((x + 1) * dims.z * dims.y) + ((y + 1) * dims.z) + (z + 1)],
		noiseGrid[((x + 1) * dims.z * dims.y) + ((y + 1) * dims.z) + z],
	      };

	      int cubeIndex = 0;
	      for (int i = 0; i < 8; i++)
		{
		  if (vertValues[i] < 0.0)
		    {
		      cubeIndex |= 1 << i;
		    }
		}

	      int* triangulation = triTable[cubeIndex];
	      //Debug.Log(string.Join(",", triangulation));
	      for (int i = 0; i < 12; i += 3)
		{
		  if (triangulation[i] != -1)
		    {
		      vec3 offset = {x - dims.x / 2, y - dims.y / 2, z - dims.z / 2};

		      vec3 vertex1 = midPointFromIndex(triangulation[i]);
		      vec3 vertex2 = midPointFromIndex(triangulation[i + 1]);
		      vec3 vertex3 = midPointFromIndex(triangulation[i + 2]);

		      
		      vertex1 += offset;
		      vertex2 += offset;
		      vertex3 += offset;

		      /*		      printf("%.1f %.1f %.1f  :  %.1f %.1f %.1f : %.1f %.1f %.1f\n",
			     vertex1.x, vertex1.y, vertex1.z,
			     vertex2.x, vertex2.y, vertex2.z,
			     vertex3.x, vertex3.y, vertex3.z);
		      */
		      if (vertexCount >= vertexSize - 3)
			{
			  vertexSize *= 2;
			  vertices = (Vertex*)realloc(vertices, sizeof(Vertex) * vertexSize);
			}

		      vec3 normal = Cross(vertex2 - vertex1, vertex3 - vertex2);
		      Vertex v1 =
			{
			  vertex1.x ,  vertex1.y ,  vertex1.z ,
			  normal.x,  normal.y,  normal.z,
			  0.0,  0.0,
			  0			  
			};
		      Vertex v2 =
			{
			  vertex2.x ,  vertex2.y ,  vertex2.z ,
			  normal.x,  normal.y,  normal.z,
			  0.0,  0.0,
			  0			  
			};
		      Vertex v3 =
			{
			  vertex3.x ,  vertex3.y ,  vertex3.z ,
			  normal.x,  normal.y,  normal.z,
			  0.0,  0.0,
			  0			  
			};
		      vertices[vertexCount++] = v1;
		      vertices[vertexCount++] = v2;
		      vertices[vertexCount++] = v3;
		      indexCount += 3;
		    }
		}
	    }
	}
    }
  vertices = (Vertex*)realloc(vertices, sizeof(Vertex) * vertexCount);
  u32* indices = (u32*)malloc(sizeof(u32) * indexCount);
  for (int i = 0; i < indexCount; i++) indices[i] = i;
  Entity* newEnt = createEntityFromTriangles(vertices, vertexCount, indices, indexCount);
  free(vertices);
  free(indices);
  return newEnt;
}

/*
vec3i getMidPointIndex(int edge, vec2i currentCoord)
{
  int x = currentCoord[0] * 2;
  int y = currentCoord[1] * 2;
  int z = currentCoord[2] * 2;
  switch (edge)
    {
    case 0:
      return new int[] { x, y, z + 1 };
    case 1:
      return new int[] { x + 1, y, z + 2 };
    case 2:
      return new int[] { x + 2, y, z + 1 };
    case 3:
      return new int[] { x + 1, y, z + 1 };

    case 4:
      return new int[] { x, y + 2, z + 1 };
    case 5:
      return new int[] { x + 1, y + 2, z + 2 };
    case 6:
      return new int[] { x + 2, y + 2, z + 2 };
    case 7:
      return new int[] { x + 1, y + 2, z };

    case 8:
      return new int[] { x, y + 1, z };
    case 9:
      return new int[] { x, y + 1, z + 2 };
    case 10:
      return new int[] { x + 2, y + 1, z + 2 };
    case 11:
      return new int[] { x + 2, y + 1, z };
    }
  return null;
}
*/
