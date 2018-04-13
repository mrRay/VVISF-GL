#ifndef ISFDoc_hpp
#define ISFDoc_hpp

#include <vector>
#include "ISFBase.hpp"
#include "ISFAttr.hpp"
#include "ISFPassTarget.hpp"

#if ISF_TARGET_QT
#include "vvisf_qt_global.h"
#endif




namespace VVISF
{




using namespace std;

class ISFScene;




/*	this class describes an ISF document.  constructing an instance of this class requires a valid 
path, and during construction the ISF file is parsed and this class gets "filled in" completely (GL 
resources will be loaded during construction).  for the most part, this class is immutable- the only 
notable exception is that the ISFAttr instances in the various input arrays can be used to store 
values for the document.  even then, the contents of the arrays aren't changed- just the contents 
of the objects in the array.	*/
class ISFDoc	{
	private:
		recursive_mutex		propLock;
		
		string			*path = nullptr;	//	full path to the loaded file
		string			*name = nullptr;	//	just the file name (including its extension)
		string			*description = nullptr;	//	description of whatever the file does
		string			*credit = nullptr;	//	credit
		ISFFileType		type = ISFFileType_Source;
		
		vector<string>			categories;	//	array of strings of the category names this doc should be listed under
		vector<ISFAttrRef>	inputs;	//	array of ISFAttrRef instances for the various inputs
		vector<ISFAttrRef>	imageInputs;	//	array of ISFAttrRef instances for the image inputs (the image inputs are stored in two arrays).
		vector<ISFAttrRef>	audioInputs;	//	array of ISFAttrRef instances for the audio inputs
		vector<ISFAttrRef>	imageImports;	//	array of ISFAttrRef instances that describe imported images. attrib's 'attribName' is the name of the sampler, attrib's 'description' is the path to the file.
		
		//bool					bufferRequiresEval = false;	//	NO by default, set to YES during file open if any of the buffers require evaluation (faster than checking every single buffer every pass)
		vector<ISFPassTargetRef>	persistentBuffers;
		vector<ISFPassTargetRef>	tempBuffers;
		vector<string>			renderPasses;
		
		string			*jsonSourceString = nullptr;	//	the JSON string from the source *including the comments and any linebreaks before/after it*
		string			*jsonString = nullptr;	//	the JSON string copied from the source- doesn't include any comments before/after it
		string			*vertShaderSource = nullptr;	//	the raw vert shader source before being find-and-replaced
		string			*fragShaderSource = nullptr;	//	the raw frag shader source before being find-and-replaced
		
		ISFScene		*parentScene = nullptr;	//	nil by default, weak ref to the scene that "owns" me.  only non-nil when an ISFScene is using the doc to render.
		
	public:
		ISFDoc(const string & inPath, ISFScene * inParentScene=nullptr) throw(ISFErr);
		ISFDoc(const string & inFSContents, const string & inVSContents, ISFScene * inParentScene=nullptr);
		~ISFDoc();
		
		string getPath() const { return (path==nullptr) ? string("") : string(*path); }
		string getName() const { return (name==nullptr) ? string("") : string(*name); }
		string getDescription() const { return (description==nullptr) ? string("") : string(*description); }
		string getCredit() const { return (credit==nullptr) ? string("") : string(*credit); }
		ISFFileType getType() const { return type; }
		
		vector<string> & getCategories() { return categories; }
		vector<ISFAttrRef> & getInputs() { return inputs; }
		vector<ISFAttrRef> & getImageInputs() { return imageInputs; }
		vector<ISFAttrRef> & getAudioInputs() { return audioInputs; }
		vector<ISFAttrRef> & getImageImports() { return imageImports; }
		vector<ISFAttrRef> & getInputsOfType(const ISFValType & inInputType);
		
		//	this method must be called before rendering (passes/etc may have expressions that require the render dims to be evaluated)
		void evalBufferDimensionsWithRenderSize(const VVGL::Size & inSize);
		vector<ISFPassTargetRef> getPersistentBuffers() const { return persistentBuffers; }
		vector<ISFPassTargetRef> getTempBuffers() const { return tempBuffers; }
		vector<string> & getRenderPasses() { return renderPasses; }
		const VVGLBufferRef getBufferForKey(const string & n);
		const VVGLBufferRef getPersistentBufferForKey(const string & n);
		const VVGLBufferRef getTempBufferForKey(const string & n);
		const ISFPassTargetRef getPersistentTargetForKey(const string & n);
		const ISFPassTargetRef getTempTargetForKey(const string & n);
		
		string * getJSONSourceString() const { return jsonSourceString; }
		string * getJSONString() const { return jsonString; }
		string * getVertShaderSource() const { return vertShaderSource; }
		string * getFragShaderSource() const { return fragShaderSource; }
		void getJSONSourceString(string & outStr);
		void getJSONString(string & outStr);
		void getVertShaderSource(string & outStr);
		void getFragShaderSource(string & outStr);
		
		void setParentScene(ISFScene * n) { parentScene=n; }
		ISFScene * getParentScene() { return parentScene; }
		
		ISFAttrRef getInput(const string & inAttrName);
		vector<ISFAttrRef> getInputs(const ISFValType & n);
		
		ISFPassTargetRef getPersistentTargetBuffer(const string & n);
		ISFPassTargetRef getTempTargetBuffer(const string & n);
		
		//	returns a string describing the type of the expected texture samplers ("2" for 2D, "R" for Rect, "C" for Cube).  save this, if it changes in a later pass the shader source must be generated again.
		string generateTextureTypeString();
		//	returns a true if successful.  populates the provided strings with strings that are usable for frag/vert shaders
		bool generateShaderSource(string * outFragSrc, string * outVertSrc, GLVersion & inGLVers);
		
		friend ostream & operator<<(ostream & os, const ISFDoc & n);
		
	protected:
		//	used so we can have two constructors without duplicating code
		void _initWithRawFragShaderString(const string & inRawFile);
		//	returns a true if successful.  populates a string with variable declarations for a frag shader
		bool _assembleShaderSource_VarDeclarations(string * outVSString, string * outFSString, GLVersion & inGLVers);
		//	returns a true if successful.  populates a map with string/value pairs that will be used to evaluate variable names in strings
		bool _assembleSubstitutionMap(map<string,double*> * outMap);
};




}




#endif /* ISFDoc_hpp */
