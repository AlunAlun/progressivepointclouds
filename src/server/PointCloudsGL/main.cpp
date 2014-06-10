//
//  main.cpp
//  OpenGlWindow
//
//  Created by Mariano Rajoy on 26/10/13.
//  Copyright (c) 2013 Mariano Rajoy. All rights reserved.
//

#include <cstdlib>
#include <GLUT/glut.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#define GLM_SWIZZLE
#include "glm.hpp"
#include "type_ptr.hpp"
#include "rotate_vector.hpp"
#include "matrix_transform.hpp"
#include "type_ptr.hpp"
#include "imageloader.h"
#include <math.h>
#include "octree.h"
#include "clipper.h"
#include "shader.h"
#include "FileLoaders.h"
#include "Geometry.h"
#include <ctime>
#include <map>

using namespace std;
using namespace glm;

#define MOVE_SPEED 0.009f
#define MOUSE_SPEED 0.2f
#define NUM_OCTREE_LEVELS 15
#define LINE_CUTOFF 5000
#define CELLSIZEFACTOR 30
#define POINT_SIZE_SCALE 2.5
#define POINT_SIZE 0.00001
//#define EXPORT_OCTREE 0
//#define USE_DEPTH_COLORS

#define POINTDENSITY 20

//string INFILEPATH = "/Users/alun/Downloads/CCSR1-merge-high.off";
//string OCTREEPATH = "/Users/alun/Desktop/testdatasets/CCSR1/32bit/";


string INFILEPATH = "/Users/alun/Desktop/3Ddata/happybuddha/happy.ply";
string OCTREEPATH = "/Users/alun/Desktop/3Ddata/happybuddha/";

//string INFILEPATH = "/Users/alunevans/Downloads/UniS-LIDARdata/Day3-Studio-part1.off";
//string OCTREEPATH = "/Users/alunevans/Desktop/testdatasets/studio1/5000/";

//string INFILEPATH = "/Users/alunevans/Downloads/UniS-LIDARdata/Day3-Studio-part2.off";
//string OCTREEPATH = "/Users/alunevans/Desktop/testdatasets/studio2/100000/" ;
//
//string INFILEPATH = "/Users/alunevans/Downloads/UniS-LIDARdata/Day4-Plaza.off";
//string OCTREEPATH = "/Users/alunevans/Desktop/testdatasets/plaza/100000/";

//string INFILEPATH = "/Users/alunevans/Downloads/MOUT_script3_barrel.xyzrgba";
//string OCTREEPATH = "/Users/alunevans/Desktop/bunny/";

//
//string INFILEPATH = "/Users/alunevans/Downloads/UniS-LIDARdata/Day4-Reception.off";
//string OCTREEPATH = "/Users/alunevans/Desktop/testdatasets/reception/dist/";

//float camX = 0, camY= 17, camZ=0; float camYaw = 99; float camPitch = -5;

// Converts degrees to radians.
#define degToRad(angleDegrees) (angleDegrees * M_PI / 180.0)

// Converts radians to degrees.
#define radToDeg(angleRadians) (angleRadians * 180.0 / M_PI)

const float PI = 3.1415927;



int tmpCounter = 0;
int tmpMaxDepth = 0;

//camera and movement
//float camX = 15.7, camY= -22.2, camZ=10.85; float camYaw = 111.4; float camPitch = -34.8;
//float camX = 0, camY= 17, camZ=0; float camYaw = 99; float camPitch = -5;
float camX = 17, camY= -70, camZ=13; float camYaw = 130; float camPitch = -29;

vec3 camPos, camForward;
bool moveForward = false, moveBackward = false, moveLeft = false, moveRight = false, mouseDown = false;
int mDownX, mDownY = 0;
mat4 mvp;

//point cloud buffers, shader and uniforms
GLuint program;
GLint attribute_vertices, attribute_colors;
GLint uniform_mvp, uniform_heightNearPlane, uniform_pointSize;
GLuint vbo_points_vertices, vbo_points_colors;
vector<vec3> VERTEX_off, COLOR_off;

//octree centres buffers, shader and uniforms
GLuint program_centres;
GLint attribute_centres_vertices, attribute_centres_colors, attribute_centres_sizes;
GLint uniform_mvp_centres, uniform_heightNearPlane_centres;
GLuint vbo_centres_vertices, vbo_centres_colors, vbo_centres_sizes;
vector<vec3> VERTEX_octree, COLOR_octree;
vector<float> POINTSIZE_octree;
vector<bool> ENABLED_octree;

