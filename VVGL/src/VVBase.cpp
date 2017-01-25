#include "VVBase.hpp"




namespace VVGL
{


using namespace std;




/*	========================================	*/
#pragma mark --------------------- base functions


void GLBufferQuadPopulate(GLBufferQuad *b, Rect geoRect, Rect texRect)	{
	if (b == nullptr)
		return;
	b->bl.geo[0] = MinX(geoRect);
	b->bl.geo[1] = MinY(geoRect);
	b->bl.tex[0] = MinX(texRect);
	b->bl.tex[1] = MinY(texRect);
	
	b->br.geo[0] = MaxX(geoRect);
	b->br.geo[1] = MinY(geoRect);
	b->br.tex[0] = MaxX(texRect);
	b->br.tex[1] = MinY(texRect);
	
	b->tl.geo[0] = MinX(geoRect);
	b->tl.geo[1] = MaxY(geoRect);
	b->tl.tex[0] = MinX(texRect);
	b->tl.tex[1] = MaxY(texRect);
	
	b->tr.geo[0] = MaxX(geoRect);
	b->tr.geo[1] = MaxY(geoRect);
	b->tr.tex[0] = MaxX(texRect);
	b->tr.tex[1] = MaxY(texRect);
}










}
