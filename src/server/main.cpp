//
//  main.cpp

#include <iostream>
#include <glm/glm.hpp>



#include <cstdlib>
#include <string>
#include <cstring>
#include <vector>
#include <math.h>
#include <unistd.h>

#define GLM_SWIZZLE
#include <glm/glm.hpp>

#include "octree.h"
#include "FileLoaders.h"
#include "Geometry.h"


using namespace std;
using namespace glm;


int NUM_OCTREE_LEVELS = 8;
int LINE_CUTOFF = 5000;
int POINTDENSITY = 20;


// Converts degrees to radians.
#define degToRad(angleDegrees) (angleDegrees * M_PI / 180.0)

// Converts radians to degrees.
#define radToDeg(angleRadians) (angleRadians * 180.0 / M_PI)


vector<vec3> VERTEX_off, COLOR_off;

//octree
Octree octree;
Node *octreeRoot;
int octreeCounter = 0;




int main(int argc, char* argv[]) {
    

    
    //get current working directory
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    string path = cwd;

    if (argc < 3) {
        // Tell the user how to run the program
        std::cerr << "Usage: " << argv[0] << " <infile.off> <output_directory> [-oct: Maximum depth of octree] [-epf: number of octree entries per file] [-ppn: Maximum number of points per octree node" << std::endl;
        return 1;
    }
    else {
        for (int i = 3; i < argc; i++) {
            if (i + 1 < argc) {// Check that we haven't finished parsing already
                if (strcmp(argv[i], "-oct") == 0 ) {
                    NUM_OCTREE_LEVELS = atoi(argv[i + 1]);
                }
                else if (strcmp(argv[i],"-epf") == 0) {
                    LINE_CUTOFF = atoi(argv[i + 1]);
                }
                else if (strcmp(argv[i],"-ppn") == 0) {
                    POINTDENSITY = atoi(argv[i + 1]);
                }
                else if (strcmp(argv[i],"-h") == 0) {
                    std::cerr << "Usage: " << argv[0] << " <infile.off> <output_directory> [-oct: Maximum depth of octree] [-epf: number of octree entries per file] [-ppn: Maximum number of points per octree node" << std::endl;
                    return 1;
                }
            }
        }
    }
    printf("Progressive pointClouds input file generator. v0.3\n");
    printf("See http://alunvans.info/progressivepointclouds/ for more info\n");
    
    string INFILEPATH = argv[1];
    string OCTREEPATH = argv[2];
    const string startChar = "/";
    if (INFILEPATH.compare(0, startChar.length(), startChar)!=0) {
        //path is relative
        INFILEPATH = path + "/" + INFILEPATH;
    }
    if (OCTREEPATH.compare(0, startChar.length(), startChar)!=0) {
        //path is relative
        OCTREEPATH = path + "/" + OCTREEPATH;
    }
    
    if (OCTREEPATH.compare(OCTREEPATH.length()-1, startChar.length(), startChar)!=0) {
        //path doesn't have final slash
        OCTREEPATH += "/";
    }
    
    printf("\n   Reading file %s\n", INFILEPATH.c_str());
    printf("   Outputting to path %s\n", OCTREEPATH.c_str());
    printf("   Octree depth: %d.\n   Number of entries per file: %d.\n   Max points per entry: %d\n", NUM_OCTREE_LEVELS, LINE_CUTOFF, POINTDENSITY);
    
    std::string::size_type idx;
    std::string extension;
    idx = INFILEPATH.rfind('.');
    if(idx != std::string::npos)
        extension = INFILEPATH.substr(idx+1);
    int fileread = 0;
    if (extension == "xyzrgba")
        fileread = FileLoaders::readXrgbFile(INFILEPATH.c_str(), VERTEX_off, COLOR_off);
    else if (extension == "off")
        fileread = FileLoaders::readOFFFile(INFILEPATH.c_str(), VERTEX_off, COLOR_off);
    else if (extension == "ply")
        fileread = FileLoaders::readPLYFile(INFILEPATH.c_str(), VERTEX_off, COLOR_off);
    
    if (fileread == 0) {
        std::cerr << "Error opening filename " << INFILEPATH << "\n";
        return 0;
    }
    
    printf("\nBuilding Octree...");
    vec3 min, max, center;
    float radius;
    Geometry::getAABB(VERTEX_off, min, max);
    Geometry::getAABBdims(min, max, center, radius);
    
    octreeRoot = octree.InitOctree(center, radius, POINTDENSITY, NUM_OCTREE_LEVELS, LINE_CUTOFF);
    for (int i = 0; i < VERTEX_off.size(); i++)
    {
        Object *pNewObj = new Object;
        pNewObj->center = VERTEX_off.at(i);
        pNewObj->color = COLOR_off.at(i);
        pNewObj->radius = 0.0000000001;
        octree.InsertObject(octreeRoot, pNewObj, 0, NUM_OCTREE_LEVELS);
    }
    
    octree.CalculateColors(octreeRoot);

    octree.exportWholeOctree(octreeRoot, octreeCounter, OCTREEPATH);

    
    printf("done.\n");
    return 0;
}


