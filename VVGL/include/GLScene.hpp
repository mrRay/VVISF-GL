#ifndef VVGL_GLScene_hpp
#define VVGL_GLScene_hpp

#include "VVGL_Defines.hpp"

#include <functional>
#include <mutex>
#include <map>
#include "GLBufferPool.hpp"
#include "GLCachedProperty.hpp"

#if defined(VVGL_SDK_MAC)
#import <TargetConditionals.h>
#endif




namespace VVGL	{


using namespace std;




/*!
\ingroup VVGL_BASIC
\brief Manages drawing in a GLContext, provides a simple interface for orthographic rendering and render-to-texture operations.

\detail A GLScene is a container for a GL context that lets you provide various callbacks which are executed when the scene is rendered.  The interface is geared towards making it easy to load frag/vert/geo shaders, perform orthographic projection, providing customized drawing code, subclassing, and rendering to textures/buffers/GLBuffers.  Used as a subclass of several other classes, also used in sample apps to perform GL rendering and output.
*/

class VVGL_EXPORT GLScene	{
	//	objects/structs/data types
	public:
		//!	This defines the interface for declaring a lambda as a member variable for encapsulating drawing code
		using RenderCallback = std::function<void(const GLScene &)>;
		using RenderPrepCallback = std::function<void(const GLScene &, const bool & sceneReshaped, const bool & pgmChanged)>;
		
		//	RenderTarget has all the attachments for the GL framebuffer, which need to be tracked so we can bind/unbind appropriately
		struct RenderTarget	{
				GLBufferRef	fbo = nullptr;
				GLBufferRef	color = nullptr;
				GLBufferRef	depth = nullptr;
			public:
				RenderTarget(){};
				RenderTarget(const GLBufferRef &f, const GLBufferRef &c, const GLBufferRef &d) { fbo=f;color=c;depth=d; };
		
				inline uint32_t fboName() const { return (fbo==nullptr) ? 0 : fbo->name; };
				inline uint32_t colorName() const { return (color==nullptr) ? 0 : color->name; };
				inline uint32_t colorTarget() const { return (color==nullptr) ? GL_TEXTURE_2D : color->desc.target; };
				inline uint32_t depthName() const { return (depth==nullptr) ? 0 : depth->name; };
				inline uint32_t depthTarget() const { return (depth==nullptr) ? GL_TEXTURE_2D : depth->desc.target; };
		};
	
	
	//	instance variables
	protected:
		recursive_mutex		renderLock;
		GLContextRef		context = nullptr;
		
		bool				deleted = false;
		bool				initialized = false;
		bool				needsReshape = true;
		bool				alwaysNeedsReshape = false;
		
		//	every time the scene is doing its render prep, this lambda is executed.  drawing setup code goes here.
		RenderPrepCallback		renderPrepCallback = nullptr;
		//	this callback gets hit immediately before the program is linked (after the shaders have been compiled & attached to the program, but before the program was linked).
		RenderCallback		renderPreLinkCallback = nullptr;
		//	every time the scene renders, this lambda is executed.  drawing code goes here.
		RenderCallback		renderCallback = nullptr;
		RenderCallback		renderCleanupCallback = nullptr;
		//	the render target contains the GL framebuffer and relevant attachments (render to texture/buffer/depth buffer/etc)
		RenderTarget		renderTarget;
		
		//	these vars pertain to optional default orthogonal sizing, which i use a lot for compositing/drawing 2D UIs with GL- if 'orthoUniId' is >= 0 then the vertex shader will create an orthogonal projection matrix of size 'orthoSize' and incorporating 'orthoFlipped'
		Size				orthoSize = { 0., 0. };
		bool				orthoFlipped = false;
		GLCachedUni			orthoUni = GLCachedUni("vvglOrthoProj");
		
		//	these vars pertain to whether or not the scene clears the attached framebuffer before drawing (and what color is used to clear)
		bool				performClear = true;
		GLColor				clearColor = GLColor(0., 0., 0., 0.);
		bool				clearColorUpdated = false;
		
		//	these vars pertain to the program being used by the GL context
		string				vsString = string("");
		string				gsString = string("");
		string				fsString = string("");
		bool				vsStringUpdated = false;
		bool				gsStringUpdated = false;
		bool				fsStringUpdated = false;
		uint32_t			program = 0;	//	0, or the compiled program
		uint32_t			vs = 0;
		uint32_t			gs = 0;
		uint32_t			fs = 0;
		mutex				errLock;
		mutex				errDictLock;
		map<string,string>		errDict;
	
	
	//	functions
	public:
		//!	Creates a new OpenGL context that shares the global buffer pool's context
		GLScene();
		//!	Creates a new GLScene instance, but not a new OpenGL context- instead it uses the passed GLContext.
		GLScene(const GLContextRef & inCtx);
		
		virtual ~GLScene();
		
		virtual void prepareToBeDeleted();
		
		virtual GLBufferRef createAndRenderABuffer(const Size & inSize=Size(640.,480.), const GLBufferPoolRef & inPool=GetGlobalBufferPool());
		virtual void renderToBuffer(const GLBufferRef & inBuffer);
		virtual void render(const RenderTarget & inRenderTarget=RenderTarget());
		virtual void renderBlackFrame(const RenderTarget & inRenderTarget=RenderTarget());
		virtual void renderOpaqueBlackFrame(const RenderTarget & inRenderTarget=RenderTarget());
		virtual void renderRedFrame(const RenderTarget & inRenderTarget=RenderTarget());
		
		inline GLContextRef getContext() const { return context; }
		
		//	getters/setters for the (optional) callbacks
		void setRenderPrepCallback(const RenderPrepCallback & n);
		void setRenderPreLinkCallback(const RenderCallback & n);
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
		virtual void setGeometryShaderString(const string & n);
		virtual string getGeometryShaderString();
		virtual void setFragmentShaderString(const string & n);
		virtual string getFragmentShaderString();
		inline uint32_t getProgram() const { return program; }
		
		inline GLVersion getGLVersion() const { if (context==nullptr) return GLVersion_Unknown; return context->version; }
		
	protected:
		virtual void _renderPrep();	//	assumed that _renderLock was obtained before calling.  assumed that context is non-null and has been set as current GL context before calling.
		virtual void _initialize();	//	assumed that _renderLock was obtained before calling.  assumed that context is non-null and has been set as current GL context before calling.
		virtual void _reshape();	//	assumed that _renderLock was obtained before calling.  assumed that context is non-null and has been set as current GL context before calling.
		virtual void _renderCleanup();	//	assumed that _renderLock was obtained before calling.  assumed that context is non-null and has been set as current GL context before calling.
};




inline GLSceneRef CreateGLSceneRef() { return make_shared<GLScene>(); }
inline GLSceneRef CreateGLSceneRefUsing(const GLContextRef & inCtx) { return make_shared<GLScene>(inCtx); }




}


#endif /* VVGL_GLScene_hpp */