//points-in-octree buffers, shader and uniforms
GLuint program_octpoints;
GLint attribute_octpoints_vertices, attribute_octpoints_colors;
GLint uniform_mvp_octpoints, uniform_heightNearPlane_octpoints, uniform_pointSize_octpoints;
GLuint vbo_octpoints_vertices, vbo_octpoints_colors;
vector<vec3> VERTEX_octpoints, COLOR_octpoints;
int num_octpoints_in_frame = 0;

//octree
Octree octree;
Node *octreeRoot;
int octreeCounter = 0;

bool octree_needs_updating = false;
vector<int> octreeIndices;
map<string, Node*> octreeMap;
int kbCounter = 0;


//frustum culling
Clipper clipper;
float heightOfNearPlane = 0;
int screen_width=800, screen_height=600;

//display flags
bool showPoints = true;
bool showOctree = false;
bool showOctPoints = false;

//depthColors
vec3 depthColors[10] = {
    vec3(0.4, 0.0, 0.0),
    vec3(0.6, 0.0, 0.0),
    vec3(0.8, 0.0, 0.0),
    vec3(1.0, 0.0, 0.0),
    vec3(1.0, 0.2, 0.0),
    vec3(1.0, 0.4, 0.0),
    vec3(1.0, 0.6, 0.0),
    vec3(1.0, 1.0, 0.0),
    vec3(1.0, 1.0, 0.2),
    vec3(1.0, 1.0, 0.2),
};

int buffersShadersPointCloud()
{
    
    //VERTEX BUFFER
    glGenBuffers(1, &vbo_points_vertices);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_points_vertices);
    glBufferData(GL_ARRAY_BUFFER, VERTEX_off.size() * sizeof(vec3), &VERTEX_off[0], GL_STATIC_DRAW);
    
    //COLOR BUFFER
    glGenBuffers(1, &vbo_points_colors);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_points_colors);
    glBufferData(GL_ARRAY_BUFFER, COLOR_off.size() * sizeof(vec3), &COLOR_off[0], GL_STATIC_DRAW);
    
    Shader pointCloudShader("vertexShader.vsh", "fragmentShader.fsh");

    program = pointCloudShader.program;
    attribute_vertices = pointCloudShader.bindAttribute("vertices");
    attribute_colors = pointCloudShader.bindAttribute("colors");
    uniform_mvp = pointCloudShader.bindUniform("mvp");
    uniform_heightNearPlane = pointCloudShader.bindUniform("heightOfNearPlane");
    uniform_pointSize = pointCloudShader.bindUniform("pointSize");
    
    return 1;
}

int buffersShadersOctPoints()
{
    
    
    VERTEX_octpoints.resize(VERTEX_off.size());
    COLOR_octpoints.resize(COLOR_off.size());
    
   
    //VERTEX BUFFER
    glGenBuffers(1, &vbo_octpoints_vertices);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_octpoints_vertices);
    glBufferData(GL_ARRAY_BUFFER, VERTEX_octpoints.size() * sizeof(vec3), &VERTEX_octpoints[0], GL_DYNAMIC_DRAW);
    
    //COLOR BUFFER
    glGenBuffers(1, &vbo_octpoints_colors);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_octpoints_colors);
    glBufferData(GL_ARRAY_BUFFER, COLOR_octpoints.size() * sizeof(vec3), &COLOR_octpoints[0], GL_DYNAMIC_DRAW);
    
    Shader octPointsShader("vertexShader.vsh", "fragmentShader.fsh");
    
    program_octpoints = octPointsShader.program;
    attribute_octpoints_vertices = octPointsShader.bindAttribute("vertices");
    attribute_octpoints_vertices = octPointsShader.bindAttribute("colors");
    uniform_mvp_octpoints = octPointsShader.bindUniform("mvp");
    uniform_heightNearPlane_octpoints = octPointsShader.bindUniform("heightOfNearPlane");
    uniform_pointSize_octpoints = octPointsShader.bindUniform("pointSize");
    

    return 1;
}


