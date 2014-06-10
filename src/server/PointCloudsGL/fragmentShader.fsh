#version 120

varying vec3 vertex_colors;

void main() { 

    
    gl_FragColor = vec4(vertex_colors, 1.0);
}