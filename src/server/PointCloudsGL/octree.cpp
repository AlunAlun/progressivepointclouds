#include "octree.h"
#include <math.h>



using namespace std;



vector<string> inline StringSplit(const string &source, const char *delimiter = " ", bool keepEmpty = false)
{
    vector<string> results;
    
    size_t prev = 0;
    size_t next = 0;
    
    while ((next = source.find_first_of(delimiter, prev)) != string::npos)
    {
        if (keepEmpty || (next - prev != 0))
        {
            results.push_back(source.substr(prev, next - prev));
        }
        prev = next + 1;
    }
    
    if (prev < source.size())
    {
        results.push_back(source.substr(prev));
    }
    
    return results;
}

Octree::Octree() {
    currentFileToWrite = "r.oct";
    numberOfLines = 1;
    pointDensity = 20;
    maxTreeDepth = 10;
    
}

Node* Octree::InitOctree(glm::vec3 center, float halfWidth, int pointDens, int amaxTreeDepth, int lineCutoff){

    Node *pNode = new Node;
    pNode->center = center;
    pNode->halfWidth = halfWidth;
    pNode->pObjList = NULL;
    pNode->name = "r";
    pNode->hasChildren = false;
    
    pointDensity = pointDens;
    fileThreshold = lineCutoff;
    maxTreeDepth = amaxTreeDepth;

    return pNode;

}

void Octree::InsertObject(Node *pTree, Object *pObject, int currDepth, int maxDepth){
    
    //cout << pTree->name << " " << pTree->numObjects << "\n";
    
    if (!pTree->hasChildren) { //we haven't split yet, so add object to this node
        
        pTree->numObjects++;

        pObject->pNextObject = pTree->pObjList;
        pTree->pObjList = pObject;

        //only spread if more than 8 objects and we are not at maxDepth
        //printf("point density threshold %d\n", pointDensity);
        if (pTree->numObjects > pointDensity && currDepth != maxDepth) {
            
            pTree->hasChildren = true;
            
            Object *pA = pTree->pObjList;

            //create vector of objects to pass down
            vector<Object*> objectsToPass;
            for (int i = 0; i < pTree->numObjects; i++) {
                objectsToPass.push_back(pA);
                pA = pA->pNextObject;
            }
            


            for (int i = 0; i < pTree->numObjects; i++) {
                objectsToPass.at(i)->pNextObject = NULL;
                InsertObject(pTree, objectsToPass.at(i), currDepth, maxDepth);
            }

            pTree->pObjList = NULL;
            pTree->numObjects = 0;
        }
        else return;
    }
    
    //else we add child
    
    //calculate child index
    int index = 0;
    float pObjectCenter[3];
    pObjectCenter[0]=pObject->center.x; pObjectCenter[1]=pObject->center.y; pObjectCenter[2]=pObject->center.z;
    float pTreeCenter[3];
    pTreeCenter[0]=pTree->center.x; pTreeCenter[1]=pTree->center.y; pTreeCenter[2]=pTree->center.z;

    for (int i = 0; i < 3; i++)
    {
        float delta = pObjectCenter[i] - pTreeCenter[i];
        if (delta > 0.0f) index |= (1 << i); //ZYX
    }
    
    
    //create new node if required
    if (!pTree->pChild[index]){
        
        glm::vec3 offset;
        float step = pTree->halfWidth * 0.5f;
        offset.x = ((index & 1) ? step : -step);
        offset.y = ((index & 2) ? step : -step);
        offset.z = ((index & 4) ? step : -step);
        
        Node *pNode = new Node;
        pNode->center = pTree->center + offset;
        pNode->halfWidth = step;
        pNode->pObjList = NULL;
        pNode->name = pTree->name;
        pNode->name += to_string(index+1);
        pNode->depth = currDepth+1;
        
        pTree->pChild[index] = pNode;
    }
    
    InsertObject(pTree->pChild[index], pObject, (currDepth+1), maxDepth);
}