int buffersShadersOctree() {
    
    //VERTEX BUFFER
    glGenBuffers(1, &vbo_centres_vertices);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_centres_vertices);
    glBufferData(GL_ARRAY_BUFFER, VERTEX_octree.size() * sizeof(vec3), &VERTEX_octree[0], GL_STATIC_DRAW);
    
    //COLOR BUFFER
    glGenBuffers(1, &vbo_centres_colors);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_centres_colors);
    glBufferData(GL_ARRAY_BUFFER, COLOR_octree.size() * sizeof(vec3), &COLOR_octree[0], GL_STATIC_DRAW);
    
    //SIZE BUFFER
    glGenBuffers(1, &vbo_centres_sizes);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_centres_sizes);
    glBufferData(GL_ARRAY_BUFFER, POINTSIZE_octree.size() * sizeof(float), &POINTSIZE_octree[0], GL_STATIC_DRAW);
    
    Shader octCentresShader("vertexShader.vsh", "fragmentShader.fsh");
    
    program_centres = octCentresShader.program;
    attribute_centres_vertices = octCentresShader.bindAttribute("vertices");
    attribute_centres_colors = octCentresShader.bindAttribute("colors");
    attribute_centres_sizes = octCentresShader.bindAttribute("point_sizes");
    uniform_mvp_centres = octCentresShader.bindUniform("mvp");
    uniform_heightNearPlane_centres = octCentresShader.bindUniform("heightOfNearPlane");

    return 1;
}

void getOctreeBufferSize(Node *pTree) {
    pTree->nodeid = octreeCounter;
    octreeCounter++;
    
    for (int i = 0; i < 8; i++) {
        if (pTree->pChild[i] != NULL)
            getOctreeBufferSize(pTree->pChild[i]);
    }

}

void populateOctreeBuffers(Node *pTree) {
    VERTEX_octree[pTree->nodeid] = pTree->center;
#ifdef USE_DEPTH_COLORS
    COLOR_octree[pTree->nodeid] = depthColors[pTree->depth];
#else
    COLOR_octree[pTree->nodeid] = pTree->color;
#endif
    POINTSIZE_octree[pTree->nodeid] = pTree->halfWidth*POINT_SIZE_SCALE;
    ENABLED_octree[pTree->nodeid] = true;

    for (int i = 0; i < 8; i++) {
        if (pTree->pChild[i] != NULL)
            populateOctreeBuffers(pTree->pChild[i]);
    }
}


void createOctreeBuffers() {
    octreeCounter = 0;
    getOctreeBufferSize(octreeRoot);
    vector<vec3> newVector1 (octreeCounter);
    vector<vec3> newVector2 (octreeCounter);
    vector<float> newVector3 (octreeCounter);
    vector<bool> newVector4 (octreeCounter);
    VERTEX_octree = newVector1;
    COLOR_octree = newVector2;
    POINTSIZE_octree = newVector3;
    ENABLED_octree = newVector4;

    populateOctreeBuffers(octreeRoot);
    buffersShadersOctree();
    buffersShadersOctPoints();
    printf("\nsize octree drawlist - %d\n", (int)VERTEX_octree.size());
}


int init_resources()
{
    camPos = vec3(camX, camY, camZ);
    
    std::string::size_type idx;
    std::string extension;
    idx = INFILEPATH.rfind('.');
    if(idx != std::string::npos)
        extension = INFILEPATH.substr(idx+1);
    if (extension == "xyzrgba")
        FileLoaders::readXrgbFile(INFILEPATH.c_str(), VERTEX_off, COLOR_off);
    else if (extension == "off")
        FileLoaders::readOFFFile(INFILEPATH.c_str(), VERTEX_off, COLOR_off);
    else if (extension == "ply")
        FileLoaders::readPLYFile(INFILEPATH.c_str(), VERTEX_off, COLOR_off);
    
    buffersShadersPointCloud();
    
    printf("Building Octree...");
    vec3 min, max, center;
    float radius;
    Geometry::getAABB(VERTEX_off, min, max);
    Geometry::getAABBdims(min, max, center, radius);
    
    //set cam pos at corner of AABB
    camPos = max;

    octreeRoot = octree.InitOctree(center, radius, POINTDENSITY, NUM_OCTREE_LEVELS, LINE_CUTOFF);
    for (int i = 0; i < VERTEX_off.size(); i++)
    {
        Object *pNewObj = new Object;
        pNewObj->center = VERTEX_off.at(i);
        pNewObj->color = COLOR_off.at(i);
        pNewObj->radius = 0.0000000001;
        octree.InsertObject(octreeRoot, pNewObj, 0, NUM_OCTREE_LEVELS);
    }
    
    octree.CalculateColors(octreeRoot);
    
#ifdef EXPORT_OCTREE
    octree.exportWholeOctree(octreeRoot, octreeCounter, OCTREEPATH);
#endif
    
    printf("done.\n");
    
    createOctreeBuffers();
    
    return 1;
}

