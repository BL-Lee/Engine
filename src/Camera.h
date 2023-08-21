#ifndef __CAMERA_HEADER
#define __CAMERA_HEADER

#define CAMERA_PERSPECTIVE 1
#define CAMERA_ORTHOGRAPHIC 0

typedef int CameraViewType;
struct Camera
{
  CameraViewType viewType;
  mat4 projectionMatrix;
  mat4 viewMatrix;
  mat4 invViewMatrix;
  vec3 pos;
  vec3 direction;
  vec3 rotation;

  bool inverseViewDirty;

  f32 width;
  f32 height;
  f32 farClip;
  f32 nearClip;
  f32 vertAngleOfView;
  f32 aspectRatio;
  f32 scale;

  vec2 viewportMin;
  vec2 viewportMax;
};

//TODO: update pos, rotation
void initCamera(Camera* c);
void initCamera(Camera* c,
		float width, float height,
		float vAOV,
		float nearClip, float farClip,
		vec3 pos, vec3 dir);		
void setPerspectiveMatrix(Camera* c);
void setOrthographicMatrix(Camera* c);
void translateCameraLocal(Camera* c, vec3 delta);
void translateCameraGlobal(Camera* c, vec3 delta);
void rotateCameraLocal(Camera* c, vec3 amount);
void rotateCameraGlobal(Camera* c, vec3 amount);
bool gluInvertMatrix(const float m[16], float invOut[16]);
vec3 getCameraPos(Camera* c);
void getFrustumCornersWorldSpace(vec4* viewSpaceCorners, Camera* c);

#endif