void PopulateColors(Node *pTree)
{   glm::vec3 newColor(0.0,0.0,0.0);
    pTree->colorSet = false;
    int counter = 0;
    
    //now add color of objects in this node
    Object *pA = pTree->pObjList;
    for (int i = 0; i < pTree->numObjects; i++){
        newColor = newColor + pA->color;
        counter++;
        pA = pA->pNextObject;
    }

    
    //average colors
    if (counter > 0) {
        newColor = newColor / (float)counter;
        pTree->colorSet = true;
    }
    else {
        newColor = glm::vec3(1.0,1.0,1.0);
        pTree->colorSet = false;
    }
    
    //set color of this node
    pTree->color = newColor;
    
    //down the hole
    for (int i = 0; i < 8; i++) {
        if (pTree->pChild[i])
        {
            PopulateColors(pTree->pChild[i]);
        }
    }
}

void PropagateColors(Node *pTree) {
    glm::vec3 newColor(0.0,0.0,0.0);
    int counter = 0;

    for (int i = 0; i < 8; i++) {
        if (pTree->pChild[i]) {
            PropagateColors(pTree->pChild[i]);
            if (pTree->pChild[i]->colorSet) {
                newColor = newColor + pTree->pChild[i]->color;
                counter++;
            }
        }
    }
    if (!pTree->colorSet && counter > 0) {
        pTree->color = newColor / (float)counter;
        pTree->colorSet = true;
    }

}


void Octree::CalculateColors(Node * pTree) {
    PopulateColors(pTree);
    PropagateColors(pTree);
}

glm::vec3 Octree::CalculateColors2(Node *pTree){
    glm::vec3 newColor(0.0,0.0,0.0);
    int counter = 0;
    
    for (int i = 0; i < 8; i++) {
        if (pTree->pChild[i]) {
            newColor += CalculateColors2(pTree->pChild[i]);
            counter++;
        }
    }
    if (counter > 0) { //the node has children
        newColor /= (float)counter;
    }
    else { //node is bottom node
        Object *pA;
        for (pA = pTree->pObjList; pA; pA = pA->pNextObject) {
            newColor = newColor + pA->color;
            counter++;
        }
        if (counter > 0)
            newColor /= (float)counter;
        else
            newColor.x = 1.0;
    }
    
    return newColor;
}


/*
 * Adds the center point of all occupied nodes at a given octree depth
 */
void recursiveGetNodes(Node *pTree, vector<Node*> &nodesAtDepth, int desiredDepth, int currDepth) {
    if (desiredDepth == currDepth) {
        nodesAtDepth.push_back(pTree);
        return;
    }
    else {
        for (int i = 0; i < 8; i++)
        {
            if (pTree->pChild[i] != NULL) {
                recursiveGetNodes(pTree->pChild[i], nodesAtDepth, desiredDepth, (currDepth+1));
            }
        }
    }
}
vector<Node*> Octree::GetNodesAtDepth(Node *pTree, int desiredDepth) {
    vector<Node*> nodesAtDepth;
    
    recursiveGetNodes(pTree, nodesAtDepth, desiredDepth, 0);
    
    return nodesAtDepth;
    
}