void onIdle() {
   
    //movement
    if (moveForward)
        camPos = camPos + camForward*MOVE_SPEED;
    if (moveBackward)
        camPos = camPos - camForward*MOVE_SPEED;
    if (moveLeft) {
        vec3 xp = cross(camForward, vec3(0,1,0));
        camPos = camPos - xp*MOVE_SPEED;
    }
    if (moveRight) {
        vec3 xp = cross(camForward, vec3(0,1,0));
        camPos = camPos + xp*MOVE_SPEED;
    }
    
    vec3 axis_y(0, 1, 0);
    vec3 axis_x(1, 0, 0);
    
    
    //camera
    camForward.x = 0; camForward.y = 0; camForward.z = 1;
    mat4 matRot1 = rotate(mat4(1.0),camYaw, axis_y);
    camForward = (vec4(camForward, 1.0) * matRot1).xyz();
    vec3 newAxis = (vec4(1.0,0.0,0.0,1.0) * matRot1).xyz();
    mat4 matRot2 = rotate(mat4(1.0), camPitch, newAxis);
    camForward = (vec4(camForward, 1.0) * matRot2).xyz();
    vec3 lookAtPos = camPos + camForward;
    
    // matrices
    //mat4 model = rotate(mat4(1.0f), -90.0f, axis_x);
    mat4 model = mat4();
    mat4 view = lookAt(camPos, lookAtPos, vec3(0.0, 1.0, 0.0));
    mat4 projection = perspective(45.0f, 1.0f*screen_width/screen_height, 0.1f, 1000.0f);
    mat4 ModelViewMatrix = view*model;
    mvp = projection * ModelViewMatrix;
    
    
    // extract frustum clipping
    float *proj;
    proj = value_ptr(projection);
    float *modelview;
    modelview = value_ptr(ModelViewMatrix);
    clipper.ExtractFrustum(proj, modelview);
    
    //get height of near plane
    float fovy = 45; // degrees
    int viewport[4];
    glGetIntegerv(GL_VIEWPORT,viewport);
    heightOfNearPlane = (float)abs(viewport[3]-viewport[1]) /
    (2*tan(0.5*fovy*PI/180.0));
    
    //Set uniforms that change each frame (e.g. heightOfNearPlane)
    glUseProgram(program);
    glUniformMatrix4fv(uniform_mvp, 1, GL_FALSE, value_ptr(mvp));
    glUniform1f(uniform_heightNearPlane, heightOfNearPlane);
    
    glUseProgram(program_centres);
    glUniformMatrix4fv(uniform_mvp_centres, 1, GL_FALSE, value_ptr(mvp));
    glUniform1f(uniform_heightNearPlane_centres, heightOfNearPlane);
    
    glUseProgram(program_octpoints);
    glUniformMatrix4fv(uniform_mvp_octpoints, 1, GL_FALSE, value_ptr(mvp));
    glUniform1f(uniform_heightNearPlane_octpoints, heightOfNearPlane);
    
    
    //do we need this once a frame??
    glEnable(GL_PROGRAM_POINT_SIZE_EXT);
    
    glutPostRedisplay();
}


