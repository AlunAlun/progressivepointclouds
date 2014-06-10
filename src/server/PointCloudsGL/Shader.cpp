//
//  Shader.cpp
//  PointCloudsGL
//
//  Created by Alun on 10/06/14.
//  Copyright (c) 2014 Alun. All rights reserved.
//

#include "Shader.h"

char* Shader::readFile(const char* filename)
{
	FILE* fp=fopen(filename,"r");
	fseek(fp,0,SEEK_END);
	long file_length=ftell(fp);
	fseek(fp,0,SEEK_SET);
	char* contents=new char[file_length+1];
	for(int i=0;i<file_length+1;i++)
	{
		contents[i]=0;
	}
	fread(contents,1,file_length,fp);
	contents[file_length+1]='\0';
	fclose(fp);
	return contents;
}

Shader::Shader(const char* vertSource, const char* fragSource) {

    char* vertexShaderSourceCode=readFile("vertexShader.vsh");
    char* fragmentShaderSourceCode=readFile("fragmentShader.fsh");
    makeShaderProgram(makeVertexShader(vertexShaderSourceCode), makeFragmentShader(fragmentShaderSourceCode));
}

GLuint Shader::makeVertexShader(const char* shaderSource)
{
	GLuint vertexShaderID=glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShaderID,1,(const GLchar**)&shaderSource, NULL);
	glCompileShader(vertexShaderID);
	return vertexShaderID;
}
GLuint Shader::makeFragmentShader(const char* shaderSource)
{
	GLuint vertexShaderID=glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(vertexShaderID,1,(const GLchar**)&shaderSource, NULL);
	glCompileShader(vertexShaderID);
	return vertexShaderID;
}
void Shader::makeShaderProgram(GLuint vertexShaderID,GLuint fragmentShaderID)
{
	program=glCreateProgram();
	glAttachShader(program, vertexShaderID);
	glAttachShader(program,fragmentShaderID);
	glLinkProgram(program);
    GLint link_ok = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
    if (!link_ok) {
        fprintf(stderr, "glLinkProgram:");
    }
}

GLint Shader::bindAttribute(const char* attribute_name) {
    GLint attribute_ID = glGetAttribLocation(program, attribute_name);
    if (attribute_ID == -1) {
        fprintf(stderr, "Could not bind attribute %s\n", attribute_name);
        return 0;
    }
    else return attribute_ID;
}

GLint Shader::bindUniform(const char* uniform_name) {
    GLint uniform_ID = glGetUniformLocation(program, uniform_name);
    if (uniform_ID == -1) {
        fprintf(stderr, "Could not bind uniform %s\n", uniform_name);
        return 0;
    }
    else return uniform_ID;
}
