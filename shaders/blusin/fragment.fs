#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform float offset;
uniform sampler2D screenTexture;

void main() {
    vec2 texCoords = TexCoords;
    texCoords.y += cos(TexCoords.x * 4*2*3.14159 + offset) / 100;
    texCoords.x += cos(TexCoords.y * 4*2*3.14159 + offset) / 100;

 vec2 offsets[9] = vec2[](
        vec2(-offset,  offset), // top-left
        vec2( 0.0,    offset), // top-center
        vec2( offset,  offset), // top-right
        vec2(-offset,  0.0),   // center-left
        vec2( 0.0,    0.0f),   // center-center
        vec2( offset,  0.0),   // center-right
        vec2(-offset, -offset), // bottom-left
        vec2( 0.0,   -offset), // bottom-center
        vec2( offset, -offset)  // bottom-right    
    );

    float kernel[9] = float[](
    1.0 / 16, 2.0 / 16, 1.0 / 16,
    2.0 / 16, 4.0 / 16, 2.0 / 16,
    1.0 / 16, 2.0 / 16, 1.0 / 16  
);
    
    vec3 sampleTex[9];
    for(int i = 0; i < 9; i++)
    {
        sampleTex[i] = vec3(texture(screenTexture, texCoords.st + offsets[i]));
    }
    vec3 col = vec3(0.0);
    for(int i = 0; i < 9; i++)
        col += sampleTex[i] * kernel[i];

    FragColor = vec4(col, 1.0);
}