void onDisplay()
{
    
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    
    
    if (showPoints) {
        glUseProgram(program);
        
        
        //Binding attributes to buffers
        
        glEnableVertexAttribArray(attribute_vertices);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_points_vertices);
        glVertexAttribPointer(
                              attribute_vertices, // attribute
                              3,                 // number of elements per vertex, here (x,y,z)
                              GL_FLOAT,          // the type of each element
                              GL_FALSE,          // take our values as-is
                              0,                 // no extra data between each position
                              0                  // offset of first element
                              );
        
        glEnableVertexAttribArray(attribute_colors);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_points_colors);
        glVertexAttribPointer(
                              attribute_colors, // attribute
                              3,                 // number of elements per vertex, here (x,y,z)
                              GL_FLOAT,          // the type of each element
                              GL_FALSE,          // take our values as-is
                              0,                 // no extra data between each position
                              0                  // offset of first element
                              );

        
        
        glUniform1f(uniform_pointSize, POINT_SIZE);
        glDrawArrays(GL_POINTS, 0, (GLsizei)VERTEX_off.size());
        glDisableVertexAttribArray(attribute_vertices);
        glDisableVertexAttribArray(attribute_colors);
    }

    
    if (showOctree) {
        
        glUseProgram(program_centres);
        glEnableVertexAttribArray(attribute_centres_vertices);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_centres_vertices);
        glVertexAttribPointer(
                              attribute_centres_vertices, // attribute
                              3,                 // number of elements per vertex, here (x,y,z)
                              GL_FLOAT,          // the type of each element
                              GL_FALSE,          // take our values as-is
                              0,                 // no extra data between each position
                              0                  // offset of first element
                              );
        
        glEnableVertexAttribArray(attribute_centres_colors);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_centres_colors);
        glVertexAttribPointer(
                              attribute_centres_colors, // attribute
                              3,                 // number of elements per vertex, here (x,y,z)
                              GL_FLOAT,          // the type of each element
                              GL_FALSE,          // take our values as-is
                              0,                 // no extra data between each position
                              0                  // offset of first element
                              );
        
        glEnableVertexAttribArray(attribute_centres_sizes);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_centres_sizes);
        glVertexAttribPointer(
                              attribute_centres_sizes, // attribute
                              1,                 // number of elements per vertex, here (x,y,z)
                              GL_FLOAT,          // the type of each element
                              GL_FALSE,          // take our values as-is
                              0,                 // no extra data between each position
                              0                  // offset of first element
                              );
        

        for (int i = 0; i < (int)VERTEX_octree.size(); i++) {
            if (ENABLED_octree[i])
                glDrawArrays(GL_POINTS, i, 1);
        }
        
        glDisableVertexAttribArray(attribute_centres_vertices);
        glDisableVertexAttribArray(attribute_centres_colors);
        glDisableVertexAttribArray(attribute_centres_sizes);
        
    }
    
    if (showOctPoints) {
        glUseProgram(program_octpoints);
        
        
        //Binding attributes to buffers
        
        glEnableVertexAttribArray(attribute_octpoints_vertices);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_octpoints_vertices);
        glVertexAttribPointer(
                              attribute_octpoints_vertices, // attribute
                              3,                 // number of elements per vertex, here (x,y,z)
                              GL_FLOAT,          // the type of each element
                              GL_FALSE,          // take our values as-is
                              0,                 // no extra data between each position
                              0                  // offset of first element
                              );
        
        glEnableVertexAttribArray(attribute_octpoints_colors);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_octpoints_colors);
        glVertexAttribPointer(
                              attribute_octpoints_colors, // attribute
                              3,                 // number of elements per vertex, here (x,y,z)
                              GL_FLOAT,          // the type of each element
                              GL_FALSE,          // take our values as-is
                              0,                 // no extra data between each position
                              0                  // offset of first element
                              );
        
        
        
        glUniform1f(uniform_pointSize_octpoints, POINT_SIZE);
 
        
        glDrawArrays(GL_POINTS, 0, (GLsizei)num_octpoints_in_frame);
        
        glDisableVertexAttribArray(attribute_octpoints_vertices);
        glDisableVertexAttribArray(attribute_octpoints_colors);
    }
    
    
    


    glutSwapBuffers();
    
}

void onReshape(int width, int height) {
    screen_width = width;
    screen_height = height;
    glViewport(0, 0, screen_width, screen_height);
}

void free_resources()
{
    glDeleteProgram(program);
    glDeleteBuffers(1, &vbo_points_vertices);
    glDeleteBuffers(1, &vbo_points_colors);
    
    glDeleteProgram(program_centres);
    glDeleteBuffers(1, &vbo_centres_vertices);
    glDeleteBuffers(1, &vbo_centres_colors);
    glDeleteBuffers(1, &vbo_centres_sizes);
}

//keyboard function for translations/rotations
void keyboard(int key, int x, int y)
{
    
    switch (key)
    {
        case 49:
            showPoints = true;
            showOctree = false;
            break;
        case 50:
            showOctPoints = !showOctPoints;
            break;
        case 51:
            showPoints = false;
            showOctree = true;
            break;
        case 't':
            //testOctreeFrustum();
            break;        
        case 119:
            moveForward = true;
            break;
        case 115:
            moveBackward = true;
            break;
        case 97:
            moveLeft = true;
            break;
        case 100:
            moveRight = true;
            break;
        case 27:
            printf("%f %f %f %f %f", camPos.x, camPos.y, camPos.z, camYaw, camPitch);
            exit(0);
    }
}

void keyboardUp(unsigned char key, int x, int y)
{
    switch (key)
    {
        case 119:
            moveForward = false;
            break;
        case 115:
            moveBackward = false;
            break;
        case 97:
            moveLeft = false;
            break;
        case 100:
            moveRight = false;
            break;
    }
}