int Octree::exportNodeToBinFile(Node *pTree, string directory, vector<string> &indices) {

    
    int toReturn = 0;
    
    
    if (pTree->numObjects > 0 && pTree->depth < maxTreeDepth) {

        Object *pA = pTree->pObjList;
        for (int i = 0; i < pTree->numObjects; i++) {
            
            
            int8_t iDepth = -1;
            
            float fArray[3] = { pA->center.x,
                pA->center.y,
                pA->center.z};
            
            uint8_t r = (int8_t)(pA->color.x*255.0);
            uint8_t g = (int8_t)(pA->color.y*255.0);
            uint8_t b = (int8_t)(pA->color.z*255.0);
            
            uint8_t iArray[3] = {
                r,
                g,
                b
            };
            char mask = 0; char empty = 0;

            currentStream.write((char *)&iDepth,sizeof(int8_t)); //1
            currentStream.write((char *)&fArray,sizeof(float)*3); //12
            currentStream.write((char *)&iArray,sizeof(uint8_t)*3); //3
            currentStream.write(&mask,sizeof(char)); // 1
            //currentStream.write(&empty,sizeof(char)*15); // 15
            
            numberOfLines++;
            if (numberOfLines % fileThreshold == 0){
                currentStream.close();
                currentFileToWrite = directory + pTree->name + "_" + to_string(i) + ".oct";
                currentStream.open(currentFileToWrite,std::ios_base::binary);
                indices.push_back(pTree->name + "_" + to_string(i));
            }
            pA = pA->pNextObject;
            
        }
    } else {

        int8_t iDepth = pTree->depth;
        
        float fArray[3] = { pTree->center.x,
                            pTree->center.y,
                            pTree->center.z};
    
        uint8_t r = (int8_t)(pTree->color.x*255.0);
        uint8_t g = (int8_t)(pTree->color.y*255.0);
        uint8_t b = (int8_t)(pTree->color.z*255.0);
    

    
        uint8_t iArray[3] = {
                            r,
                            g,
                            b
                            };
        char mask = 0; char empty = 0;
        for (int i = 0; i < 8; i++){
            if (pTree->pChild[i] != NULL)
                mask |= (1 << i);
        }
        
        currentStream.write((char *)&iDepth,sizeof(int8_t)); //1
        currentStream.write((char *)&fArray,sizeof(float)*3); //12
        currentStream.write((char *)&iArray,sizeof(uint8_t)*3); //3
        currentStream.write(&mask,sizeof(char)); // 1
        //currentStream.write(&empty,sizeof(char)*15); // 15

        numberOfLines++;
        
        if (numberOfLines % fileThreshold == 0){
            currentStream.close();
            currentFileToWrite = directory + pTree->name + ".oct";
            currentStream.open(currentFileToWrite,std::ios_base::binary);
            indices.push_back(pTree->name);
        }
    }
 
    return toReturn;
}


void Octree::exportWholeOctree(Node *pTree, int octreeSize, string directory) {
    
    currentFileToWrite = directory + "r.oct";
    maxDepth = 0;
    
    printf("maxTreeDepth %d, fileThreshold %d\n", maxTreeDepth, fileThreshold);
    
    currentStream.open (currentFileToWrite,std::ios_base::binary);
    Node* traverse;
    breadthQueue.push(pTree);
    
    vector<string> indices;
    
    vector<Node*> octreeArr;
    int octreeArrIndex = 0;
    
    while(!breadthQueue.empty()) {
        traverse = breadthQueue.front();
        breadthQueue.pop();
        traverse->indexinArray = octreeArrIndex;
        octreeArr.push_back(traverse);
        octreeArrIndex++;
        
        exportNodeToBinFile(traverse, directory, indices);

        
        for (int i = 0; i < 8; i++)
        {
            if (traverse->pChild[i] != NULL)
                breadthQueue.push(traverse->pChild[i]);
        }

    }
    printf("\n");
    printf("octreeArrIndex %d\n", octreeArrIndex);
    printf("size Array %lu\n", octreeArr.size());
    printf("numLines %d\n", numberOfLines);
    
    
    
    currentStream.close();
    
    currentFileToWrite = directory + "index.oct";
    currentStream.open (currentFileToWrite);
    currentStream << pTree->halfWidth << "\n";
    currentStream << numberOfLines << "\n";
  
    for (size_t i = 0; i < indices.size(); i++) {
        currentStream << indices.at(i) << "\n";
    }

    currentStream.close();
    
}


