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

#ifndef __PointCloudsGL__Geometry__
#define __PointCloudsGL__Geometry__

#include <iostream>
#include <vector>
#include <glm/glm.hpp>
using namespace std;
using namespace glm;

class Geometry {
public:
    static void getAABB(vector<vec3> points, vec3 &min, vec3 &max);
    static void getAABBdims(vec3 min, vec3 max, vec3 &center, float &radius);
    static void getAABBpoints(vec3 c, float r, vector<vec3> &b);
    static void projectAABB(vec3 c, float r, mat4 mat_mvp, int screen_width, int screen_height, vec2 &screenMin, vec2 &screenMax);
};

#endif /* defined(__PointCloudsGL__Geometry__) */