void mouseButtons(int button, int state, int x, int y)
{
    if (state == GLUT_DOWN){
        mouseDown = true;
        mDownX = x; mDownY = y;
    }
    else mouseDown = false;
}

void mouseMove(int x, int y)
{
    if (mouseDown) {
        camYaw -= (mDownX-x)*MOUSE_SPEED;
        if (camYaw > 90) camYaw -= 360;
        if (camYaw < 0) camYaw += 360;
        camPitch += (mDownY-y)*MOUSE_SPEED;
        if (camPitch > 90) camPitch = 90;
        if (camPitch < -90) camPitch = -90;
        
        mDownX = x; mDownY = y;
    }
}

int main(int argc, char* argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA|GLUT_ALPHA|GLUT_DOUBLE|GLUT_DEPTH);
    glutInitWindowSize(screen_width, screen_height);
    glutCreateWindow("PointCloud - UPF - IMPART");
    //glutFullScreen();
    
    bool start = false;
    start = init_resources();
    
    if (start) {
   
        glutDisplayFunc(onDisplay);
        glutReshapeFunc(onReshape);
        glutSpecialFunc(keyboard);
        glutKeyboardUpFunc(keyboardUp);
        glutMouseFunc(mouseButtons);
        glutMotionFunc(mouseMove);
        glutIdleFunc(onIdle);
        glEnable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glutMainLoop();
    }
    
    free_resources();
    return 0;
}



/*
 * Add vertices to drawlist for a box defined by its centers and halfwidth
 *
void addBoxVerts(vec3 c, float hW, vector<vec3> &lineVerts, vector<vec3> &colors, vec3 lineColor) {
    //bottom square
    lineVerts.push_back(vec3(c.x-hW, c.y-hW, c.z-hW)); //1
    lineVerts.push_back(vec3(c.x-hW, c.y-hW, c.z+hW)); //2
    lineVerts.push_back(vec3(c.x-hW, c.y-hW, c.z+hW)); //2
    lineVerts.push_back(vec3(c.x+hW, c.y-hW, c.z+hW)); //3
    lineVerts.push_back(vec3(c.x+hW, c.y-hW, c.z+hW)); //3
    lineVerts.push_back(vec3(c.x+hW, c.y-hW, c.z-hW)); //4
    lineVerts.push_back(vec3(c.x+hW, c.y-hW, c.z-hW)); //4
    lineVerts.push_back(vec3(c.x-hW, c.y-hW, c.z-hW)); //1
    //line up then round again
    lineVerts.push_back(vec3(c.x-hW, c.y-hW, c.z-hW)); //1
    lineVerts.push_back(vec3(c.x-hW, c.y+hW, c.z-hW)); //5
    lineVerts.push_back(vec3(c.x-hW, c.y+hW, c.z-hW)); //5
    lineVerts.push_back(vec3(c.x-hW, c.y+hW, c.z+hW)); //6
    lineVerts.push_back(vec3(c.x-hW, c.y+hW, c.z+hW)); //6
    lineVerts.push_back(vec3(c.x+hW, c.y+hW, c.z+hW)); //7
    lineVerts.push_back(vec3(c.x+hW, c.y+hW, c.z+hW)); //7
    lineVerts.push_back(vec3(c.x+hW, c.y+hW, c.z-hW)); //8
    lineVerts.push_back(vec3(c.x+hW, c.y+hW, c.z-hW)); //8
    lineVerts.push_back(vec3(c.x-hW, c.y+hW, c.z-hW)); //5
    // go to 6 then down to 2
    lineVerts.push_back(vec3(c.x-hW, c.y+hW, c.z+hW)); //6
    lineVerts.push_back(vec3(c.x-hW, c.y-hW, c.z+hW)); //2
    // go to 3 then up to 7
    lineVerts.push_back(vec3(c.x+hW, c.y-hW, c.z+hW)); //3
    lineVerts.push_back(vec3(c.x+hW, c.y+hW, c.z+hW)); //7
    //go to 8 then down to 4
    lineVerts.push_back(vec3(c.x+hW, c.y+hW, c.z-hW)); //8
    lineVerts.push_back(vec3(c.x+hW, c.y-hW, c.z-hW)); //4
    
    //add colors
    for (int i = 0; i < 24; i++) {
        //colors.push_back(vec3(0.0,1.0,0.0));
        colors.push_back(lineColor);
    }
}*/

