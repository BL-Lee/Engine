#include "Camera.h"

//TODO: update pos, rotation
void initCamera(Camera* c)
{

 vec3 cameraTarget = {0.0,0.0,0.0};
  vec3 zAxis = NormalizeVec3(-c->direction);
  vec3 globalY = {0.0,1.0,0.0};
  vec3 xAxis = NormalizeVec3(Cross(globalY, zAxis));
  vec3 yAxis = NormalizeVec3(Cross(zAxis, xAxis));

  mat4 v =
    { xAxis.x,                yAxis.x, zAxis.x, 0.0,
      xAxis.y,                yAxis.y, zAxis.y, 0.0,
      xAxis.z,                yAxis.z, zAxis.z, 0.0,
      -c->pos.x, -c->pos.y, -c->pos.z, 1.0      
    };

  c->viewMatrix = v;
  c->viewMatrix = LookAt(c->pos, cameraTarget, globalY);
  c->inverseViewDirty = true;
}
void initCamera(Camera* c,
		float width, float height,
		float vAOV,
		float nearClip, float farClip,
		vec3 pos, vec3 dir)
{
  c->aspectRatio = width / height;
  c->vertAngleOfView = vAOV;
  c->nearClip = nearClip;
  c->farClip = farClip;
  c->pos = pos;
  c->direction = dir;
  initCamera(c);
}
		
void setPerspectiveMatrix(Camera* c)
{
    mat4 matrix = Perspective(c->vertAngleOfView, c->aspectRatio, c->nearClip, c->farClip);
    c->projectionMatrix = matrix;
    c->viewType = CAMERA_PERSPECTIVE;
      c->inverseViewDirty = true;
}

void setOrthographicMatrix(Camera* c)
{
  f32 t = -tan(c->vertAngleOfView * HMM_PI32 / 360.0f) * 5.0f;
  f32 b = -t;
  f32 r = t * c->aspectRatio;
  f32 l = -r;
  mat4 matrix = Orthographic(t, b, r, l, c->nearClip, c->farClip);
  c->projectionMatrix = matrix;
  c->viewType = CAMERA_ORTHOGRAPHIC;
    c->inverseViewDirty = true;
}

void translateCameraLocal(Camera* c, vec3 delta)
{
  c->viewMatrix =  Translate(-delta) * c->viewMatrix;
  c->inverseViewDirty = true;
}
void translateCameraGlobal(Camera* c, vec3 delta)
{
  c->viewMatrix =  c->viewMatrix * Translate(-delta);
    c->inverseViewDirty = true;
}

void rotateCameraLocal(Camera* c, vec3 amount)
{
  vec3 zAxis = {0.0,0.0,1.0};
  vec3 yAxis = {0.0,1.0,0.0};
  vec3 xAxis = {1.0,0.0,0.0};
  c->viewMatrix = Rotate(amount.z, zAxis) *
    Rotate(amount.y, yAxis) *
    Rotate(amount.x, xAxis) *
    c->viewMatrix;
    c->inverseViewDirty = true;
}
void rotateCameraGlobal(Camera* c, vec3 amount)
{
  vec3 zAxis = {0.0,0.0,1.0};
  vec3 yAxis = {0.0,1.0,0.0};
  vec3 xAxis = {1.0,0.0,0.0};
  c->viewMatrix = c->viewMatrix * Rotate(amount.z, zAxis) *
    Rotate(amount.y, yAxis) *
    Rotate(amount.x, xAxis);
    c->inverseViewDirty = true;
}

