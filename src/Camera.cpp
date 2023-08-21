#include "Camera.h"
//#include "ImportUtils.cpp"
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
  c->width = width;
  c->height = height;
  c->viewportMin = {0, 0};
  c->viewportMax = {width, height};
  c->pos = pos;
  c->direction = dir;
  vec3 zero = {0.0,0.0,0.0};
  c->rotation = zero;
  initCamera(c);
}

void setCameraViewMatrix(Camera* c)
{
  vec3 globalY = {0.0,1.0,0.0};
  c->viewMatrix = LookAt(c->pos, c->pos + c->direction, globalY);
  c->inverseViewDirty = true;  
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
  vec3 local = {0.0,0.0,0.0};
  local -= delta.z *  c->direction;
  vec3 globalY = {0.0,1.0,0.0};
  local += Normalize(Cross(c->direction, globalY)) * delta.x;
  c->pos += local;
  setCameraViewMatrix(c);
}
void rotateCameraLocal(Camera* c, vec3 amount)
{
  c->rotation += amount;
  c->direction.x = cos(c->rotation.y) * cos(c->rotation.x);
  c->direction.y = sin(c->rotation.x);
  c->direction.z = sin(c->rotation.y) * cos(c->rotation.x);
  c->direction = Normalize(c->direction);
  setCameraViewMatrix(c);
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

void getFrustumCornersWorldSpace(vec4* viewSpaceCorners, Camera* c)
{
  //vec4 viewSpaceCorners[8];
  //near left bottom
  viewSpaceCorners[0] = {-1.0, -1.0, -1.0, 1.0};
  viewSpaceCorners[1] = {1.0, -1.0, -1.0, 1.0};
  viewSpaceCorners[2] = {-1.0, 1.0, -1.0, 1.0};
  viewSpaceCorners[3] = {1.0, 1.0, -1.0, 1.0};

  viewSpaceCorners[4] = {-1.0, -1.0, 1.0, 1.0};
  viewSpaceCorners[5] = {1.0, -1.0, 1.0, 1.0};
  viewSpaceCorners[6] = {-1.0, 1.0, 1.0, 1.0};
  viewSpaceCorners[7] = {1.0, 1.0, 1.0, 1.0};

  mat4 projToWorld;
  mat4 VP = c->projectionMatrix * c->viewMatrix;
  gluInvertMatrix((float*)&VP, (float*)&projToWorld);
  for (int i = 0; i < 8; i++)
    {
      viewSpaceCorners[i] = projToWorld * viewSpaceCorners[i];
      viewSpaceCorners[i] /= viewSpaceCorners[i].w;
    }
}

Ray rayFromScreenPoint(vec2 mouseCoords)
{
  Ray ray;
  vec4 zero = {0.0,0.0,0.0,1.0};
  
  //Will calculate invViewMatrix to get this
  ray.origin = getCameraPos(&mainCamera);

  //mouse coords from [-1,1]
  vec2 mousePosCoords = { (mouseCoords.x / mainWindow.width) * 2 - 1, (mouseCoords.y / mainWindow.height) * 2 - 1} ;

  //Ray from origin (camera eye) to point on the screen on far plane
  //All camera space
  vec4 rayDirCameraSpace = {
    TanF(ToRadians(mainCamera.vertAngleOfView * 0.5)) * mainCamera.aspectRatio * mainCamera.farClip * mousePosCoords.x,
    TanF(ToRadians(mainCamera.vertAngleOfView * 0.5)) * mainCamera.farClip * -mousePosCoords.y,
    -mainCamera.farClip,
    0.0
  };

  //Direction is that ray in world space
  ray.direction = Normalize(mainCamera.invViewMatrix * rayDirCameraSpace).xyz;
  return ray;
}