int Octree::exportOctreeArrayToBinFile(vector<Node*> octreeArr, string directory) {
    
    
    
    vector<string> indices;
    
    currentFileToWrite = directory + "r.oct";
    currentStream.open (currentFileToWrite,std::ios_base::binary);
    
    int cnt = 0;
    
    Node *currNode;
    for (int i = 0; i < octreeArr.size(); i++) {
        
        cnt++;
        currNode = octreeArr[i];
        
        if (cnt > fileThreshold){
            cnt = 0;
            currentStream.close();
            currentFileToWrite = directory + to_string(i) + ".oct";
            currentStream.open(currentFileToWrite,std::ios_base::binary);
            indices.push_back(to_string(i));
        }
        
        
        uint8_t iDepth = currNode->depth;
        int32_t iArrIndex = i;
        
        float fArray[6] = { currNode->center.x,
            currNode->center.y,
            currNode->center.z,
            currNode->color.x,
            currNode->color.y,
            currNode->color.z,
        };
        
        int32_t iArray[8] = {0, 0, 0, 0, 0, 0, 0, 0};
        char mask = 0; char empty = 0;
        for (int i = 0; i < 8; i++){
            if (currNode->pChild[i] != NULL){
                mask |= (1 << i);
                iArray[i] = currNode->pChild[i]->indexinArray;
            }
        }
        
        currentStream.write((char *)&iDepth,sizeof(uint8_t)); //1
        currentStream.write((char *)&iArrIndex,sizeof(int32_t));//4
        currentStream.write((char *)&fArray,sizeof(float)*6); //24
        currentStream.write(&mask,sizeof(char));//1
        currentStream.write(&empty,sizeof(char)*2);//2
        currentStream.write((char *)&iArray, sizeof(int32_t)*8);//32
        
        //write points
        if (currNode->numObjects > 0 && currNode->depth < 10) {
            Object *pObject = currNode->pObjList;
            for (int j = 0; j < currNode->numObjects; j++)
            {
                cnt++;
                float fArray[6] = {
                    pObject->center.x,
                    pObject->center.y,
                    pObject->center.z,
                    pObject->color.x,
                    pObject->color.y,
                    pObject->color.z,
                };
                pObject = pObject->pNextObject;
                
                uint8_t iDepth = 99;
                currentStream.write((char *)&iDepth,sizeof(uint8_t)); //1
                currentStream.write((char *)&iArrIndex,sizeof(int32_t));//4
                currentStream.write((char *)&fArray,sizeof(float)*6);//24
                currentStream.write(&empty,sizeof(char)*35);//35
            }
        }
    }
    currentStream.close();
    
    
    currentFileToWrite = directory + "index.oct";
    currentStream.open (currentFileToWrite);
    currentStream << octreeArr[0]->halfWidth << "\n";
    currentStream << octreeArr.size() << "\n";
    
    for (size_t i = 0; i < indices.size(); i++) {
        currentStream << indices.at(i) << "\n";
    }
    
    currentStream.close();
    
    return 1;
}




void Octree::exportWholeOctreeArray(Node *pTree, int octreeSize, string directory) {

    
    printf("maxTreeDepth %d, fileThreshold %d\n", maxTreeDepth, fileThreshold);
    

    Node* traverse;
    breadthQueue.push(pTree);
    
    vector<string> indices;
    
    vector<Node*> octreeArr;
    int octreeArrIndex = 0;
    
    while(!breadthQueue.empty()) {
        traverse = breadthQueue.front();
        breadthQueue.pop();
        traverse->indexinArray = octreeArrIndex;
        octreeArr.push_back(traverse);
        octreeArrIndex++;
    
        for (int i = 0; i < 8; i++)
        {
            if (traverse->pChild[i] != NULL)
                breadthQueue.push(traverse->pChild[i]);
        }
        
    }

    exportOctreeArrayToBinFile(octreeArr, directory);
    
}
















vector<int> Octree::readOctreeIndicesFromFile(std::string indexFile) {
    vector<int> indices;
    string line;
    ifstream myfile (indexFile);
    if (myfile.is_open()) {
        while ( getline (myfile,line) )
        {
            line.erase(0,1);
            indices.push_back(atoi(line.c_str()));
        }
        myfile.close();
    }
    
    return indices;
}



