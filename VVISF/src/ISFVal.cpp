#include "ISFVal.hpp"
#include "VVISF_StringUtils.hpp"

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
	_type = ISFValType_None;
	_val.boolVal = false;
}
ISFVal::ISFVal(const ISFValType & inType, const bool & inBool) : _type(inType)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	//_type = inType;
	_val.boolVal = inBool;
}
ISFVal::ISFVal(const ISFValType & inType, const int32_t & inLong) : _type(inType)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	//_type = inType;
	_val.longVal = inLong;
}
ISFVal::ISFVal(const ISFValType & inType, const double & inFloat) : _type(inType)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	//_type = inType;
	_val.floatVal = inFloat;
}
ISFVal::ISFVal(const ISFValType & inType, const double & inX, const double & inY) : _type(inType)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	//_type = inType;
	_val.pointVal[0] = inX;
	_val.pointVal[1] = inY;
}
ISFVal::ISFVal(const ISFValType & inType, const double * inBuffer, const size_t inSizeToCopy) : _type(inType)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	//_type = inType;
	double		*rPtr = const_cast<double*>(inBuffer);
	for (size_t i=0; i<inSizeToCopy; ++i)	{
		_val.pointVal[i] = *rPtr;
		++rPtr;
	}
}
ISFVal::ISFVal(const ISFValType & inType, const double & inR, const double & inG, const double & inB, const double & inA) : _type(inType)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	//_type = inType;
	_val.colorVal[0] = inR;
	_val.colorVal[1] = inG;
	_val.colorVal[2] = inB;
	_val.colorVal[3] = inA;
}
ISFVal::ISFVal(const ISFValType & inType, const GLBufferRef & inImage) : _type(inType)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	//_type = inType;
	_imageVal = inImage;
}


/*	========================================	*/
#pragma mark --------------------- methods


double ISFVal::getDoubleVal() const	{
	double		returnMe = 0.;
	switch (_type)	{
	case ISFValType_None:
		break;
	case ISFValType_Event:
	case ISFValType_Bool:
		returnMe = (_val.boolVal) ? 1. : 0.;
		break;
	case ISFValType_Long:
		returnMe = (double)_val.longVal;
		break;
	case ISFValType_Float:
		returnMe = (double)_val.floatVal;
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
	switch (_type)	{
	case ISFValType_None:
		returnMe = false;
		break;
	case ISFValType_Event:
		returnMe = _val.boolVal;
		break;
	case ISFValType_Bool:
		returnMe = _val.boolVal;
		break;
	case ISFValType_Long:
		returnMe = (_val.longVal>0) ? true : false;
		break;
	case ISFValType_Float:
		returnMe = (_val.floatVal>0.) ? true : false;
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
	switch (_type)	{
	case ISFValType_None:
		returnMe = 0;
		break;
	case ISFValType_Event:
	case ISFValType_Bool:
		returnMe = (_val.boolVal) ? 1 : 0;
		break;
	case ISFValType_Long:
		returnMe = _val.longVal;
		break;
	case ISFValType_Float:
		returnMe = (int32_t)_val.floatVal;
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
	switch (_type)	{
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
		//cout << "\timageVal was " << _imageVal << endl;
		return _imageVal;
		break;
	}
	return nullptr;
}
void ISFVal::setImageBuffer(const GLBufferRef & n)	{
	//cout << __FUNCTION__ << ", self is " << this << endl;
	switch (_type)	{
	case ISFValType_None:
	case ISFValType_Event:
	case ISFValType_Bool:
	case ISFValType_Long:
	case ISFValType_Float:
	case ISFValType_Point2D:
	case ISFValType_Color:
		break;
	case ISFValType_Cube:
	case ISFValType_Image:
	case ISFValType_Audio:
	case ISFValType_AudioFFT:
		//cout << "\timageVal was " << _imageVal << endl;
		_imageVal=n;
		break;
	}
	//cout << "\timageVal is now " << _imageVal << endl;
}

string ISFVal::getTypeString() const	{
	return ISFValTypeString(_type);
}
string ISFVal::getValString() const	{
	switch (_type)	{
	case ISFValType_None:
		return string("None");
	case ISFValType_Event:
		return string("Event/None");
	case ISFValType_Bool:	{
		return (_val.boolVal) ? string("true") : string("false");
	}
	case ISFValType_Long:	{
		return FmtString("%d",_val.longVal);
	}
	case ISFValType_Float:	{
		return FmtString("%f",_val.floatVal);
	}
	case ISFValType_Point2D:	{
		return FmtString("(%0.2f, %0.2f)",_val.pointVal[0],_val.pointVal[1]);
	}
	case ISFValType_Color:	{
		return FmtString("{%0.2f, %0.2f, %0.2f, %0.2f}",_val.colorVal[0],_val.colorVal[1],_val.colorVal[2],_val.colorVal[3]);
	}
	case ISFValType_Cube:
	case ISFValType_Image:
	case ISFValType_Audio:
	case ISFValType_AudioFFT:	{
		if (_imageVal != nullptr)
			return _imageVal->getDescriptionString();
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


}
