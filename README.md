# Progressive Pointclouds in WebGL
================================

Library for loading progressive point clouds in WebGL. It is formed of two parts, a server (offline) component and a web-based client. The offline component processes raw pointcloud data and outputs a series of files which are downloaded by the client in order to progressively render the pointcloud dataset

## Offline Component
### Compiling
A make file is included in the project and has been tested for MacOS and Ubuntu.

### Usage
./webpointcloud <infile.off> <output_directory> [-oct: Maximum depth of octree] [-epf: number of octree entries per file] [-ppn: Maximum number of points per octree node]

## Browser client
Simple WebGL renderer with included test dataset. All the interesting code is in pointclouds.js.

Demos: 

http://alunevans.info/apps/webgl/pointclouds/?dir=kamruddin

http://alunevans.info/apps/webgl/pointclouds/old/

