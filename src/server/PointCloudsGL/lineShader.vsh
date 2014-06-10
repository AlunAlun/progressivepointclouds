#version 120

attribute vec3 vertices;

uniform mat4 mvp;

void main() {

    
    gl_Position = mvp * vec4(vertices,1.0);


}