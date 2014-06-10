//
//  3DLoaders.h
//  PointCloudsGL
//
//  Created by Alun on 10/06/14.
//  Copyright (c) 2014 Alun. All rights reserved.
//

#ifndef __PointCloudsGL___DLoaders__
#define __PointCloudsGL___DLoaders__

#include <iostream>
#include <vector>
#include "glm.hpp"

class FileLoaders {
public:
    static void readOFFFile(const char* filename, std::vector<glm::vec3> & VERTEX, std::vector<glm::vec3> & COLOR);
    static void readXrgbFile(const char* filename, std::vector<glm::vec3> & VERTEX, std::vector<glm::vec3> & COLOR);
    static void readPLYFile(const char* filename, std::vector<glm::vec3> & VERTEX, std::vector<glm::vec3> & COLOR);
    
};

#endif /* defined(__PointCloudsGL___DLoaders__) */
