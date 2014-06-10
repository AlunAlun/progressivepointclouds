//
//  3DLoaders.cpp
//  PointCloudsGL
//
//  Created by Alun on 10/06/14.
//  Copyright (c) 2014 Alun. All rights reserved.
//

#include "FileLoaders.h"
#include <fstream>
using namespace std;
using namespace glm;

void FileLoaders::readOFFFile(const char* filename, vector<vec3> & VERTEX, vector<vec3> & COLOR){
    string line;
    string word;
    
    //Open the OFF file
    ifstream f(filename);
    if(!f.is_open())
        printf("Error Opening the file");
    
    int counter = 0;
    int numVerts = 100000000;
    
    while (getline(f, line)) {
        //        if (counter+2 > numVerts)
        //            break;
        if (counter == 0)
            printf("Reading OFF file...");
        else if (counter == 1)
            sscanf (line.c_str(), "%d %*d %*d", &numVerts);
        else {
            
            int r = 0, g = 0, b = 0;
            float x = 0, y = 0, z = 0;
            
            sscanf (line.c_str(), "%f %f %f %d %d %d %*d", &x, &z, &y, &r,&g,&b);
            
            vec3 newVert(-x,y,z);
            vec3 newColor(r/255.0f, g/255.0f, b/255.0f);
            
            VERTEX.push_back(newVert);
            COLOR.push_back(newColor);
        }
        counter++;
    }
    printf("done.\n");
}

void FileLoaders::readXrgbFile(const char* filename, vector<vec3> & VERTEX, vector<vec3> & COLOR){
    string line;
    string word;
    
    //Open the OFF file
    ifstream f(filename);
    if(!f.is_open())
        printf("Error Opening the file");
    
    int counter = 0;
    int index = 0;
    
    while (getline(f, line)) {
        
        vec3 newVert;
        int r = 0, g = 0, b = 0;
        
        sscanf (line.c_str(), "%d %f %f %f %d %d %d %*d", &index, &newVert.x, &newVert.y, &newVert.z,&r,&g,&b);
        
        vec3 newColor(r, g, b);
        
        VERTEX.push_back(newVert);
        COLOR.push_back(newColor);
        
        counter++;
    }
    printf("done: %d\n", counter);
}


void FileLoaders::readPLYFile(const char* filename, vector<vec3> & VERTEX, vector<vec3> & COLOR){
    string line;
    string word;
    
    //Open the PLY file
    ifstream f(filename);
    if(!f.is_open())
        printf("Error Opening the file");
    
    int counter = 0;

    int numPoints = 0;
    unsigned long pos = 0;
    bool startReading = false;
    
    while (getline(f, line)) {
        pos = line.find("element vertex");
        if (pos != std::string::npos){
            char a, b;
            sscanf (line.c_str(), "%s %s %d", &a, &b, &numPoints);
            
        }
        if (line.compare("end_header") == 0)
            startReading = true;
        if (!startReading || numPoints == 0) continue;
        
        vec3 newVert;
        
        sscanf (line.c_str(), "%f %f %f", &newVert.x, &newVert.y, &newVert.z);
        
        vec3 newColor(1.0, 1.0, 1.0);
        
        VERTEX.push_back(newVert);
        COLOR.push_back(newColor);
        
        counter++;
        if (counter == numPoints) break;
    }
    printf("done: %d\n", counter);
}
