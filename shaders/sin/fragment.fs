#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform float offset;
uniform sampler2D screenTexture;

void main() {
    vec2 texCoords = TexCoords;
    texCoords.y += cos(TexCoords.x * 4*2*3.14159 + offset) / 100;
    texCoords.x += cos(TexCoords.y * 4*2*3.14159 + offset) / 100;

    FragColor = texture(screenTexture, texCoords);
}