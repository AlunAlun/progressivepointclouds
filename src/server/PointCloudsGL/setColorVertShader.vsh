#version 120

attribute vec3 vertices;
attribute vec3 colors;
attribute float point_sizes;

uniform mat4 mvp;
uniform float heightOfNearPlane;


varying vec3 vertex_colors;

void main() {
    
    vertex_colors = colors;
    
    gl_Position = mvp * vec4(vertices,1.0);
    gl_PointSize = (heightOfNearPlane * point_sizes) / gl_Position.w;


}