#ifndef VVGLScene_hpp
#define VVGLScene_hpp

#include <functional>
#include <mutex>
#include <map>
#include "VVGLBufferPool.hpp"
#include "VVGLCachedUni.hpp"
#include "VVGLCachedAttrib.hpp"

#if ISF_TARGET_MAC
#import <TargetConditionals.h>
#endif




namespace VVGL	{


using namespace std;


class VVGLScene;




class VVGLScene	{
	//	objects/structs/data types
	public:
		//	this defines the interface for declaring a lambda as a member variable for encapsulating drawing code
		using RenderCallback = std::function<void(const VVGLScene &)>;
		using RenderPrepCallback = std::function<void(const VVGLScene &, const bool &, const bool &)>;
		
		//	RenderTarget has all the attachments for the GL framebuffer, which need to be tracked so we can bind/unbind appropriately
		struct RenderTarget	{
				VVGLBufferRef	fbo = nullptr;
				VVGLBufferRef	color = nullptr;
				VVGLBufferRef	depth = nullptr;
			public:
				RenderTarget(){};
				RenderTarget(const VVGLBufferRef &f, const VVGLBufferRef &c, const VVGLBufferRef &d) { fbo=f;color=c;depth=d; };
		
				inline uint32_t fboName() const { return (fbo==nullptr) ? 0 : fbo->name; };
				inline uint32_t colorName() const { return (color==nullptr) ? 0 : color->name; };
				inline uint32_t colorTarget() const { return (color==nullptr) ? GL_TEXTURE_2D : color->desc.target; };
				inline uint32_t depthName() const { return (depth==nullptr) ? 0 : depth->name; };
				inline uint32_t depthTarget() const { return (depth==nullptr) ? GL_TEXTURE_2D : depth->desc.target; };
		};
	
	
	//	instance variables
	protected:
		recursive_mutex		renderLock;
		VVGLContextRef		context = nullptr;
		
		bool				deleted = false;
		bool				initialized = false;
		bool				needsReshape = true;
		bool				alwaysNeedsReshape = false;
		
		RenderPrepCallback		renderPrepCallback = nullptr;	//	every time the scene is doing its render prep, this lambda is executed.  drawing setup code goes here.
		RenderCallback		renderCallback = nullptr;	//	every time the scene renders, this lambda is executed.  drawing code goes here.
		RenderCallback		renderCleanupCallback = nullptr;
		RenderTarget		renderTarget;	//	the render target contains the GL framebuffer and relevant attachments (render to texture/buffer/depth buffer/etc)
		
		//	these vars pertain to optional default orthogonal sizing, which i use a lot for compositing/drawing 2D UIs with GL- if 'orthoUniId' is >= 0 then the vertex shader will create an orthogonal projection matrix of size 'orthoSize' and incorporating 'orthoFlipped'
		Size				orthoSize = { 0., 0. };
		bool				orthoFlipped = false;
		VVGLCachedUni		orthoUni = VVGLCachedUni("vvglOrthoProj");
		//int32_t				orthoUniLoc = -1;
		
		//	these vars pertain to whether or not the scene clears the attached framebuffer before drawing (and what color is used to clear)
		bool				performClear = true;
		GLColor				clearColor = GLColor(0., 0., 0., 0.);
		bool				clearColorUpdated = false;
		
		//	these vars pertain to the program being used by the GL context
		string				vsString = string("");
		string				fsString = string("");
		bool				vsStringUpdated = false;
		bool				fsStringUpdated = false;
		uint32_t			program = 0;	//	0, or the compiled program
		uint32_t			vs = 0;
		uint32_t			fs = 0;
		mutex				errLock;
		mutex				errDictLock;
		map<string,string>		errDict;
	
	
	//	functions
	public:
		//	creates a new GL context that shares the global buffer pool's context
		VVGLScene();
		//	uses the passed context- doesn't create any new GL contexts (just retains what was passed to it)
		VVGLScene(const VVGLContextRef & inCtx);
		
		//VVGLScene(const VVGLContext * inSharedCtx, const Size & inSize={640.,480.});
		virtual ~VVGLScene();
		
		virtual void prepareToBeDeleted();
		
		virtual VVGLBufferRef createAndRenderABuffer(const Size & inSize=Size(640.,480.), const VVGLBufferPoolRef & inPool=GetGlobalBufferPool());
		virtual void renderToBuffer(const VVGLBufferRef & inBuffer);
		virtual void render(const RenderTarget & inRenderTarget=RenderTarget());
		virtual void renderBlackFrame(const RenderTarget & inRenderTarget=RenderTarget());
		virtual void renderOpaqueBlackFrame(const RenderTarget & inRenderTarget=RenderTarget());
		virtual void renderRedFrame(const RenderTarget & inRenderTarget=RenderTarget());
		
		inline VVGLContextRef getContext() const { return context; }
		
		//	getters/setters for the (optional) callbacks
		void setRenderPrepCallback(const RenderPrepCallback & n);
		void setRenderCallback(const RenderCallback & n);
		void setRenderCleanupCallback(const RenderCallback & n);
		
		//	getters/setters for orthogonal projection
		void setAlwaysNeedsReshape(const bool & n);
		virtual void setOrthoSize(const Size & n);
		Size getOrthoSize() const;
		void setOrthoFlipped(const bool & n);
		bool getOrthoFlipped() const;
		
		//	setters for clear color
		void setClearColor(const GLColor & n);
		void setClearColor(const float & r, const float & g, const float & b, const float & a);
		void setClearColor(float * n);
		void setPerformClear(const bool & n);
		
		//	getters/setters for program stuff
		virtual void setVertexShaderString(const string & n);
		virtual string getVertexShaderString();
		virtual void setFragmentShaderString(const string & n);
		virtual string getFragmentShaderString();
		inline uint32_t getProgram() const { return program; }
		
		GLVersion getGLVersion() const { if (context==nullptr) return GLVersion_Unknown; return context->version; }
		
	protected:
		virtual void _renderPrep();	//	assumed that _renderLock was obtained before calling.  assumed that context is non-null and has been set as current GL context before calling.
		virtual void _initialize();	//	assumed that _renderLock was obtained before calling.  assumed that context is non-null and has been set as current GL context before calling.
		virtual void _reshape();	//	assumed that _renderLock was obtained before calling.  assumed that context is non-null and has been set as current GL context before calling.
		virtual void _renderCleanup();	//	assumed that _renderLock was obtained before calling.  assumed that context is non-null and has been set as current GL context before calling.
};




}


#endif /* VVGLScene_hpp */
