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
struct MD5Mesh
{
  int numJoints;
  int numMeshes;
  
};
