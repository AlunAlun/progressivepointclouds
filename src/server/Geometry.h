//
//  Geometry.h
//  PointCloudsGL
//
//  Created by Alun on 10/06/14.
//  Copyright (c) 2014 Alun. All rights reserved.
//

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
