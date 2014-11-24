// Progressive Pointcloud visualisation with WebGL - server application

// Copyright 2014 by Alun Evans <alun@alunthomasevans.co.uk>
// https://github.com/AlunAlun/progressivepointclouds

// This file is part of the "Progressive Pointclouds for WebGL" application.

// "Progressive Pointclouds for WebGL" is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// "Progressive Pointclouds for WebGL" is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with "Progressive Pointclouds for WebGL".  If not, see <http://www.gnu.org/licenses/>.

#include "FileLoaders.h"
#include <fstream>
using namespace std;
using namespace glm;

int FileLoaders::readOFFFile(const char* filename, vector<vec3> & VERTEX, vector<vec3> & COLOR){
    string line;
    string word;
    
    //Open the OFF file
    ifstream f(filename);
    if(!f.is_open()){
        return 0;
    }
    
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
            
            sscanf (line.c_str(), "%f %f %f %d %d %d %*d", &x, &y, &z, &r,&g,&b);
            
            vec3 newVert(-x,y,z);
            vec3 newColor(r/255.0f, g/255.0f, b/255.0f);
            
            VERTEX.push_back(newVert);
            COLOR.push_back(newColor);
        }
        counter++;
    }
    printf("done.\n");
    return 1;
}

int FileLoaders::readXrgbFile(const char* filename, vector<vec3> & VERTEX, vector<vec3> & COLOR){
    string line;
    string word;
    
    //Open the OFF file
    ifstream f(filename);
    if(!f.is_open()){

        return 0;
    }
    
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
    return 1;
}


int FileLoaders::readPLYFile(const char* filename, vector<vec3> & VERTEX, vector<vec3> & COLOR){
    string line;
    string word;
    
    //Open the PLY file
    ifstream f(filename);
    if(!f.is_open()){

        return 0;
    }
    
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
        int r = 255, g = 255, b = 255;
        sscanf (line.c_str(), "%f %f %f %d %d %d", &newVert.x, &newVert.y, &newVert.z,&r,&g,&b);
        
        vec3 newColor(r/255.0, g/255.0, b/255.0);
        
        VERTEX.push_back(newVert);
        COLOR.push_back(newColor);
        
        counter++;
        if (counter == numPoints) break;
    }
    printf("done: %d\n", counter);
    return 1;
}
