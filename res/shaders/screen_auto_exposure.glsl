#version 330 core
out vec4 FragColor;
  
in vec2 TexCoord;

uniform sampler2D screenTexture;
uniform float exposure;

vec3 ACESFilm(vec3 x)
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return clamp(((x*(a*x+b))/(x*(c*x+d)+e)), 0.0, 1.0);
}

void main()
{
    const float gamma = 2.2;
    vec3 hdrColor = texture(screenTexture, TexCoord).rgb;
  
    // exposure tone mapping
    vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);

    //tone mapping
    mapped = ACESFilm(mapped);
    
    // gamma correction 
    mapped = pow(mapped, vec3(1.0 / gamma));
  
    FragColor = vec4(mapped, 1.0);
}
