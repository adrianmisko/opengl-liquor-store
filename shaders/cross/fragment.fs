#version 330 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D screenTexture; 
uniform float time;
float rt_w = 1200;
float rt_h = 800;
float stitching_size = 6.0;
int invert = 0;

vec4 PostFX(sampler2D tex, vec2 uv, float time)
{
  vec4 c = vec4(0.0);
  float size = stitching_size;
  vec2 cPos = uv * vec2(rt_w, rt_h);
  vec2 tlPos = floor(cPos / vec2(size, size));
  tlPos *= size;
  int remX = int(mod(cPos.x, size));
  int remY = int(mod(cPos.y, size));
  if (remX == 0 && remY == 0)
    tlPos = cPos;
  vec2 blPos = tlPos;
  blPos.y += (size - 1.0);
  if ((remX == remY) || 
     (((int(cPos.x) - int(blPos.x)) == (int(blPos.y) - int(cPos.y)))))
  {
    if (invert == 1)
      c = vec4(0.2, 0.15, 0.05, 1.0);
    else
      c = texture2D(tex, tlPos * vec2(1.0/rt_w, 1.0/rt_h)) * 1.4;
  }
  else
  {
    if (invert == 1)
      c = texture2D(tex, tlPos * vec2(1.0/rt_w, 1.0/rt_h)) * 1.4;
    else
      c = vec4(0.0, 0.0, 0.0, 1.0);
  }
  return c;
}

void main (void)
{
  vec2 uv = TexCoords;
  if (uv.y > 0.5)
  {
    FragColor = PostFX(screenTexture, uv, time);
  }
  else
  {
    uv.y += 0.5;
    vec4 c1 = texture2D(screenTexture, uv);
    FragColor = c1;      
  }
}