bool gluInvertMatrix(const float m[16], float invOut[16])
{
    float inv[16], det;
    int i;

    inv[0] = m[5]  * m[10] * m[15] - 
             m[5]  * m[11] * m[14] - 
             m[9]  * m[6]  * m[15] + 
             m[9]  * m[7]  * m[14] +
             m[13] * m[6]  * m[11] - 
             m[13] * m[7]  * m[10];

    inv[4] = -m[4]  * m[10] * m[15] + 
              m[4]  * m[11] * m[14] + 
              m[8]  * m[6]  * m[15] - 
              m[8]  * m[7]  * m[14] - 
              m[12] * m[6]  * m[11] + 
              m[12] * m[7]  * m[10];

    inv[8] = m[4]  * m[9] * m[15] - 
             m[4]  * m[11] * m[13] - 
             m[8]  * m[5] * m[15] + 
             m[8]  * m[7] * m[13] + 
             m[12] * m[5] * m[11] - 
             m[12] * m[7] * m[9];

    inv[12] = -m[4]  * m[9] * m[14] + 
               m[4]  * m[10] * m[13] +
               m[8]  * m[5] * m[14] - 
               m[8]  * m[6] * m[13] - 
               m[12] * m[5] * m[10] + 
               m[12] * m[6] * m[9];

    inv[1] = -m[1]  * m[10] * m[15] + 
              m[1]  * m[11] * m[14] + 
              m[9]  * m[2] * m[15] - 
              m[9]  * m[3] * m[14] - 
              m[13] * m[2] * m[11] + 
              m[13] * m[3] * m[10];

    inv[5] = m[0]  * m[10] * m[15] - 
             m[0]  * m[11] * m[14] - 
             m[8]  * m[2] * m[15] + 
             m[8]  * m[3] * m[14] + 
             m[12] * m[2] * m[11] - 
             m[12] * m[3] * m[10];

    inv[9] = -m[0]  * m[9] * m[15] + 
              m[0]  * m[11] * m[13] + 
              m[8]  * m[1] * m[15] - 
              m[8]  * m[3] * m[13] - 
              m[12] * m[1] * m[11] + 
              m[12] * m[3] * m[9];

    inv[13] = m[0]  * m[9] * m[14] - 
              m[0]  * m[10] * m[13] - 
              m[8]  * m[1] * m[14] + 
              m[8]  * m[2] * m[13] + 
              m[12] * m[1] * m[10] - 
              m[12] * m[2] * m[9];

    inv[2] = m[1]  * m[6] * m[15] - 
             m[1]  * m[7] * m[14] - 
             m[5]  * m[2] * m[15] + 
             m[5]  * m[3] * m[14] + 
             m[13] * m[2] * m[7] - 
             m[13] * m[3] * m[6];

    inv[6] = -m[0]  * m[6] * m[15] + 
              m[0]  * m[7] * m[14] + 
              m[4]  * m[2] * m[15] - 
              m[4]  * m[3] * m[14] - 
              m[12] * m[2] * m[7] + 
              m[12] * m[3] * m[6];

    inv[10] = m[0]  * m[5] * m[15] - 
              m[0]  * m[7] * m[13] - 
              m[4]  * m[1] * m[15] + 
              m[4]  * m[3] * m[13] + 
              m[12] * m[1] * m[7] - 
              m[12] * m[3] * m[5];

    inv[14] = -m[0]  * m[5] * m[14] + 
               m[0]  * m[6] * m[13] + 
               m[4]  * m[1] * m[14] - 
               m[4]  * m[2] * m[13] - 
               m[12] * m[1] * m[6] + 
               m[12] * m[2] * m[5];

    inv[3] = -m[1] * m[6] * m[11] + 
              m[1] * m[7] * m[10] + 
              m[5] * m[2] * m[11] - 
              m[5] * m[3] * m[10] - 
              m[9] * m[2] * m[7] + 
              m[9] * m[3] * m[6];

    inv[7] = m[0] * m[6] * m[11] - 
             m[0] * m[7] * m[10] - 
             m[4] * m[2] * m[11] + 
             m[4] * m[3] * m[10] + 
             m[8] * m[2] * m[7] - 
             m[8] * m[3] * m[6];

    inv[11] = -m[0] * m[5] * m[11] + 
               m[0] * m[7] * m[9] + 
               m[4] * m[1] * m[11] - 
               m[4] * m[3] * m[9] - 
               m[8] * m[1] * m[7] + 
               m[8] * m[3] * m[5];

    inv[15] = m[0] * m[5] * m[10] - 
              m[0] * m[6] * m[9] - 
              m[4] * m[1] * m[10] + 
              m[4] * m[2] * m[9] + 
              m[8] * m[1] * m[6] - 
              m[8] * m[2] * m[5];

    det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

    if (det == 0)
        return false;

    det = 1.0 / det;

    for (i = 0; i < 16; i++)
        invOut[i] = inv[i] * det;

    return true;
}
vec3 getCameraPos(Camera* c)
{
  if (c->inverseViewDirty)
    {
      gluInvertMatrix((float*)&c->viewMatrix, (float*)&c->invViewMatrix);
      c->inverseViewDirty = false;
    }
  vec3 pos = {
    c->invViewMatrix[3][0],
        c->invViewMatrix[3][1],
	    c->invViewMatrix[3][2]
  };
  return pos;
}


/*
void cameraSetUp(Camera* c, vec3 up)
{
  vec3 pos = {c->transform[0][3],c->transform[1][3],c->transform[2][3]};
  vec3 dir = c->transform[2].XYZ;
  mat4 set = LookAt(pos, dir, up);
  c->transform = set * c->transform;
  c->pos = pos;
  c->direction = dir;
  }*/
