#ifndef ISFTargetBuffer_hpp
#define ISFTargetBuffer_hpp

#include "ISFBase.hpp"

#include "exprtk/exprtk.hpp"




namespace VVISFKit
{




using namespace std;



	
	//	ISFPassTargetRef is a shared pointer to an ISFPassTarget- this is what you should use
	class ISFPassTarget;
	using ISFPassTargetRef = std::shared_ptr<ISFPassTarget>;
	
	
	
	
	/*		this class represents a target buffer for a render pass in an ISF shader- it stores the 
	VVGLBuffer (the GL resource) as well as the expressions determining the width/height (the raw 
	string, the evaluated expression- capable of being executed with substitutions for variables, 
	and the evaluated value), and the cached uniform locations for this target buffer's attributes 
	in the compiled GL program (so you don't have to look up the uniform location every frame).			*/
	
	
	class ISFPassTarget	{
		private:
			string			name;
			VVGLBufferRef		buffer = nullptr;
			
			mutex			targetLock;
			
			double			targetWidth = 1.0;	//	the target width for this pass.  the expression evaluates to this value
			string			*targetWidthString = nullptr;
			exprtk::expression<double>		*targetWidthExpression = nullptr;
			double			widthExpressionVar = 1.0;	//	the expression expects you to maintain static memory for the variables in its symbol table (the memory has to be retained as long as the expression is in use)
	
			double			targetHeight = 1.0;	//	the target height for this pass.  the expression evaluates to this value
			string			*targetHeightString = nullptr;
			exprtk::expression<double>		*targetHeightExpression = nullptr;
			double			heightExpressionVar = 1.0;	//	the expression expects you to maintain static memory for the variables in its symbol table (the memory has to be retained as long as the expression is in use)
	
			bool			floatFlag = false;	//	NO by default, if YES makes float texutres
			int32_t			uniformLocation[4] = { -1, -1, -1, -1 };	//	the location of this attribute in the compiled GLSL program. cached here because lookup times are costly when performed every frame.  there are 4 because images require four uniforms (one of the texture name, one for the size, one for the img rect, and one for the flippedness)
		public:
			//	"class method" that creates a buffer ref
			static ISFPassTargetRef Create(const string & inName);
	
			ISFPassTarget(const string & inName);
			~ISFPassTarget();
			ISFPassTarget(const ISFPassTarget & n) = default;
			ISFPassTarget(ISFPassTarget && n) = default;
			ISFPassTarget & operator=(ISFPassTarget & n) = delete;
			ISFPassTarget & operator=(ISFPassTarget && n) = delete;
	
			void setTargetSize(const VVGL::Size & inSize, const bool & inResize=true, const bool & inCreateNewBuffer=true);
			void setTargetWidthString(const string & n);
			const string getTargetWidthString();
			void setTargetHeightString(const string & n);
			const string getTargetHeightString();
			void setFloatFlag(const bool & n);
			bool getFloatFlag() const { return floatFlag; }
			void clearBuffer();
	
			bool targetSizeNeedsEval() const { return (targetHeightString!=nullptr || targetHeightString!=nullptr); }
			void evalTargetSize(const VVGL::Size & inSize, map<string, double*> & inSymbols, const bool & inResize, const bool & inCreateNewBuffer);
	
			string & getName() { return name; }
			VVGLBufferRef & getBuffer() { return buffer; }
			void setBuffer(const VVGLBufferRef & n) { buffer=n; }
	
			VVGL::Size targetSize() { return { targetWidth, targetHeight }; }
	
			void setUniformLocation(const int & inIndex, const int32_t & inNewVal) { uniformLocation[inIndex]=inNewVal; }
			int32_t getUniformLocation(const int & inIndex) const { return uniformLocation[inIndex]; }
			void clearUniformLocations() { for (int i=0; i<4; ++i) uniformLocation[i]=-1; }
	};
}

#endif /* ISFTargetBuffer_hpp */
