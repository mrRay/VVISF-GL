#ifndef ISFAttr_hpp
#define ISFAttr_hpp

#include "VVISF_Base.hpp"
#include <stdint.h>
#include <vector>
#include <string>




namespace VVISF	{




using namespace std;




//	this class describes a single ISF attribute
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
		
		inline string & getName() const { return const_cast<string&>(name); }
		inline string & getDescription() const { return const_cast<string&>(description); }
		inline string & getLabel() const { return const_cast<string&>(label); }
		inline ISFValType & getType() const { return const_cast<ISFValType&>(type); }
		inline ISFVal & getCurrentVal() { return currentVal; }
		inline void setCurrentVal(const ISFVal & n) { currentVal=n; }
		double * updateAndGetEvalVariable();	//	updates this attribute's eval variable with the double val of "currentVal", and returns a ptr to the eval variable
		inline bool shouldHaveImageBuffer() const { return (type==ISFValType_Cube || type==ISFValType_Image || type==ISFValType_Audio || type==ISFValType_AudioFFT); }
		inline GLBufferRef getCurrentImageBuffer() { if (!shouldHaveImageBuffer()) return nullptr; return currentVal.getImageBuffer(); }
		inline void setCurrentImageBuffer(const GLBufferRef & n) { /*cout<<__PRETTY_FUNCTION__<<"..."<<*this<<", "<<*n<<endl;*/if (ISFValTypeUsesImage(type)) currentVal = ISFImageVal(n); else cout << "\terr: tried to set current image buffer in non-image attr (" << name << ")\n"; /*cout<<"\tcurrentVal is now "<<currentVal<<endl;*/ }
		inline ISFVal & getMinVal() { return minVal; }
		inline ISFVal & getMaxVal() { return maxVal; }
		inline ISFVal & getDefaultVal() { return defaultVal; }
		inline ISFVal & getIdentityVal() { return identityVal; }
		inline vector<string> & getLabelArray() { return labelArray; }
		inline vector<int32_t> & getValArray() { return valArray; }
		inline bool getIsFilterInputImage() { return isFilterInputImage; }
		inline void setIsFilterInputImage(const bool & n) { isFilterInputImage=n; }
		inline void clearUniformLocations() { for (int i=0; i<4; ++i) uniformLocation[i]=0; }
		inline void setUniformLocation(const int & inIndex, const int32_t & inNewVal) { if (inIndex<0 || inIndex>3) return; uniformLocation[inIndex] = inNewVal; }
		inline int32_t getUniformLocation(const int & inIndex) { if (inIndex<0 || inIndex>3) return 0; return uniformLocation[inIndex]; }
		//inline bool isNullVal() { return (type==ISFValType_None); }
		
		VVISF_EXPORT friend ostream & operator<<(ostream & os, const ISFAttr & n);
		void lengthyDescription();
		string getDescription();
};




}




#endif /* ISFAttr_hpp */
