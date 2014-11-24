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

#ifndef PointCloudsGL_octree_h
#define PointCloudsGL_octree_h
#include <vector>
#include <string>
#include <queue>
#include <map>

#include <fstream>
#include <glm/glm.hpp>

struct Object {
    glm::vec3 center;
    glm::vec3 color;

    float radius;
    Object *pNextObject = NULL;
};

struct Node {
    glm::vec3 center;
    glm::vec3 color;
    std::string name;
    bool colorSet = false;
    int nodeid = -1;
    float halfWidth;
    int depth = 0;
    bool hasChildren = false;
    Node *pChild[8] = {NULL};
    int numObjects = 0;
    Object *pObjList = NULL;
    bool drawObjects = false;
    bool childMask[8] = {NULL};
    int indexinArray = -1;

};

class Octree {
private:

    std::string currentFileToWrite;
    std::ofstream currentStream;
    std::queue<Node*> breadthQueue;
    void exportNode(Node *pTree);
    int exportNodeToFile(Node *pTree);
    int exportNodeToBinFile(Node *pTree, std::string directory, std::vector<std::string> &indices);
    int exportNodeAndNameToBinFile(Node *pTree, std::string directory);
    int exportObjectToBinFile(Object *pObject);
    int exportOctreeArrayToBinFile(std::vector<Node*> octreeArr, std::string directory);
    int numberOfLines;
    int pointDensity;
    int maxTreeDepth;
    int maxDepth;
    int fileThreshold;
public:
    Octree();
    
    Node *InitOctree(glm::vec3 center, float halfWidth, int pointDens, int amaxTreeDepth, int lineCutoff);
    
    void InsertObject(Node *pTree, Object *pObject, int currDepth, int maxDepth);
    
    void CalculateColors(Node *pTree);
    
    void exportWholeOctree(Node *pTree, int octreeSize, std::string directory);
    void exportWholeOctreeArray(Node *pTree, int octreeSize, std::string directory);
    std::vector<int> readOctreeIndicesFromFile(std::string indexFile);
    Node* readOctreeFromFile(std::string rootNodeName, std::string rootFile);
    void readOctreeFromFileToMap(std::map<std::string, Node*>& octreeNodesMap, std::string rootFile, int findParent);
    
    std::vector<Node*> GetNodesAtDepth(Node *pTree, int desiredDepth);
    void getOctreeBufferSize(Node *pTree);
    
    void updatePointColorsFromOctree(Node *pTree, std::vector<glm::vec3>& COLOR);

};






#endif
