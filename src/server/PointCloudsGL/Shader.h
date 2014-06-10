//
//  Shader.h
//  PointCloudsGL
//
//  Created by Alun on 10/06/14.
//  Copyright (c) 2014 Alun. All rights reserved.
//

#ifndef __PointCloudsGL__Shader__
#define __PointCloudsGL__Shader__

#include <iostream>
#include <GLUT/glut.h>

class Shader {
public:
    GLuint program;

    Shader(const char* vertSource, const char* fragSource);
    static char* readFile(const char* filename);
    GLuint makeVertexShader(const char* shaderSource);
    GLuint makeFragmentShader(const char* shaderSource);
    void makeShaderProgram(GLuint vertexShaderID, GLuint fragmentShaderID);
    GLint bindAttribute(const char* attribute_name);
    GLint bindUniform(const char* uniform_name);
    
    
};

#endif /* defined(__PointCloudsGL__Shader__) */
