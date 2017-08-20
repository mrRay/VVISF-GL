#include "VVGeom.hpp"




namespace VVGL
{


using namespace std;




Rect ResizeRect(const Rect & fitThisRect, const Rect & inThisRect, const SizingMode & sizingMode)	{
	Rect		returnMe;
	double		aAspect = fitThisRect.size.width / fitThisRect.size.height;
	double		bAspect = inThisRect.size.width / inThisRect.size.height;
	switch (sizingMode)	{
	case SizingMode_Fit:
		//	if the rect i'm trying to fit stuff *into* is wider than the rect i'm resizing
		if (bAspect > aAspect)	{
			returnMe.size.height = inThisRect.size.height;
			returnMe.size.width = returnMe.size.height * aAspect;
		}
		//	else if the rect i'm resizing is wider than the rect it's going into
		else if (bAspect < aAspect)	{
			returnMe.size.width = inThisRect.size.width;
			returnMe.size.height = returnMe.size.width / aAspect;
		}
		else	{
			returnMe.size.width = inThisRect.size.width;
			returnMe.size.height = inThisRect.size.height;
		}
		returnMe.origin.x = (inThisRect.size.width-returnMe.size.width)/2.0+inThisRect.origin.x;
		returnMe.origin.y = (inThisRect.size.height-returnMe.size.height)/2.0+inThisRect.origin.y;
		break;
	case SizingMode_Fill:
		//	if the rect i'm trying to fit stuff *into* is wider than the rect i'm resizing
		if (bAspect > aAspect)	{
			returnMe.size.width = inThisRect.size.width;
			returnMe.size.height = returnMe.size.width / aAspect;
		}
		//	else if the rect i'm resizing is wider than the rect it's going into
		else if (bAspect < aAspect)	{
			returnMe.size.height = inThisRect.size.height;
			returnMe.size.width = returnMe.size.height * aAspect;
		}
		else	{
			returnMe.size.width = inThisRect.size.width;
			returnMe.size.height = inThisRect.size.height;
		}
		returnMe.origin.x = (inThisRect.size.width-returnMe.size.width)/2.0+inThisRect.origin.x;
		returnMe.origin.y = (inThisRect.size.height-returnMe.size.height)/2.0+inThisRect.origin.y;
		break;
	case SizingMode_Stretch:
		returnMe = { inThisRect.origin.x, inThisRect.origin.y, inThisRect.size.width, inThisRect.size.height };
		break;
	case SizingMode_Copy:
		returnMe.size = { (double)(int)fitThisRect.size.width, (double)(int)fitThisRect.size.height };
		returnMe.origin.x = (double)(int)((inThisRect.size.width-returnMe.size.width)/2.0+inThisRect.origin.x);
		returnMe.origin.y = (double)(int)((inThisRect.size.height-returnMe.size.height)/2.0+inThisRect.origin.y);
		break;
	}
	return returnMe;
}




void GLBufferQuadPopulate(GLBufferQuadXY *b, Rect geoRect)	{
	if (b == nullptr)
		return;
	b->bl.geo[0] = MinX(geoRect);
	b->bl.geo[1] = MinY(geoRect);
	
	b->br.geo[0] = MaxX(geoRect);
	b->br.geo[1] = MinY(geoRect);
	
	b->tl.geo[0] = MinX(geoRect);
	b->tl.geo[1] = MaxY(geoRect);
	
	b->tr.geo[0] = MaxX(geoRect);
	b->tr.geo[1] = MaxY(geoRect);
}
void GLBufferQuadPopulate(GLBufferQuadXYST *b, Rect geoRect, Rect texRect, bool flippedTex)	{
	//cout << __FUNCTION__ << ", " << texRect << endl;
	if (b == nullptr)
		return;
	if (!flippedTex)	{
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
	else	{
		b->bl.geo[0] = MinX(geoRect);
		b->bl.geo[1] = MinY(geoRect);
		b->bl.tex[0] = MinX(texRect);
		b->bl.tex[1] = MaxY(texRect);
	
		b->br.geo[0] = MaxX(geoRect);
		b->br.geo[1] = MinY(geoRect);
		b->br.tex[0] = MaxX(texRect);
		b->br.tex[1] = MaxY(texRect);
	
		b->tl.geo[0] = MinX(geoRect);
		b->tl.geo[1] = MaxY(geoRect);
		b->tl.tex[0] = MinX(texRect);
		b->tl.tex[1] = MinY(texRect);
	
		b->tr.geo[0] = MaxX(geoRect);
		b->tr.geo[1] = MaxY(geoRect);
		b->tr.tex[0] = MaxX(texRect);
		b->tr.tex[1] = MinY(texRect);
	}
}
void GLBufferQuadPopulate(GLBufferQuadXYC *b, Rect geoRect, GLColor geoColor)	{
	b->bl.geo[0] = MinX(geoRect);
	b->bl.geo[1] = MinY(geoRect);
	b->bl.color = geoColor;
	
	b->br.geo[0] = MaxX(geoRect);
	b->br.geo[1] = MinY(geoRect);
	b->br.color = geoColor;
	
	b->tl.geo[0] = MinX(geoRect);
	b->tl.geo[1] = MaxY(geoRect);
	b->tl.color = geoColor;
	
	b->tr.geo[0] = MaxX(geoRect);
	b->tr.geo[1] = MaxY(geoRect);
	b->tr.color = geoColor;
}
void GLBufferQuadPopulate(GLBufferQuadXYZ *b, Rect geoRect)	{
	if (b == nullptr)
		return;
	b->bl.geo[0] = MinX(geoRect);
	b->bl.geo[1] = MinY(geoRect);
	b->bl.geo[2] = 0.;
	
	b->br.geo[0] = MaxX(geoRect);
	b->br.geo[1] = MinY(geoRect);
	b->br.geo[2] = 0.;
	
	b->tl.geo[0] = MinX(geoRect);
	b->tl.geo[1] = MaxY(geoRect);
	b->tl.geo[2] = 0.;
	
	b->tr.geo[0] = MaxX(geoRect);
	b->tr.geo[1] = MaxY(geoRect);
	b->tr.geo[2] = 0.;
}
void GLBufferQuadPopulate(GLBufferQuadXYZST *b, Rect geoRect, Rect texRect, bool flippedTex)	{
	if (b == nullptr)
		return;
	if (!flippedTex)	{
		b->bl.geo[0] = MinX(geoRect);
		b->bl.geo[1] = MinY(geoRect);
		b->bl.geo[2] = 0.;
		b->bl.tex[0] = MinX(texRect);
		b->bl.tex[1] = MinY(texRect);
	
		b->br.geo[0] = MaxX(geoRect);
		b->br.geo[1] = MinY(geoRect);
		b->br.geo[2] = 0.;
		b->br.tex[0] = MaxX(texRect);
		b->br.tex[1] = MinY(texRect);
	
		b->tl.geo[0] = MinX(geoRect);
		b->tl.geo[1] = MaxY(geoRect);
		b->tl.geo[2] = 0.;
		b->tl.tex[0] = MinX(texRect);
		b->tl.tex[1] = MaxY(texRect);
	
		b->tr.geo[0] = MaxX(geoRect);
		b->tr.geo[1] = MaxY(geoRect);
		b->tr.geo[2] = 0.;
		b->tr.tex[0] = MaxX(texRect);
		b->tr.tex[1] = MaxY(texRect);
	}
	else	{
		b->bl.geo[0] = MinX(geoRect);
		b->bl.geo[1] = MinY(geoRect);
		b->bl.geo[2] = 0.;
		b->bl.tex[0] = MinX(texRect);
		b->bl.tex[1] = MinY(texRect);
	
		b->br.geo[0] = MaxX(geoRect);
		b->br.geo[1] = MinY(geoRect);
		b->br.geo[2] = 0.;
		b->br.tex[0] = MaxX(texRect);
		b->br.tex[1] = MinY(texRect);
	
		b->tl.geo[0] = MinX(geoRect);
		b->tl.geo[1] = MaxY(geoRect);
		b->tl.geo[2] = 0.;
		b->tl.tex[0] = MinX(texRect);
		b->tl.tex[1] = MaxY(texRect);
	
		b->tr.geo[0] = MaxX(geoRect);
		b->tr.geo[1] = MaxY(geoRect);
		b->tr.geo[2] = 0.;
		b->tr.tex[0] = MaxX(texRect);
		b->tr.tex[1] = MaxY(texRect);
	}
}
void GLBufferQuadPopulate(GLBufferQuadXYZC *b, Rect geoRect, GLColor geoColor)	{
	b->bl.geo[0] = MinX(geoRect);
	b->bl.geo[1] = MinY(geoRect);
	b->bl.geo[2] = 0.;
	b->bl.color = geoColor;
	
	b->br.geo[0] = MaxX(geoRect);
	b->br.geo[1] = MinY(geoRect);
	b->br.geo[2] = 0.;
	b->br.color = geoColor;
	
	b->tl.geo[0] = MinX(geoRect);
	b->tl.geo[1] = MaxY(geoRect);
	b->tl.geo[2] = 0.;
	b->tl.color = geoColor;
	
	b->tr.geo[0] = MaxX(geoRect);
	b->tr.geo[1] = MaxY(geoRect);
	b->tr.geo[2] = 0.;
	b->tr.color = geoColor;
}




}
