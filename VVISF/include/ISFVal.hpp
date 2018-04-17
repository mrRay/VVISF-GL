#ifndef ISFVal_h
#define ISFVal_h

#include "ISFBase.hpp"
//#include "VVGLBuffer.hpp"
#include "ISFStringUtils.hpp"

#if ISF_SDK_QT
#include "vvisf_qt_global.h"
#endif



namespace VVISF
{


using namespace std;


//	this enum describes the different kinds of ISF values
enum ISFValType	{
	ISFValType_None,
	ISFValType_Event,	//!<	no data, just an event.  sends a 1 the next render after the event is received, a 0 any other time it's rendered
	ISFValType_Bool,	//!<	a boolean choice, sends 1 or 0 to the shader
	ISFValType_Long,	//!<	sends a long
	ISFValType_Float,	//!<	sends a float
	ISFValType_Point2D,	//!<	sends a 2 element vector
	ISFValType_Color,	//!<	sends a 4 element vector representing an RGBA color
	ISFValType_Cube,	//!<	sends a long- the texture number (like GL_TEXTURE0) of a cubemap texture to pass to the shader
	ISFValType_Image,	//!<	sends a long- the texture number (like GL_TEXTURE0) to pass to the shader
	ISFValType_Audio,	//!<	sends a long- the texture number (like GL_TEXTURE0) to pass to the shader
	ISFValType_AudioFFT	//!<	sends a long- the texture number (like GL_TEXTURE0) to pass to the shader
};
string ISFValTypeString(const ISFValType & n);
inline bool ISFValTypeUsesImage(const ISFValType & n) { return (n==ISFValType_Cube || n==ISFValType_Image || n==ISFValType_Audio || n==ISFValType_AudioFFT); }

//	this union stores the value of an ISFVal (the member of the union used depends on the ISFVal's 'type' member)
union ISFValUnion {
	bool		boolVal;
	int32_t		longVal;
	double		floatVal;
	double		pointVal[2];
	double		colorVal[4];
};


//	ISFVal is the base struct- it defines a type, a contains a union with the value for the type
struct ISFVal	{
	private:
		ISFValType		type = ISFValType_None;
		ISFValUnion		val = { .boolVal = false };
		VVGLBufferRef	imageVal = nullptr;	//	we store the VVGLBufferRef as a member of the struct because storing it in the union didn't work out (a variant might work, but that has to wait to c++17)
	
	public:
		//	constructor methods
		ISFVal();
		ISFVal(const ISFValType & inType) : type(inType) {}
		ISFVal(const ISFValType & inType, const bool & inBool);
		ISFVal(const ISFValType & inType, const int32_t & inLong);
		ISFVal(const ISFValType & inType, const double & inFloat);
		ISFVal(const ISFValType & inType, const double & inX, const double & inY);
		ISFVal(const ISFValType & inType, const double * inXY, const size_t inSizeToCopy);
		ISFVal(const ISFValType & inType, const double & inR, const double & inG, const double & inB, const double & inA);
		ISFVal(const ISFValType & inType, const VVGLBufferRef & inImage);
	public:
		//	accessor methods
		inline ISFValType getType() const { return type; }
		double getDoubleVal() const;
		bool getBoolVal() const;
		int32_t getLongVal() const;
		inline double * getPointValPtr() { if (type!=ISFValType_Point2D) return nullptr; return &(val.pointVal[0]); }
		inline double getPointValByIndex(const int & inIndex) { if (type!=ISFValType_Point2D || inIndex<0 || inIndex>1) return 0.; return val.pointVal[inIndex]; }
		inline void setPointValByIndex(const int & inIndex, const double & inVal) { if (type!=ISFValType_Point2D) return; val.pointVal[inIndex]=inVal; }
		inline double * getColorValPtr() { if (type!=ISFValType_Color) return nullptr; return &(val.colorVal[0]); }
		inline double getColorValByChannel(const int & inIndex) { if (type!=ISFValType_Color || inIndex<0 || inIndex>3) return 0.; return val.colorVal[inIndex]; }
		inline void setColorValByChannel(const int & inIndex, const double & inVal) { if (type!=ISFValType_Color) return; val.colorVal[inIndex]=inVal; }
		VVGLBufferRef getImageBuffer() const;
		void setImageBuffer(const VVGLBufferRef & n);
		string getTypeString() const;
		string getValString() const;
		
		inline bool isNullVal() const { return (type == ISFValType_None); }
		inline bool isEventVal() const { return (type == ISFValType_Event); }
		inline bool isBoolVal() const { return (type == ISFValType_Bool); }
		inline bool isLongVal() const { return (type == ISFValType_Long); }
		inline bool isFloatVal() const { return (type == ISFValType_Float); }
		inline bool isPoint2DVal() const { return (type == ISFValType_Point2D); }
		inline bool isColorVal() const { return (type == ISFValType_Color); }
		inline bool isCubeVal() const { return (type == ISFValType_Cube); }
		inline bool isImageVal() const { return (type == ISFValType_Image); }
		inline bool isAudioVal() const { return (type == ISFValType_Audio); }
		inline bool isAudioFFTVal() const { return (type == ISFValType_AudioFFT); }
		
		friend ostream & operator<<(ostream & os, const ISFVal & n) { os << FmtString("<ISFVal %s/%s>", n.getTypeString().c_str(), n.getValString().c_str()); return os; }
};


//	these functions make it simpler to create ISFVal instances
ISFVal ISFNullVal();
ISFVal ISFEventVal(const bool & n=false);
ISFVal ISFBoolVal(const bool & n);
ISFVal ISFLongVal(const int32_t & n);
ISFVal ISFFloatVal(const double & n);
ISFVal ISFPoint2DVal(const double & inX, const double & inY);
ISFVal ISFColorVal(const double & inR, const double & inG, const double & inB, const double & inA);
ISFVal ISFImageVal(const VVGLBufferRef & n);
ISFVal ISFAudioVal();



}


#endif /* ISFVal_h */
