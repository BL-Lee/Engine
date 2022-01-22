#include "Camera.h"

//TODO: update pos, rotation
void initCamera(Camera* c)
{
  vec3 cameraTarget = {0.0,0.0,0.0};
  vec3 globalY = {0.0,1.0,0.0};
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


