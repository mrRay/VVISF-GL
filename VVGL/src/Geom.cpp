#include "Geom.hpp"




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









}
