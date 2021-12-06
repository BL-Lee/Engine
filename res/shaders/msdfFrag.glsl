#version 330 core

in vec2 TexCoord;
in float TexUnit;
out vec4 color;
uniform sampler2D tex[16];

//uniform vec4 bgColor;
//uniform vec4 fgColor;
//uniform float pxRange; // set to distance field's pixel range

float screenPxRangeFunc() {
  float pxRange = 2;
  vec2 unitRange = vec2(pxRange)/vec2(textureSize(tex[int(TexUnit)], 0));
  vec2 screenTexSize = vec2(1.0)/fwidth(TexCoord);
  return max(0.5*dot(unitRange, screenTexSize), 1.0);
}
float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}
/*
void main() {
  vec3 msd = texture(tex[int(TexUnit)], TexCoord).rgb;
  float sd = median(msd.r, msd.g, msd.b);
  float pxRange = 4;
  vec2 unitRange = vec2(pxRange)/vec2(textureSize(tex[int(TexUnit)], 0));
  vec2 screenTexSize = vec2(1.0)/fwidth(TexCoord);
  float screenPxRange =  max(0.5*dot(unitRange, screenTexSize), 1.0);
  screenPxRange = 2;
  float screenPxDistance = screenPxRange*(sd - 0.5);
  float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);
  color = mix(vec4(0.0,1.0,0.0,0.0), vec4(0.0,1.0,1.0,1.0), opacity);
}
*/
void main() {
  vec3 msd = texture(tex[int(TexUnit)], TexCoord).rgb;
  float sd = median(msd.r, msd.g, msd.b);
  float screenPxDistance = screenPxRangeFunc()*(sd - 0.5);

  float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);
    color = vec4(opacity);
  return;

  color = mix(vec4(0.0,0.0,0.0,0.0), vec4(1.0,0.0,1.0,1.0), opacity);
}