Node* Octree::readOctreeFromFile(string rootNodeName, string rootFile) {

    map <string, string> octreeChildrenMap;

    map<string, Node*> octreeNodesMap;
    
    
    string line;
    ifstream myfile (rootFile);
    if (myfile.is_open()) {
        while ( getline (myfile,line) )
        {
            if (line[0] == 'r') {
            
                vector<string> split = StringSplit(line);
                octreeChildrenMap[split.at(0)] = split.at(8);
                Node *pNode = new Node;
                pNode->center = glm::vec3( (float)::atof(split.at(1).c_str()),
                                           (float)::atof(split.at(2).c_str()),
                                           (float)::atof(split.at(3).c_str())
                                         );
                pNode->halfWidth = (float)::atof(split.at(4).c_str());
                pNode->color = glm::vec3( (float)::atof(split.at(5).c_str()),
                                        (float)::atof(split.at(6).c_str()),
                                        (float)::atof(split.at(7).c_str())
                                        );
                
                pNode->pObjList = NULL;
                pNode->name = split.at(0);
                pNode->depth = (int)split.at(0).length()-1;
                pNode->colorSet = true;
                
                string children = split.at(8);
                for(string::size_type i = 0; i < children.size(); ++i) {
                    if (children[i] == '1')
                        pNode->childMask[i] = true;
                    else
                        pNode->childMask[i] = false;
                }
                
                octreeNodesMap[pNode->name] = pNode;
            }
            
        }
        myfile.close();
    }
    
    map<string, string>::const_iterator itr;
    for (itr = octreeChildrenMap.begin(); itr != octreeChildrenMap.end(); ++itr) {
        Node* currNode = octreeNodesMap[(*itr).first];
        string str = (*itr).second;
        string newNodeName;
        for(string::size_type i = 0; i < str.size(); ++i) {
                        if (str[i] == '1') {
                newNodeName = currNode->name + to_string(i+1);
                currNode->pChild[i] = octreeNodesMap[newNodeName];
            }
        }
        
        //cout << "\n";
    }
    
    return octreeNodesMap[rootNodeName];
}

void Octree::readOctreeFromFileToMap(map<string, Node*>& octreeNodesMap, string rootFile, int findParents) {
    
    map <string, string> octreeChildrenMap;
    
    //map<string, Node*> octreeNodesMap;
    
    
    string line;
    ifstream myfile (rootFile);
    if (myfile.is_open()) {
        while ( getline (myfile,line) )
        {
            if (line[0] == 'r') {
                
                vector<string> split = StringSplit(line);
                octreeChildrenMap[split.at(0)] = split.at(8);
                Node *pNode = new Node;
                pNode->center = glm::vec3( (float)::atof(split.at(1).c_str()),
                                          (float)::atof(split.at(2).c_str()),
                                          (float)::atof(split.at(3).c_str())
                                          );
                pNode->halfWidth = (float)::atof(split.at(4).c_str());
                pNode->color = glm::vec3( (float)::atof(split.at(5).c_str()),
                                         (float)::atof(split.at(6).c_str()),
                                         (float)::atof(split.at(7).c_str())
                                         );
                
                pNode->pObjList = NULL;
                pNode->name = split.at(0);
                pNode->depth = (int)split.at(0).length()-1;
                pNode->colorSet = true;
                
                string children = split.at(8);
                for(string::size_type i = 0; i < children.size(); ++i) {
                    if (children[i] == '1')
                        pNode->childMask[i] = true;
                    else
                        pNode->childMask[i] = false;
                }
                
                octreeNodesMap[pNode->name] = pNode;
                if (findParents) {
                    //get node parent name
                    string parentName = pNode->name;
                    parentName.erase(parentName.length()-1, 1);
                    string childID = pNode->name;
                    childID.erase(0, childID.length()-1);
                    int iChildID = atoi(childID.c_str())-1;
                    try {
                    octreeNodesMap[parentName]->pChild[iChildID] = pNode;
                    }
                    catch(int e){
                        printf("Couldn't find parent. Ex: %d\n",e);
                    }

                }
            }
            
        }
        myfile.close();
    }
    // search for children if they exist
    map<string, string>::const_iterator itr;
    for (itr = octreeChildrenMap.begin(); itr != octreeChildrenMap.end(); ++itr) {
        Node* currNode = octreeNodesMap[(*itr).first];
        string str = (*itr).second;
        string newNodeName;
        for(string::size_type i = 0; i < str.size(); ++i) {
            if (str[i] == '1') {
                newNodeName = currNode->name + to_string(i+1);
                currNode->pChild[i] = octreeNodesMap[newNodeName];
            }
        }
        
        //cout << "\n";
    }
    

    

}

