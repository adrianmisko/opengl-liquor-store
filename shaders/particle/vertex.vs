#version 330 core
layout (location = 0) in vec3 vertex;

uniform mat4 V;
uniform mat4 P;
uniform mat4 M;

void main() {
    gl_Position = P * V * M * vec4(vertex, 1.0);
}