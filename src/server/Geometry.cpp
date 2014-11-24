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

#include "Geometry.h"



/*
* Get AABBB for a set of points, pass ref to min and max vec3
*/
void Geometry::getAABB(vector<vec3> points, vec3 &min, vec3 &max) {
    float MAXVALUE = 100000;
    min.x = MAXVALUE; min.y = MAXVALUE; min.z = MAXVALUE;
    max.x = -MAXVALUE; max.y = -MAXVALUE; max.z = -MAXVALUE;
    vec3 currPoint;
    for (int i = 0; i < points.size(); i++)
    {
        currPoint = points.at(i);
        if (currPoint.x < min.x) min.x = currPoint.x;
        if (currPoint.y < min.y) min.y = currPoint.y;
        if (currPoint.z < min.z) min.z = currPoint.z;
        if (currPoint.x > max.x) max.x = currPoint.x;
        if (currPoint.y > max.y) max.y = currPoint.y;
        if (currPoint.z > max.z) max.z = currPoint.z;
    }
}

/*
 * Get center and radius of given AABB
 */
void Geometry::getAABBdims(vec3 min, vec3 max, vec3 &center, float &radius) {
    vec3 width = (max - min);
    center = min + width/2.0f;
    radius = -10000.0f;
    if (width.x > radius) radius = width.x;
    if (width.y > radius) radius = width.y;
    if (width.z > radius) radius = width.z;
    radius /= 2.0f;
}

void Geometry::getAABBpoints(vec3 c, float r, vector<vec3> &b) {
    b.push_back(vec3(c.x-r, c.y-r, c.z-r));
    b.push_back(vec3(c.x+r, c.y-r, c.z-r));
    b.push_back(vec3(c.x-r, c.y+r, c.z-r));
    b.push_back(vec3(c.x+r, c.y+r, c.z-r));
    
    b.push_back(vec3(c.x-r, c.y-r, c.z+r));
    b.push_back(vec3(c.x+r, c.y-r, c.z+r));
    b.push_back(vec3(c.x-r, c.y+r, c.z+r));
    b.push_back(vec3(c.x+r, c.y+r, c.z+r));
}

void Geometry::projectAABB(vec3 c, float r, mat4 mat_mvp, int screen_width, int screen_height, vec2 &screenMin, vec2 &screenMax) {
    vector<vec3> bounds;
    getAABBpoints(c, r, bounds);
    int maxX = -100000, maxY = -100000, minX = 100000, minY = 100000;
    for (int i = 0; i < 8; i++){
        vec4 p(bounds.at(i).x, bounds.at(i).y, bounds.at(i).z, 1.0);
        vec4 p_mvp = mat_mvp * p;
        vec4 p_mvp_norm = normalize(p_mvp);
        int winX = (int) ((( p_mvp_norm.x + 1 ) / 2.0) * screen_width);
        int winY = (int) ((( 1 - p_mvp_norm.y ) / 2.0) * screen_height);
        if (winX > maxX) maxX = winX;
        if (winX < minX) minX = winX;
        if (winY > maxY) maxY = winY;
        if (winY < minY) minY = winY;
    }
    screenMin = vec2(minX, minY);
    screenMax = vec2(maxX, maxY);
}