/*
void readObjFile(const char* filename, vector<vec3> & VERTEX, vector<vec3> & NORMALS, vector<vec2> & TEXTURES){
    
    // Obj parser: reads the obj file and creates the vectors VERTEX,NORMALS and TEXTURES with the values corresponding to the indices
    
    string line;
    string word;
    vector<vec3> NORMALS_TEMP, VERTEX_TEMP;
    vector<vec2> TEXTURE_TEMP;
    
    //Open the OBJ file
    ifstream f(filename);
    if(!f.is_open())
        printf("Error Opening the file");
    
    while (getline(f, line)) {
        if ((line[0] == '#') || (line == ""))
            word = "Don't repeat last word.";
        
        istringstream l(line);
        l >> word;
        
        if (word == "v") {
            //Vertices
            vec3 vertex_temp;
            sscanf (line.c_str(), "%*s %f %f %f", &vertex_temp.x, &vertex_temp.y, &vertex_temp.z);
            VERTEX_TEMP.push_back(vertex_temp);
        }
        
        else if (word == "vn") {
            //Normals
            vec3 normal_temp;
            sscanf (line.c_str(), "%*s %f %f %f", &normal_temp.x, &normal_temp.y, &normal_temp.z);
            NORMALS_TEMP.push_back(normal_temp);
        }
        
        else if (word == "vt") {
            //Textures
            vec2 texture_temp;
            sscanf (line.c_str(), "%*s %f %f", &texture_temp.x, &texture_temp.y);
            texture_temp.y = -texture_temp.y;
            TEXTURE_TEMP.push_back(texture_temp);
        }
        
        else if (word == "f"){
            //Faces -> Index by index, the vectors are filled with the values.
            GLushort vertex_indices[3], texture_indices[3], normals_indices[3];
            sscanf (line.c_str(), "%*s %hd/%hd/%hd %hd/%hd/%hd %hd/%hd/%hd", &vertex_indices[0], &texture_indices[0], &normals_indices[0], &vertex_indices[1], &texture_indices[1],&normals_indices[1], &vertex_indices[2], &texture_indices[2], &normals_indices[2]);
            VERTEX.push_back(VERTEX_TEMP[vertex_indices[0]-1]);
            VERTEX.push_back(VERTEX_TEMP[vertex_indices[1]-1]);
            VERTEX.push_back(VERTEX_TEMP[vertex_indices[2]-1]);
            NORMALS.push_back(NORMALS_TEMP[normals_indices[0]-1]);
            NORMALS.push_back(NORMALS_TEMP[normals_indices[1]-1]);
            NORMALS.push_back(NORMALS_TEMP[normals_indices[2]-1]);
            TEXTURES.push_back(TEXTURE_TEMP[texture_indices[0]-1]);
            TEXTURES.push_back(TEXTURE_TEMP[texture_indices[1]-1]);
            TEXTURES.push_back(TEXTURE_TEMP[texture_indices[2]-1]);
        }
    }
    
}
 */

