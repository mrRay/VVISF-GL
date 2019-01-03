#ifndef ISFVal_h
#define ISFVal_h

#include "VVISF_Base.hpp"
#include "VVISF_StringUtils.hpp"

/*!
\file
*/




namespace VVISF
{


using namespace std;


/*!
\brief Enumerates the different kinds of ISF values.
\ingroup VVISF_BASIC
*/
enum ISFValType	{
	ISFValType_None,	//!<	No data/unknown value type.
	ISFValType_Event,	//!<	No data, just an event.  sends a 1 the next render after the event is received, a 0 any other time it's rendered
	ISFValType_Bool,	//!<	A boolean choice, sends 1 or 0 to the shader
	ISFValType_Long,	//!<	Sends a long
	ISFValType_Float,	//!<	Sends a float
	ISFValType_Point2D,	//!<	Sends a 2 element vector
	ISFValType_Color,	//!<	Sends a 4 element vector representing an RGBA color
	ISFValType_Cube,	//!<	Sends a long- the texture number (like GL_TEXTURE0) of a cubemap texture to pass to the shader
	ISFValType_Image,	//!<	Sends a long- the texture number (like GL_TEXTURE0) to pass to the shader
	ISFValType_Audio,	//!<	Sends a long- the texture number (like GL_TEXTURE0) to pass to the shader
	ISFValType_AudioFFT	//!<	Sends a long- the texture number (like GL_TEXTURE0) to pass to the shader
};
/*!
\brief Returns a string describing the passed value type.
\ingroup VVISF_BASIC
\relatesalso VVISF::ISFValType
*/
string StringFromISFValType(const ISFValType & n);
/*!
\brief Returns a true if the passed value type uses an image for its value.
\ingroup VVISF_BASIC
\relatesalso VVISF::ISFValType
*/
inline bool ISFValTypeUsesImage(const ISFValType & n) { return (n==ISFValType_Cube || n==ISFValType_Image || n==ISFValType_Audio || n==ISFValType_AudioFFT); }



/*!
\brief ISFVal describes an ISF value- it has a type (ISFValType) and a type-dependent value.  Intended to be immutable.
\ingroup VVISF_BASIC
*/
struct VVISF_EXPORT ISFVal	{
	private:
		//	this union stores the value of an ISFVal (the member of the union used depends on the ISFVal's 'type' member)
		union ISFValUnion {
			bool		boolVal;
			int32_t		longVal;
			double		floatVal;
			double		pointVal[2];
			double		colorVal[4];
		};
		ISFValType		_type = ISFValType_None;
		ISFValUnion		_val = { false };
		VVGL::GLBufferRef		_imageVal = nullptr;	//	we store the GLBufferRef as a member of the struct because storing it in the union didn't work out (a variant might work, but that has to wait to c++17)
	
	public:
		
		
		/*!
		\name Constructor functions
		*/
		///@{
		
		//	Returns a null-type ISFVal
		ISFVal();
		//	Returns an ISFVal with the passed type, and the default/unpopulated value for that type.
		ISFVal(const ISFValType & inType) : _type(inType) {}
		//	Returns an ISFVal of the passed type with the passed bool value.
		ISFVal(const ISFValType & inType, const bool & inBool);
		//	Returns an ISFVal of the passed type with the passed long value.
		ISFVal(const ISFValType & inType, const int32_t & inLong);
		//	Returns an ISFVal of the passed type with the passed float value.
		ISFVal(const ISFValType & inType, const double & inFloat);
		//	Returns an ISFVal of the passed type populated with the two passed values.  Works well for 2d points.
		ISFVal(const ISFValType & inType, const double & inX, const double & inY);
		ISFVal(const ISFValType & inType, const double * inXY, const size_t inSizeToCopy);
		//	Returns an ISFVal of the passed type populated with the four passed values.  Works well for colors.
		ISFVal(const ISFValType & inType, const double & inR, const double & inG, const double & inB, const double & inA);
		//	Returns an ISFVal of the passed type with the passed image value.  Works well for image-, audio-, and audioFFT-type values.
		ISFVal(const ISFValType & inType, const VVGL::GLBufferRef & inImage);
		
		///@}
		
		
		//! These functions retrieve various properties of the ISFVal instance.
		/*!
		\name Setter/getter functions
		While ISFVal is meant to be immutable, a couple setters are included for populating some of the more complex value types to make life easier.
		*/
		///@{
		
