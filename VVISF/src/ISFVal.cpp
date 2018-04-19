#include "ISFVal.hpp"
#include "ISFStringUtils.hpp"

//#include "GLBuffer.hpp"




namespace VVISF
{


/*	========================================	*/
#pragma mark --------------------- static functions


string ISFValTypeString(const ISFValType & n)	{
	switch (n)	{
	case ISFValType_None:
		return string("Null");
	case ISFValType_Event:
		return string("Event");
	case ISFValType_Bool:
		return string("Bool");
	case ISFValType_Long:
		return string("Long");
	case ISFValType_Float:
		return string("Float");
	case ISFValType_Point2D:
		return string("Point2D");
	case ISFValType_Color:
		return string("Color");
	case ISFValType_Cube:
		return string("Cube");
	case ISFValType_Image:
		return string("Image");
	case ISFValType_Audio:
		return string("Audio");
	case ISFValType_AudioFFT:
		return string("AudioFFT");
	}
	return string("?");
}


/*	========================================	*/
#pragma mark --------------------- constructor/destructor


ISFVal::ISFVal()	{
	//cout << __PRETTY_FUNCTION__ << endl;
	type = ISFValType_None;
	val.boolVal = false;
}
ISFVal::ISFVal(const ISFValType & inType, const bool & inBool) : type(inType)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	val.boolVal = inBool;
}
ISFVal::ISFVal(const ISFValType & inType, const int32_t & inLong) : type(inType)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	val.longVal = inLong;
}
ISFVal::ISFVal(const ISFValType & inType, const double & inFloat) : type(inType)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	val.floatVal = inFloat;
}
ISFVal::ISFVal(const ISFValType & inType, const double & inX, const double & inY) : type(inType)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	val.pointVal[0] = inX;
	val.pointVal[1] = inY;
}
ISFVal::ISFVal(const ISFValType & inType, const double * inBuffer, const size_t inSizeToCopy) : type(inType)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	double		*rPtr = const_cast<double*>(inBuffer);
	for (size_t i=0; i<inSizeToCopy; ++i)	{
		val.pointVal[i] = *rPtr;
		++rPtr;
	}
}
ISFVal::ISFVal(const ISFValType & inType, const double & inR, const double & inG, const double & inB, const double & inA) : type(inType)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	val.colorVal[0] = inR;
	val.colorVal[1] = inG;
	val.colorVal[2] = inB;
	val.colorVal[3] = inA;
}
ISFVal::ISFVal(const ISFValType & inType, const GLBufferRef & inImage) : type(inType)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	imageVal = inImage;
}


/*	========================================	*/
#pragma mark --------------------- methods