//void clipperResetOctreeVisibility(Node *pTree) {
//
//    ENABLED_octree[pTree->nodeid] = false;
//
//    for (int i = 0; i < 8; i++) {
//        if (pTree->pChild[i] != NULL)
//            clipperResetOctreeVisibility(pTree->pChild[i]);
//    }
//}
//
//void clipperSetOctreeVisibility(Node *pTree) {
//
//    int hit = clipper.SphereInFrustum2(pTree->center.x, pTree->center.y, pTree->center.z, pTree->halfWidth);
//    if (hit == 0) return;
//    if (pTree->depth > tmpMaxDepth) tmpMaxDepth = pTree->depth;
//
//    vec2 screenMin, screenMax;
//    Geometry::projectAABB(pTree->center, pTree->halfWidth, mvp, screen_width, screen_height, screenMin, screenMax);
//    int AABBheight = screenMax.y-screenMin.y;
//    if (AABBheight > heightOfNearPlane/CELLSIZEFACTOR)
//    {
//
//        if (pTree->depth == NUM_OCTREE_LEVELS-1) {
//            ENABLED_octree[pTree->nodeid] = true;
//            return;
//        }
//        if (strcmp(pTree->name.c_str(), "r28567") == 0) {
//            int bob = 0;
//        }
//
//        for (int i = 0; i < 8; i++) {
//
//            if (pTree->pChild[i] != NULL) {
//                clipperSetOctreeVisibility(pTree->pChild[i]);
//
//            }
//            else if (octree_from_file) {
//                if (pTree->childMask[i] == true) {
//                    string childName = pTree->name + to_string(i+1);
//                    string childNameShort = pTree->name + to_string(i+1);
//                    childNameShort.erase(0,1);
//                    int iChildName = atoi(childNameShort.c_str());
//                    //get new filename
//                    string newFileName;
//                    bool fileFound = false;
//                    for (size_t i = 0; i < octreeIndices.size()-1; i++ ) {
//
//                        if (iChildName > octreeIndices[i] && iChildName < octreeIndices[i+1]) {
//                            newFileName = "r" + to_string(octreeIndices[i]) + ".oct";
//                            fileFound = true;
//                            break;
//                        }
//                    }
//                    if (fileFound) {
//                        octree.readOctreeFromFileToMap(octreeMap, OCTREEPATH + newFileName, 1);
//                        kbCounter += 430;
//                        printf("opened file %s; total kb: %d\n", newFileName.c_str(), kbCounter);
//
//
//                    }
//                    //set flag to enable all buffers
//                    octree_needs_updating = true;
//
//                    //quickly update ids and enabled buffer otherwise we crash
//                    octreeCounter = 0;
//                    getOctreeBufferSize(octreeRoot);
//                    vector<bool> newVector (octreeCounter);
//                    ENABLED_octree = newVector;
//
//                    //carry on setting visibilities
//                    clipperSetOctreeVisibility(pTree->pChild[i]);
//
//                    tmpCounter++;
//
//
//                }
//            }
//        }
//        if (showOctPoints) {
//            Object *pO;
//            pO = pTree->pObjList;
//            for (int i = 0; i < pTree->numObjects; i++) {
//                VERTEX_octpoints[num_octpoints_in_frame] = pO->center;
//                COLOR_octpoints[num_octpoints_in_frame] = pO->color;
//                pO = pO->pNextObject;
//                num_octpoints_in_frame++;
//            }
//        }
//    }
//    else
//        ENABLED_octree[pTree->nodeid] = true;
//
//
//}




//void testOctreeFrustum() {
//
//    tmpCounter = 0;
//    clipperResetOctreeVisibility(octreeRoot);
//
//    num_octpoints_in_frame = 0;
//    tmpCounter = 0;
//    tmpMaxDepth = 0;
//    clipperSetOctreeVisibility(octreeRoot);
//
//
//    if (octree_needs_updating) {
//        octreeCounter = 0;
//        getOctreeBufferSize(octreeRoot);
//        vector<vec3> newVector1 (octreeCounter);
//        vector<vec3> newVector2 (octreeCounter);
//        vector<float> newVector3 (octreeCounter);
//        vector<bool> newVector4 (octreeCounter);
//        VERTEX_octree = newVector1;
//        COLOR_octree = newVector2;
//        POINTSIZE_octree = newVector3;
//        ENABLED_octree = newVector4;
//
//        populateOctreeBuffers(octreeRoot);
//
//        glBindBuffer(GL_ARRAY_BUFFER, vbo_centres_vertices);
//        glBufferData(GL_ARRAY_BUFFER, VERTEX_octree.size() * sizeof(vec3), &VERTEX_octree[0], GL_STATIC_DRAW);
//
//        glBindBuffer(GL_ARRAY_BUFFER, vbo_centres_colors);
//        glBufferData(GL_ARRAY_BUFFER, COLOR_octree.size() * sizeof(vec3), &COLOR_octree[0], GL_STATIC_DRAW);
//
//        glBindBuffer(GL_ARRAY_BUFFER, vbo_centres_sizes);
//        glBufferData(GL_ARRAY_BUFFER, POINTSIZE_octree.size() * sizeof(float), &POINTSIZE_octree[0], GL_STATIC_DRAW);
//
//
//        octree_needs_updating = false;
//    }
//
//    //printf("%d\n", tmpCounter);
//    if (showOctPoints) {
//        glBindBuffer(GL_ARRAY_BUFFER, vbo_octpoints_vertices);
//        glBufferSubData(GL_ARRAY_BUFFER, 0, num_octpoints_in_frame * sizeof(vec3), &VERTEX_octpoints[0]);
//
//        glBindBuffer(GL_ARRAY_BUFFER, vbo_octpoints_colors);
//        glBufferSubData(GL_ARRAY_BUFFER, 0, num_octpoints_in_frame * sizeof(vec3), &COLOR_octpoints[0]);
//    }
//
//}

