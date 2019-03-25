#version 330 core
in vec2 TexCoords;  
out vec4 FragColor;

uniform sampler2D screenTexture; 

float rt_w = 1200; 
float rt_h = 800; 

// Swirl effect parameters
float radius = 400.0;
uniform float angle;
vec2 center = vec2(600.0, 400.0);

vec4 PostFX(sampler2D screenTexture, vec2 uv)
{
  vec2 texSize = vec2(rt_w, rt_h);
  vec2 tc = uv * texSize;
  tc -= center;
  float dist = length(tc);
  if (dist < radius) 
  {
    float percent = (radius - dist) / radius;
    float theta = percent * percent * angle * 8.0;
    float s = sin(theta);
    float c = cos(theta);
    tc = vec2(dot(tc, vec2(c, -s)), dot(tc, vec2(s, c)));
  }
  tc += center;
  vec3 color = texture2D(screenTexture, tc / texSize).rgb;
  return vec4(color, 1.0);
}

void main (void)
{
  vec2 uv = TexCoords;
  FragColor = PostFX(screenTexture, uv);
}