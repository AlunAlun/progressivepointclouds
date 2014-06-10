#ifndef _CLIPPER_
#define _CLIPPER_

#include <math.h>
#include <GLUT/glut.h>

class Clipper
{
public:
	float frustum[6][4];
	void ExtractFrustum(float* proj, float* modl);
	bool PointInFrustum( float x, float y, float z );
	bool SphereInFrustum( float x, float y, float z, float radius );
    int SphereInFrustum2( float x, float y, float z, float radius );

};

#endif