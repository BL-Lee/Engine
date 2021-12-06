#include <noise1234.c>

float* generatePerlinNoiseGrid(int x, int y, int z,
			       float scale,
			       vec3 offset)
{
  float* result = (float*)malloc(sizeof(float) * x * y * z);
  for (int i = 0; i < x; i++)
    {
      for (int j = 0; j < y; j++)
	{
	  for (int k = 0; k < z; k++)
	    {
	      int index =
		k + (j * z) + (i * y * z);
	      result[index] = noise3(
				     (i + offset.x) * scale,
				     (j + offset.y) * scale,
				     (k + offset.z) * scale
				     );
	      result[index] *= j % 2;
	    }
	}
    }
  return result;
}
float* generatePerlinNoiseGrid(int x, int y, int z)
{
  vec3 zero = {0};
  return generatePerlinNoiseGrid(x,y,z,
				 (f32)5/x,
				 zero);
}
