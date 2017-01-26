#ifndef ISFScene_hpp
#define ISFScene_hpp

#include "ISFDoc.hpp"
#if ISF_TARGET_MAC
#import <TargetConditionals.h>
#endif




namespace VVISF
{


using namespace std;




class ISFScene : public VVGLShaderScene	{
	private:
		bool			throwExceptions = false;	//	NO by default
		
		mutex			propertyLock;	//	locks the below two vars
		//bool			loadingInProgress = false;
		ISFDocRef		doc;	//	the ISFDoc being used
		
		//	access to these vars should be restricted by the 'renderLock' var inherited from VVGLScene
		VVGL::Size		renderSize = size;	//	the last size at which i was requested to render a buffer (used to produce vals from normalized point inputs that need a render size to be used)
		Timestamper		timestamper;	//	used to generate time values, some of which get passed to the ISF scene
		uint32_t		renderFrameIndex = 0;	//	used to pass FRAMEINDEX to shaders
		double			renderTime = 0.;	//	this is the render time that gets passed to the ISF
		double			renderTimeDelta = 0.;	//	this is the render time delta (frame duration) which gets passed to the ISF
		uint32_t		passIndex = 1;	//	used to store the index of the rendered pass, which gets passed to the shader
		string			*compiledInputTypeString = nullptr;	//	a sequence of characters, either "2" or "R" or "C", one character for each input image. describes whether the shader was compiled to work with 2D textures or RECT textures or cube textures.
		
		//	access to these vars should be restricted by the 'renderLock' var inherited from VVGLScene
		int32_t			vertexAttribLoc = -1;	//	-1, or the location of the attribute in the compiled GL program for the vertex input
		int32_t			renderSizeUniformLoc = -1;	//	-1, or the location of the uniform var in the compiled GL program for the render size
		int32_t			passIndexUniformLoc = -1;	//	-1, or the location of the uniform var in the compiled GL program for the pass index
		int32_t			timeUniformLoc = -1;	//	-1, or the location of the uniform var in the compiled GL program for the time in seconds
		int32_t			timeDeltaUniformLoc = -1;	//	-1, or the location of the uniform var in the compiled GL program for time (in seconds) since the last frame was rendered
		int32_t			dateUniformLoc = -1;	//	-1, or the location of the uniform var in the compiled GL program for the date
		int32_t			renderFrameIndexUniformLoc = -1;	//	-1, or the location of the uniform var in the compiled GL program for the render frame index
		
		//	access to these vars should be restricted by the 'renderLock' var inherited from VVGLScene
		VVGLBufferRef		geoXYVBO = nullptr;
	
	public:
		ISFScene();
		ISFScene(const VVGLContext * inCtx);
		//ISFScene(const VVGLContext * inSharedCtx=nullptr, const VVGL::Size & inSize={640.,480.});
		virtual ~ISFScene();
		
		virtual void prepareToBeDeleted();
		
		void useFile(const string & inPath);
		string getFilePath();
		string getFileName();
		string getFileDescription();
		string getFileCredit();
		ISFFileType getFileType();
		vector<string> getFileCategories();
		
		void setBufferForInputNamed(const VVGLBufferRef & inBuffer, const string & inName);
		void setFilterInputBuffer(const VVGLBufferRef & inBuffer);
		void setBufferForInputImageKey(const VVGLBufferRef & inBuffer, const string & inString);
		void setBufferForAudioInputKey(const VVGLBufferRef & inBuffer, const string & inString);
		VVGLBufferRef getBufferForImageInput(const string & inKey);
		VVGLBufferRef getBufferForAudioInput(const string & inKey);
		VVGLBufferRef getPersistentBufferNamed(const string & inKey);
		VVGLBufferRef getTempBufferNamed(const string & inKey);
		
		void setValueForInputNamed(const ISFVal & inVal, const string & inName);
		ISFVal valueForInputNamed(const string & inName);
		
		virtual VVGLBufferRef createAndRenderABuffer(const VVGL::Size & inSize=VVGL::Size(640.,480.), const VVGLBufferPoolRef & inPool=GetGlobalBufferPool());
		
		void renderToBuffer(const VVGLBufferRef & inTargetBuffer, const VVGL::Size & inRenderSize, const double & inRenderTime, map<int32_t,VVGLBufferRef> * outPassDict);
		void renderToBuffer(const VVGLBufferRef & inTargetBuffer, const VVGL::Size & inRenderSize, const double & inRenderTime);
		void renderToBuffer(const VVGLBufferRef & inTargetBuffer, const VVGL::Size & inRenderSize);
		void renderToBuffer(const VVGLBufferRef & inTargetBuffer);
		
		virtual void setSize(const VVGL::Size & n);
		VVGL::Size getSize() const { return size; }
		VVGL::Size getRenderSize() const { return renderSize; }
		inline Timestamp getTimestamp() { return timestamper.nowTime(); }
		inline void setThrowExceptions(const bool & n) { throwExceptions=n; }
		
		//virtual void renderToBuffer(const VVGLBufferRef & inTargetBuffer, const VVGL::Size & inRenderSize=VVGL::Size(640.,480.), const double & inRenderTime=timestamper.nowTime().getTimeInSeconds(), map<string,VVGLBufferRef> * outPassDict=nullptr);
		
		ISFAttrRef getInputNamed(const string & inName);
		vector<ISFAttrRef> getImageInputs();
		vector<ISFAttrRef> getAudioInputs();
		vector<ISFAttrRef> getImageImports();
		
		inline ISFDocRef getDoc() { lock_guard<mutex> lock(propertyLock); return doc; }
		
		virtual void setVertexShaderString(const string & n);
		virtual void setFragmentShaderString(const string & n);
		
	protected:
		void _setUpRenderCallback();
		virtual void _renderPrep();
		virtual void _initialize();
		virtual void _renderCleanup();
		virtual void _render(const VVGLBufferRef & inTargetBuffer, const VVGL::Size & inSize, const double & inTime, map<int32_t,VVGLBufferRef> * outPassDict);
};




}




#endif /* ISFScene_hpp */
