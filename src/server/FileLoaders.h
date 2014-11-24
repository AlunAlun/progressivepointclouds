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

#ifndef __PointCloudsGL___DLoaders__
#define __PointCloudsGL___DLoaders__

#include <iostream>
#include <vector>
#include <glm/glm.hpp>

class FileLoaders {
public:
    static int readOFFFile(const char* filename, std::vector<glm::vec3> & VERTEX, std::vector<glm::vec3> & COLOR);
    static int readXrgbFile(const char* filename, std::vector<glm::vec3> & VERTEX, std::vector<glm::vec3> & COLOR);
    static int readPLYFile(const char* filename, std::vector<glm::vec3> & VERTEX, std::vector<glm::vec3> & COLOR);
    
};

#endif /* defined(__PointCloudsGL___DLoaders__) */
