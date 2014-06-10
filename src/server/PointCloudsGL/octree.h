//
//  octree.h
//  PointCloudsGL
//
//  Created by Alun on 17/01/14.
//  Copyright (c) 2014 Alun. All rights reserved.
//

#ifndef PointCloudsGL_octree_h
#define PointCloudsGL_octree_h
#include <vector>
#include <string>
#include <queue>
#include <map>

#include <fstream>
#include "glm.hpp"

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
    glm::vec3 CalculateColors2(Node *pTree);
    std::vector<Node*> GetNodesAtDepth(Node *pTree, int desiredDepth);
    void exportWholeOctree(Node *pTree, int octreeSize, std::string directory);
    void exportWholeOctreeArray(Node *pTree, int octreeSize, std::string directory);
    std::vector<int> readOctreeIndicesFromFile(std::string indexFile);
    Node* readOctreeFromFile(std::string rootNodeName, std::string rootFile);
    void readOctreeFromFileToMap(std::map<std::string, Node*>& octreeNodesMap,
                                 std::string rootFile, int findParent);

};






#endif
