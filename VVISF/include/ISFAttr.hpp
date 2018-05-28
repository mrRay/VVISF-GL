#ifndef ISFAttr_hpp
#define ISFAttr_hpp

#include "VVISF_Base.hpp"
#include <stdint.h>
#include <vector>
#include <string>




namespace VVISF	{




using namespace std;




/*!
\brief Describes a single ISF attribute
\ingroup VVISF_BASIC
*/
class VVISF_EXPORT ISFAttr	{
	protected:
		string			name;
		string			description;
		string			label;
		
		ISFValType			type = ISFValType_None;
		ISFVal				currentVal = ISFNullVal();
		ISFVal				minVal = ISFNullVal();	//	if it's an audio/audiofft, it's a long-type val.  otherwise, null or an ISFVal subclass of the appropriate type
		ISFVal				maxVal = ISFNullVal();	//	if it's an audio/audiofft, it's a long-type val.  otherwise, null or an ISFVal subclass of the appropriate type
		ISFVal				defaultVal = ISFNullVal();
		ISFVal				identityVal = ISFNullVal();
		vector<string>		labelArray;	//	only used if it's a LONG. vector containing strings that correspond to the values in "valArray"
		vector<int32_t>		valArray;	//	only used if it's a LONG. vector containing ints with the values that correspond to the accompanying labels
		
		bool				isFilterInputImage = false;	//	if true, this is an image-type input and is the main input for an image filter
		int32_t				uniformLocation[4] = { -1, -1, -1, -1 };	//	the location of this attribute in the compiled GLSL program. cached here because lookup times are costly when performed every frame.  there are 4 because images require four uniforms (one of the texture name, one for the size, one for the img rect, and one for the flippedness)
		
		double				evalVariable = 1.0;	//	attribute values are available in expression evaluation- to support this, each attribute needs to maintain a double which it populates with its current value
	public:
		
		/*
		\brief You probably shouldn't be creating attributes directly- instead, you'll likely encounter it as an ISFAttrRef vended by an ISFDoc which may or may not be owned by an ISFScene.  Nevertheless, this is the constructor:
		\param inName The name of the ISF attribute
		\param inDesc The attribute description
		\param inLabel The attribute label
		\param inType The attribute's value type
		\param inMinVal The attribute's min value, or a null value if there is no min val or a min val would be inappropriate
		\param inMaxVal The attribute's max value, or a null value if there is no max val or a max val would be inappropriate
		\param inDefVal The attribute's default value, or a null value if there is no default val or a default val would be inappropriate
		\param inIdenVal The attribute's identity value, or a null value if there is no identity val or an identity val would be inappropriate
		\param inLabels A ptr to a vector containing strings with the attribute's labels, or null if the labels are inappropriate or absent.  The receiver does not assume ownership of the passed ptr.
		\param inVals A ptr to a vector containing ints with the attribute's values (presuming long-type attribute), or null if the values are inappropriate or absent.  The receiver does not assume ownership of the passed ptr.
		*/
		ISFAttr(const string & inName,
			const string & inDesc,
			const string & inLabel,
			const ISFValType & inType,
			const ISFVal & inMinVal=ISFNullVal(),
			const ISFVal & inMaxVal=ISFNullVal(),
			const ISFVal & inDefVal=ISFNullVal(),
			const ISFVal & inIdenVal=ISFNullVal(),
			const vector<string> * inLabels=nullptr,
			const vector<int32_t> * inVals=nullptr);
		~ISFAttr();
		
		//!	Returns the attribute's name, or null
		inline string & getName() const { return const_cast<string&>(name); }
		//!	Returns the attribute's description, or null
		inline string & getDescription() const { return const_cast<string&>(description); }
		//!	Returns the attribute's label, or null
		inline string & getLabel() const { return const_cast<string&>(label); }
		//!	Returns the attribute's value type.
		inline ISFValType & getType() const { return const_cast<ISFValType&>(type); }
		//!	Returns the attribute's current value.
		inline ISFVal & getCurrentVal() { return currentVal; }
		//!	Sets the attribute's current value.
		inline void setCurrentVal(const ISFVal & n) { currentVal=n; }
		//	updates this attribute's eval variable with the double val of "currentVal", and returns a ptr to the eval variable
		double * updateAndGetEvalVariable();
		//!	Returns a true if this attribute's value is expressed with an image buffer
		inline bool shouldHaveImageBuffer() const { return ISFValTypeUsesImage(type); }
		//!	Returns the receiver's image buffer
		inline GLBufferRef getCurrentImageBuffer() { if (!shouldHaveImageBuffer()) return nullptr; return currentVal.getImageBuffer(); }
		//!	Sets the receiver's current value with the passed image buffer
		inline void setCurrentImageBuffer(const GLBufferRef & n) { /*cout<<__PRETTY_FUNCTION__<<"..."<<*this<<", "<<*n<<endl;*/if (shouldHaveImageBuffer()) currentVal = ISFImageVal(n); else cout << "\terr: tried to set current image buffer in non-image attr (" << name << ")\n"; /*cout<<"\tcurrentVal is now "<<currentVal<<endl;*/ }
		//!	Gets the attribute's min val
		inline ISFVal & getMinVal() { return minVal; }
		//!	Gets the attribute's max val
		inline ISFVal & getMaxVal() { return maxVal; }
		//!	Gets the attribute's default val (the value which will be assigned to the attribute when it is first created and used for rendering)
		inline ISFVal & getDefaultVal() { return defaultVal; }
		//!	Gets the attribute's identity val (the value at which this attribute's effects are indistinguishable from its raw input).
		inline ISFVal & getIdentityVal() { return identityVal; }
		//!	Gets the attribute's labels as a vector of string values.  Only used if the attribute is a 'long'.
		inline vector<string> & getLabelArray() { return labelArray; }
		//!	Gets the attribute's values as a vector of int values.  Only used if the attribute is a 'long'.
		inline vector<int32_t> & getValArray() { return valArray; }
		//!	Returns a true if this attribute is used to send the input image to the filter.
		inline bool getIsFilterInputImage() { return isFilterInputImage; }
		inline void setIsFilterInputImage(const bool & n) { isFilterInputImage=n; }
		inline void clearUniformLocations() { for (int i=0; i<4; ++i) uniformLocation[i]=0; }
		inline void setUniformLocation(const int & inIndex, const int32_t & inNewVal) { if (inIndex<0 || inIndex>3) return; uniformLocation[inIndex] = inNewVal; }
		inline int32_t getUniformLocation(const int & inIndex) { if (inIndex<0 || inIndex>3) return 0; return uniformLocation[inIndex]; }
		//inline bool isNullVal() { return (type==ISFValType_None); }
		
		VVISF_EXPORT friend ostream & operator<<(ostream & os, const ISFAttr & n);
		void lengthyDescription();
		//!	Returns a human-readable string that briefly describes the attribute.
		string getDescription();
};




}




#endif /* ISFAttr_hpp */