		//!	Returns the value type.
		inline ISFValType type() const { return _type; }
		//!	Returns a double describing the value of this object.  Safe to call, even if the value type shouldn't be represented by a double.
		double getDoubleVal() const;
		//!	Returns a bool describing the value of this object.  Safe to call, even if the value type shouldn't be represented by a bool.
		bool getBoolVal() const;
		//!	Returns a long describing the value of this object.  Safe to call, even if the value type shouldn't be represented by a long.
		int32_t getLongVal() const;
		//!	Returns a null if the receiver isn't a Point2D-type object, otherwise it returns a pointer to the two-element array containing the point values.  This pointer is only valid for the lifetime of the receiver.
		inline double * getPointValPtr() { if (_type!=ISFValType_Point2D) return nullptr; return &(_val.pointVal[0]); }
		//!	Returns 0. if the receiver value type isn't Point2D or the passed index is out of bounds, otherwise it returns the point value at the passed index.
		inline double getPointValByIndex(const int & inIndex) { if (_type!=ISFValType_Point2D || inIndex<0 || inIndex>1) return 0.; return _val.pointVal[inIndex]; }
		//!	Does nothing if the receiver's value type isn't Point2D or the passed index is out of bounds, otherwise it sets the value at the passed index.
		inline void setPointValByIndex(const int & inIndex, const double & inVal) { if (_type!=ISFValType_Point2D || inIndex<0 || inIndex>1) return; _val.pointVal[inIndex]=inVal; }
		//!	Returns a null if the receiver isn't a color-type object, otherwise it returns a pointer to the four-element array containing the color values.  This pointer is only valid for the lifetime of the receiver.
		inline double * getColorValPtr() { if (_type!=ISFValType_Color) return nullptr; return &(_val.colorVal[0]); }
		//!	Does nothing if the receiver's value type isn't color or the passed index is out of bounds, otherwise it returns the value of the color channel at the passed index.
		inline double getColorValByChannel(const int & inIndex) { if (_type!=ISFValType_Color || inIndex<0 || inIndex>3) return 0.; return _val.colorVal[inIndex]; }
		//!	Does nothing if the receiver's value type isn't color or the passed index is out of bounds, otherwise it sets the value of the color channel at the passed index.
		inline void setColorValByChannel(const int & inIndex, const double & inVal) { if (_type!=ISFValType_Color || inIndex<0 || inIndex>3) return; _val.colorVal[inIndex]=inVal; }
		//!	Returns null if the receiver's value type cannot be represented as an image, otherwise it returns the image buffer (almost certainly a GL texture) that is the receiver's value.
		VVGL::GLBufferRef imageBuffer() const;
		//!	Does nothing if the receiver's value type cannot be represented as an image, otherwise it sets the receiver's image value with the passed buffer.  This buffer will be "retained" for the duration of the receiver's lifetime.
		void setImageBuffer(const VVGL::GLBufferRef & n);
		//!	Returns a string describing the type of the receiver.
		string getTypeString() const;
		//!	Returns a string describing the value of the receiver.
		string getValString() const;
		
		///@}
		
		
		/*!
		\name Value type queries
		\brief Inline functions that simplify the process of checking to see if a given ISFVal matches a particular value type.
		*/
		///@{
		
		//!	Returns true if the receiver is a null value.
		inline bool isNullVal() const { return (_type == ISFValType_None); }
		//!	Returns true if the receiver is an event value.
		inline bool isEventVal() const { return (_type == ISFValType_Event); }
		//!	Returns true if the receiver is a bool value.
		inline bool isBoolVal() const { return (_type == ISFValType_Bool); }
		//!	Returns true if the receiver is a long value.
		inline bool isLongVal() const { return (_type == ISFValType_Long); }
		//!	Returns true if the receiver is a float value.
		inline bool isFloatVal() const { return (_type == ISFValType_Float); }
		//!	Returns true if the receiver is a point2D value.
		inline bool isPoint2DVal() const { return (_type == ISFValType_Point2D); }
		//!	Returns true if the receiver is a color value.
		inline bool isColorVal() const { return (_type == ISFValType_Color); }
		//!	Returns true if the receiver is a cube texture value.
		inline bool isCubeVal() const { return (_type == ISFValType_Cube); }
		//!	Returns true if the receiver is an image value.
		inline bool isImageVal() const { return (_type == ISFValType_Image); }
		//!	Returns true if the receiver is an audio value (image).
		inline bool isAudioVal() const { return (_type == ISFValType_Audio); }
		//!	Returns true if the receiver is an audio fft value (image).
		inline bool isAudioFFTVal() const { return (_type == ISFValType_AudioFFT); }
		
		///@}
		
		friend ostream & operator<<(ostream & os, const ISFVal & n) { os << VVGL::FmtString("<ISFVal %s/%s>", n.getTypeString().c_str(), n.getValString().c_str()); return os; }
};


/*!
\name ISFVal creation functions.
\brief These functions create ISFVal instances.  Everything here can be done with constructor functions, but the syntax here is a nicer and clearer.
*/
///@{

/*!
\relatesalso VVISF::ISFVal
\brief Creates and returns a null-type ISFVal.
*/
VVISF_EXPORT ISFVal ISFNullVal();
/*!
\relatesalso VVISF::ISFVal
\brief Creates and returns an event-type ISFVal.  Events don't technically have a value- events should send a "true" for one frame and a false for every other frame.  For convenience, ISFVal uses a bool to cache the event value.
*/
VVISF_EXPORT ISFVal ISFEventVal(const bool & n=false);
/*!
\relatesalso VVISF::ISFVal
\brief Creates and returns a boolean-type ISFVal with the passed value.
*/
VVISF_EXPORT ISFVal ISFBoolVal(const bool & n);
/*!
\relatesalso VVISF::ISFVal
\brief Creates and returns a long-type ISFVal with the passed value.
*/
VVISF_EXPORT ISFVal ISFLongVal(const int32_t & n);
/*!
\relatesalso VVISF::ISFVal
\brief Creates and returns a float-type ISFVal with the passed value.
*/
VVISF_EXPORT ISFVal ISFFloatVal(const double & n);
/*!
\relatesalso VVISF::ISFVal
\brief Creates and returns a 2d point-type ISFVal with the passed point values.
*/
VVISF_EXPORT ISFVal ISFPoint2DVal(const double & inX, const double & inY);
/*!
\relatesalso VVISF::ISFVal
\brief Creates and returns a color-type ISFVal with the passed R/G/B/A color values.
*/
VVISF_EXPORT ISFVal ISFColorVal(const double & inR, const double & inG, const double & inB, const double & inA);
/*!
\relatesalso VVISF::ISFVal
\brief Creates and returns an image-type ISFVal with the passed buffer.
*/
VVISF_EXPORT ISFVal ISFImageVal(const VVGL::GLBufferRef & n);

///@}



}


#endif /* ISFVal_h */
