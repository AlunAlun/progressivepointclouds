#version 120

attribute vec3 vertices;
attribute vec3 colors;

uniform mat4 mvp;
uniform float heightOfNearPlane;
uniform float pointSize;

varying vec3 vertex_colors;

void main() {
    

    vertex_colors = colors;

    gl_Position = mvp * vec4(vertices,1.0);
    gl_PointSize = (heightOfNearPlane * pointSize) / gl_Position.w;


}