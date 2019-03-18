#ifndef ISFPassTarget_hpp
#define ISFPassTarget_hpp

#include "VVISF_Base.hpp"

#include "exprtk/exprtk.hpp"




namespace VVISF
{




using namespace std;




class ISFDoc;




//! Describes the target of a render pass for an ISF file, stores a number of properties and values specific to this render pass.
/*!
\ingroup VVISF_BASIC
Stores the GLBuffer (the GL resource) which this pass will render into, as well as the expressions that determine the width/height (both the raw string as well as the evaluated expression, capable of being executed with substitutions for variables) and the evaluated value.  Also caches the uniform locations for this target buffer's attributes in the compiled GLSL program.
*/
class VVISF_EXPORT ISFPassTarget	{
	private:
		string			_name;
		VVGL::GLBufferRef		_buffer = nullptr;
		ISFDoc			*_parentDoc;	//	weak ref to the parent doc (ISFDoc*) that created and owns me
		
		mutex			_targetLock;
		
		double			_targetWidth = 1.0;	//	the target width for this pass.  the expression evaluates to this value
		string			*_targetWidthString = nullptr;
		exprtk::expression<double>		*_targetWidthExpression = nullptr;
		double			_widthExpressionVar = 1.0;	//	the expression expects you to maintain static memory for the variables in its symbol table (the memory has to be retained as long as the expression is in use)

		double			_targetHeight = 1.0;	//	the target height for this pass.  the expression evaluates to this value
		string			*_targetHeightString = nullptr;
		exprtk::expression<double>		*_targetHeightExpression = nullptr;
		double			_heightExpressionVar = 1.0;	//	the expression expects you to maintain static memory for the variables in its symbol table (the memory has to be retained as long as the expression is in use)

		bool			_floatFlag = false;	//	NO by default, if YES makes float texutres
		bool			_persistentFlag = false;	//	NO by default, if YES this is a persistent buffer (and it needs to be cleared to black on creation)
		VVGL::GLCachedUniRef	_cachedUnis[4] = { nullptr, nullptr, nullptr, nullptr };
	public:
		//	"class method" that creates a buffer ref
		static ISFPassTargetRef Create(const string & inName, const ISFDoc * inParentDoc);
		//	clears the static tex-to-tex copier maintained by the class
		static void cleanup();
		
		ISFPassTarget(const string & inName, const ISFDoc * inParentDoc);
		~ISFPassTarget();
		ISFPassTarget(const ISFPassTarget & n) = default;
		ISFPassTarget(ISFPassTarget && n) = default;
		ISFPassTarget & operator=(ISFPassTarget & n) = delete;
		ISFPassTarget & operator=(ISFPassTarget && n) = delete;
		
		//!	Sets the receiver's target width string to the passed value.  This string will be evaluated using the exprtk lib, and the resulting value will be used to determine the width at which this pass renders.
		void setTargetWidthString(const string & n);
		//!	Gets the receiver's target width string.
		const string targetWidthString();
		//!	Sets the receiver's target height string to the passed value.  This string will be evaluated using the exprtk lib, and the resulting value will be used to determine the height at which this pass renders.
		void setTargetHeightString(const string & n);
		//!	Gets the receiver's target height string.
		const string targetHeightString();
		//!	Sets the float flag for this pass- if true, this pass needs to render to a high-bitdepth texture.
		void setFloatFlag(const bool & n);
		//!	Gets the float flag for this pass- if true, this pass needs to render to a high-bitdepth texture.
		bool floatFlag() const { return _floatFlag; }
		//! Sets the persistent flag for this pass- if true, this pass's buffer will be used as an input when rendering the next frame.
		void setPersistentFlag(const bool & n);
		//! Gets the persistent flag for this pass- if true, the pass's buffer will be used as an input when rendering the next frame.
		bool persistentFlag() const { return _persistentFlag; }
		//!	Deletes any GL resources that might presently be cached by this pass.
		void clearBuffer();
		
		bool targetSizeNeedsEval() const { return (_targetHeightString!=nullptr || _targetHeightString!=nullptr); }
		void evalTargetSize(const VVGL::Size & inSize, map<string, double*> & inSymbols, const bool & inResize, const bool & inCreateNewBuffer);
		
		//!	Returns the receiver's name.
		string & name() { return _name; }
		//!	Returns the GLBuffer currently cached with this pass, or null.
		VVGL::GLBufferRef & buffer() { return _buffer; }
		//!	Sets the GLBuffer currently cached with this pass.
		void setBuffer(const VVGL::GLBufferRef & n) { _buffer=n; }
		
		//!	Returns the last-calculated target size for this pass.
		VVGL::Size targetSize() { return { _targetWidth, _targetHeight }; }
		
		void cacheUniformLocations(const int & inPgmToCheck) { for (int i=0; i<4; ++i) _cachedUnis[i]->cacheTheLoc(inPgmToCheck); }
		int32_t getUniformLocation(const int & inIndex) const { return (inIndex<0||inIndex>3) ? -1 : _cachedUnis[inIndex]->loc; }
		void clearUniformLocations() { for (int i=0; i<4; ++i) _cachedUnis[i]->purgeCache(); }
	
	private:
		void setTargetSize(const VVGL::Size & inSize, const bool & inResize=true, const bool & inCreateNewBuffer=true);
	
};




}

#endif /* ISFPassTarget_hpp */