double ISFVal::getDoubleVal() const	{
	double		returnMe = 0.;
	switch (type)	{
	case ISFValType_None:
		break;
	case ISFValType_Event:
	case ISFValType_Bool:
		returnMe = (val.boolVal) ? 1. : 0.;
		break;
	case ISFValType_Long:
		returnMe = (double)val.longVal;
		break;
	case ISFValType_Float:
		returnMe = (double)val.floatVal;
		break;
	case ISFValType_Point2D:
	case ISFValType_Color:
	case ISFValType_Cube:
	case ISFValType_Image:
	case ISFValType_Audio:
	case ISFValType_AudioFFT:
		break;
	}
	return returnMe;
}
bool ISFVal::getBoolVal() const	{
	bool		returnMe = false;
	switch (type)	{
	case ISFValType_None:
		returnMe = false;
		break;
	case ISFValType_Event:
		returnMe = val.boolVal;
		break;
	case ISFValType_Bool:
		returnMe = val.boolVal;
		break;
	case ISFValType_Long:
		returnMe = (val.longVal>0) ? true : false;
		break;
	case ISFValType_Float:
		returnMe = (val.floatVal>0.) ? true : false;
		break;
	case ISFValType_Point2D:
	case ISFValType_Color:
	case ISFValType_Cube:
	case ISFValType_Image:
	case ISFValType_Audio:
	case ISFValType_AudioFFT:
		break;
	}
	return returnMe;
}
int32_t ISFVal::getLongVal() const	{
	int32_t		returnMe = 0;
	switch (type)	{
	case ISFValType_None:
		returnMe = 0;
		break;
	case ISFValType_Event:
	case ISFValType_Bool:
		returnMe = (val.boolVal) ? 1 : 0;
		break;
	case ISFValType_Long:
		returnMe = val.longVal;
		break;
	case ISFValType_Float:
		returnMe = (int32_t)val.floatVal;
		break;
	case ISFValType_Point2D:
	case ISFValType_Color:
	case ISFValType_Cube:
	case ISFValType_Image:
	case ISFValType_Audio:
	case ISFValType_AudioFFT:
		break;
	}
	return returnMe;
}
GLBufferRef ISFVal::getImageBuffer() const	{
	//cout << __FUNCTION__ << ", self is " << this << endl;
	switch (type)	{
	case ISFValType_None:
		break;
	case ISFValType_Event:
	case ISFValType_Bool:
	case ISFValType_Long:
	case ISFValType_Float:
	case ISFValType_Point2D:
	case ISFValType_Color:
		return nullptr;
		break;
	case ISFValType_Cube:
	case ISFValType_Image:
	case ISFValType_Audio:
	case ISFValType_AudioFFT:
		//cout << "\timageVal was " << imageVal << endl;
		return imageVal;
		break;
	}
	return nullptr;
}
void ISFVal::setImageBuffer(const GLBufferRef & n)	{
	//cout << __FUNCTION__ << ", self is " << this << endl;
	imageVal=n;
	//cout << "\timageVal is now " << imageVal << endl;
}

string ISFVal::getTypeString() const	{
	return ISFValTypeString(type);
}
string ISFVal::getValString() const	{
	switch (type)	{
	case ISFValType_None:
		return string("None");
	case ISFValType_Event:
		return string("Event/None");
	case ISFValType_Bool:	{
		return (val.boolVal) ? string("true") : string("false");
	}
	case ISFValType_Long:	{
		return FmtString("%d",val.longVal);
	}
	case ISFValType_Float:	{
		return FmtString("%f",val.floatVal);
	}
	case ISFValType_Point2D:	{
		return FmtString("(%0.2f, %0.2f)",val.pointVal[0],val.pointVal[1]);
	}
	case ISFValType_Color:	{
		return FmtString("{%0.2f, %0.2f, %0.2f, %0.2f}",val.colorVal[0],val.colorVal[1],val.colorVal[2],val.colorVal[3]);
	}
	case ISFValType_Cube:
	case ISFValType_Image:
	case ISFValType_Audio:
	case ISFValType_AudioFFT:	{
		if (imageVal != nullptr)
			return imageVal->getDescriptionString();
	}
	}
	return string("");
}


/*	========================================	*/
#pragma mark --------------------- factory functions (creates ISFVal instances)


ISFVal ISFNullVal()	{
	return ISFVal();
}
ISFVal ISFEventVal(const bool & n)	{
	return ISFVal(ISFValType_Event, n);
}
ISFVal ISFBoolVal(const bool & n)	{
	return ISFVal(ISFValType_Bool, n);
}
ISFVal ISFLongVal(const int32_t & n)	{
	return ISFVal(ISFValType_Long, n);
}
ISFVal ISFFloatVal(const double & n)	{
	return ISFVal(ISFValType_Float, n);
}
ISFVal ISFPoint2DVal(const double & inX, const double & inY)	{
	return ISFVal(ISFValType_Point2D, inX, inY);
}
ISFVal ISFColorVal(const double & inR, const double & inG, const double & inB, const double & inA)	{
	return ISFVal(ISFValType_Color, inR, inG, inB, inA);
}
ISFVal ISFImageVal(const GLBufferRef & n)	{
	return ISFVal(ISFValType_Image, n);
}
ISFVal ISFAudioVal()	{
	return ISFVal(ISFValType_Audio);
}
ISFVal ISFAudioFFTVal()	{
	return ISFVal(ISFValType_AudioFFT);
}


}
