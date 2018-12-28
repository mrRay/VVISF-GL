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




//! Manages drawing in a GLContext, provides a simple interface for orthographic rendering, render-to-texture operations, and loading vert/geo/frag shaders.
/*!
\ingroup VVGL_BASIC
A GLScene is a container for a GL context that lets you provide various callbacks which are executed at specific times during rendering.  The interface is geared towards making it easy to load frag/vert/geo shaders, perform orthographic projection, providing customized drawing code, subclassing, and rendering to textures/buffers/GLBuffers.  Used as a subclass of several other classes, also used in sample apps to perform GL rendering and output.
*/

class VVGL_EXPORT GLScene	{
	//	objects/structs/data types
	public:
		/*!
		\brief	This defines the interface for a function/lambda that is used for encapsulating user-provided drawing code.  Both the render callback and the render cleanup callback use this type.
		\param inScene The scene executing the callback.
		*/
		using RenderCallback = std::function<void(const GLScene & inScene)>;
		/*!
		\brief This defines the interface for the render prep callback, which is executed prior to the render callback.
		\param inScene The scene executing the callback.
		\param inSceneReshaped True if the scene has been reshaped (if orthographic projection has been enabled/disabled or if the orthographic size changed).
		\param inPgmChanged True if the program has changed (if one of the shaders has been modified)
		*/
		using RenderPrepCallback = std::function<void(const GLScene & inScene, const bool & inSceneReshaped, const bool & inPgmChanged)>;
		
		//!	RenderTarget member vars are used to provide attachments for the GL framebuffer.  These buffers need to be tracked so they can be bound/unbound appropriately, which is what this structure is for.
		struct RenderTarget	{
				//!	Must be Type_FBO
				GLBufferRef	fbo = nullptr;
				//!	Probably Type_Tex, but anything that will function as a color attachment will work
				GLBufferRef	color = nullptr;
				//!	Probably Type_Tex, but anything that will function as a depth attachment will work
				GLBufferRef	depth = nullptr;
			public:
				RenderTarget(){};
				//!	Preferred constructor, populates all three attachments at once
				RenderTarget(const GLBufferRef &f, const GLBufferRef &c, const GLBufferRef &d) { fbo=f;color=c;depth=d; };
				
				//!	Returns the name of the FBO (or 0 if there's no FBO)
				inline uint32_t fboName() const { return (fbo==nullptr) ? 0 : fbo->name; };
				//!	Returns the name of the buffer/texture to be used as the color attachment (returns 0 if there's no color attachment)
				inline uint32_t colorName() const { return (color==nullptr) ? 0 : color->name; };
				//!	Returns the texture target of the buffer/texture to be used as the color attachment.
				inline uint32_t colorTarget() const { return (color==nullptr) ? GL_TEXTURE_2D : color->desc.target; };
				//!	Returns the name of the buffer/texture to be used as the depth attachment (returns 0 if there's no depth attachment)
				inline uint32_t depthName() const { return (depth==nullptr) ? 0 : depth->name; };
				//!	Returns the texture target of the buffer/texture to be used as the depth attachment.
				inline uint32_t depthTarget() const { return (depth==nullptr) ? GL_TEXTURE_2D : depth->desc.target; };
		};
	
	
	//	instance variables
	protected:
		recursive_mutex		_renderLock;
		GLContextRef		_context = nullptr;
		
		bool				_deleted = false;
		bool				_initialized = false;
		bool				_needsReshape = true;
		bool				_alwaysNeedsReshape = false;
		
		//	every time the scene is doing its render prep, this lambda is executed.  drawing setup code goes here.
		RenderPrepCallback		_renderPrepCallback = nullptr;
		//	this callback gets hit immediately before the program is linked (after the shaders have been compiled & attached to the program, but before the program was linked).
		RenderCallback		_renderPreLinkCallback = nullptr;
		//	every time the scene renders, this lambda is executed.  drawing code goes here.
		RenderCallback		_renderCallback = nullptr;
		RenderCallback		_renderCleanupCallback = nullptr;
		//	the render target contains the GL framebuffer and relevant attachments (render to texture/buffer/depth buffer/etc)
		RenderTarget		_renderTarget;
		
		//	these vars pertain to optional default orthogonal sizing, which i use a lot for compositing/drawing 2D UIs with GL- if 'orthoUniId' is >= 0 then the vertex shader will create an orthogonal projection matrix of size 'orthoSize' and incorporating 'orthoFlipped'
		Size				_orthoSize = { 0., 0. };
		bool				_orthoFlipped = false;
		GLCachedUni			_orthoUni = GLCachedUni("vvglOrthoProj");
		
		//	these vars pertain to whether or not the scene clears the attached framebuffer before drawing (and what color is used to clear)
		bool				_performClear = true;
		GLColor				_clearColor = GLColor(0., 0., 0., 0.);
		bool				_clearColorUpdated = false;
		
		//	these vars pertain to the program being used by the GL context
		string				*_vsString = nullptr;
		string				*_gsString = nullptr;
		string				*_fsString = nullptr;
		bool				_vsStringUpdated = false;
		bool				_gsStringUpdated = false;
		bool				_fsStringUpdated = false;
		uint32_t			_program = 0;	//	0, or the compiled program
		uint32_t			_vs = 0;
		uint32_t			_gs = 0;
		uint32_t			_fs = 0;
		mutex				_errLock;
		mutex				_errDictLock;
		map<string,string>		_errDict;
		
		//	this class- and subclasses of it- often need to create GPU resources.  by default the global buffer pool (GetGlobalBufferPool()) will be used- unless this var is non-null...
		GLBufferPoolRef		_privatePool = nullptr;	//	by default this is null and the scene will try to use the global buffer pool to create interim resources (temp/persistent buffers).  if non-null, the scene will use this pool to create interim resources.
		GLTexToTexCopierRef	_privateCopier = nullptr;	//	by default this is null and the scene will try to use the global buffer copier to create interim resources.  if non-null, the scene will use this copier to create interim resources.
	
	
	//	functions
	public:
		//!	Creates a new OpenGL context that shares the global buffer pool's context
		GLScene();
		//!	Creates a new GLScene instance, but not a new OpenGL context- instead it uses the passed GLContext.
		GLScene(const GLContextRef & inCtx);
		
		virtual ~GLScene();
		
		virtual void prepareToBeDeleted();
		
		/*!
		\name Render functions
		\brief These functions cause the scene to render.
		*/
		///@{
		
		//!	Creates an 8 bit per channel GL texture, uses it as a color attachment to the GL context and then renders into it.  Calls setOrthoSize() with the size of the image in the passed buffer.
		virtual GLBufferRef createAndRenderABuffer(const Size & inSize=Size(640.,480.), const GLBufferPoolRef & inPool=nullptr);
		//!	Uses the passed buffer as a color attachment to the GL context and then renders into it.  Calls setOrthoSize() with the size of the image in the passed buffer.
		virtual void renderToBuffer(const GLBufferRef & inBuffer);
		//!	Renders the GL scene, uses whatever attachments are present in the passed RenderTarget.  Doesn't call setOrthoSize() before rendering!
		virtual void render(const RenderTarget & inRenderTarget=RenderTarget());
		//!	Makes the scene render transparent black into whatever attachments are present in the passed RenderTarget.
		virtual void renderBlackFrame(const RenderTarget & inRenderTarget=RenderTarget());
		//!	Makes the scene render opaque black into whatever attachments are present in the passed RenderTarget.
		virtual void renderOpaqueBlackFrame(const RenderTarget & inRenderTarget=RenderTarget());
		//!	Makes the scene render opaque red into whatever attachments are present in the passed RenderTarget.
		virtual void renderRedFrame(const RenderTarget & inRenderTarget=RenderTarget());
		
		///@}
		
		
		//!	Returns the context used by this scene.
		inline GLContextRef context() const { return _context; }
		
		
		/*!
		\name Callback setup functions
		\brief These functions set the functions or lambdas to be executed at various stages during rendering.
		*/
		///@{
		
		//!	The render pre-link callback is executed if there's a shader after the shader/shaders have been successfully compiled, but before they've been linked.  Procedurally, this is the first callback to execute.  You probably don't want to perform any draw calls in this callback- this is a good time to configure your geometry shader.
		void setRenderPreLinkCallback(const RenderCallback & n);
		//!	The render prep callback is executed after any shaders have been compiled and linked, after the attachments have been made, and after the framebuffer has been cleared (assuming the scene is configured to perform a clear).  Procedurally, this is the second callback to execute, and the last to execute before draw calls are expected.  You probably don't want to perform any draw calls in this callback- this is a good time to do any ancillary setup outside of this scene that needs to occur before drawing begins.
		void setRenderPrepCallback(const RenderPrepCallback & n);
		//!	The render callback is expected to contain the drawing code that you want the scene to perform.  Procedurally, this is the third callback to execute.
		void setRenderCallback(const RenderCallback & n);
		//!	The render cleanup callback is executed immediately after the render callback.  Procedurally, this is the fourth and final callback to execute as a result of a render call.  You probably don't want to perform any draw calls in this callback- the context has already been flushed and there isn't a framebuffer any more.  This is a good time to do any ancillary teardown outside of this scene that needs to occur before you finish and return execution to whatever started rendering.
		void setRenderCleanupCallback(const RenderCallback & n);
		
		///@}
		
		
		/*!
		\name Viewport/camera setup
		\brief These functions configure orthogonal projection- GLScene was primarily written for 2d orthogonal rendering.
		*/
		///@{
		
		//!	Defaults to false.  If set to true, the viewport and orthogonal projection setup is applied every single frame (GL2 uses glOrtho, GL3+ uses a 4x4 matrix uniform in the shader to pass a projection transform to the program).  You can leave this false unless you plan on modifying the viewport or equivalent of projection matrices in your callbacks.
		void setAlwaysNeedsReshape(const bool & n);
		//!	Sets the orthogonal render size.  If the size is 0x0 then orthogonal rendering is skipped- any other positive values will cause the scene to configure itself for orthographic projection at the passed size during rendering.
		virtual void setOrthoSize(const Size & n);
		//!	Gets the orthogonal render size.  If the size is 0x0 then orthogonal rendering is disabled.
		Size orthoSize() const;
		//!	The _orthoFlipped member variable defaults to false- if it's set to true, the orthographic projection or equivalent will be flipped vertically.
		void setOrthoFlipped(const bool & n);
		//!	Gets the value of the _orthoFlipped member variable.  If this is true, the orthographic projection or equivalent will be flipped vertically.
		bool orthoFlipped() const;
		///@}
		
		
		/*!
		\name Clear color setup
		\brief These functions configure whether or not the scene will clear prior to rendering, and what color it will use to clear.
		*/
		///@{
		
		//!	Sets the clear color to the passed color.
		void setClearColor(const GLColor & n);
		//!	Sets the color color using the passed float values.
		void setClearColor(const float & r, const float & g, const float & b, const float & a);
		//!	Sets the clear color using a pointer to enough memory to contain four float values.
		void setClearColor(float * n);
		//!	Sets the '_performClear' member variable.  If it's true (the default) the context will clear to its clear color prior to drawing.  The clear is performed before the render prep callback.
		void setPerformClear(const bool & n);
		
		///@}
		
		
		//	getters/setters for program stuff
		/*!
		\name Shader setup
		\brief These functions get and set the various shaders this scene uses to render.
		*/
		///@{
		
		//!	Sets the vertex shader string.
		virtual void setVertexShaderString(const string & n);
		//!	Gets the vertex shader string.
		virtual string vertexShaderString();
		//!	Sets the geometry shader string.
		virtual void setGeometryShaderString(const string & n);
		//!	Gets the geometry shader string.
		virtual string geometryShaderString();
		//!	Sets the fragment shader string.
		virtual void setFragmentShaderString(const string & n);
		//!	Gets the fragment shader string.
		virtual string fragmentShaderString();
		//!	Gets the program ID.
		inline uint32_t program() const { return _program; }
		
		///@}
		
		
		//!	Returns the version of OpenGL currently being used by this scene's GL context.
		inline GLVersion glVersion() const { if (_context==nullptr) return GLVersion_Unknown; return _context->version; }
		
		//!	Sets the receiver's private buffer pool (default is null).  If non-null, this buffer pool will be used to generate any GL resources required by this scene.  Handy if you have a variety of GL contexts that aren't shared and you have to switch between them rapidly on a per-frame basis.
		void setPrivatePool(const GLBufferPoolRef & n) { _privatePool=n; }
		//!	Gets the receiver's private buffer pool- null by default, only non-null if something called setPrivatePool().
		GLBufferPoolRef privatePool() const { return _privatePool; }
		//!	Sets the receiver's private buffer copier (which should default to null).  If non-null, this copier will be used to copy any resources that need to be copied- like setPrivatePool(), handy if you have a variety of GL contexts that aren't shared and you have to switch between them rapidly on a per-frame basis.
		void setPrivateCopier(const GLTexToTexCopierRef & n) { _privateCopier=n; }
		//!	Gets the receiver's private buffer copier- null by default, only non-null if something called setPrivateCopier().
		GLTexToTexCopierRef privateCopier() const { return _privateCopier; }
		
	protected:
		//	assumed that _renderLock was obtained before calling.  assumed that context is non-null and has been set as current GL context before calling.
		virtual void _renderPrep();
		//	assumed that _renderLock was obtained before calling.  assumed that context is non-null and has been set as current GL context before calling.
		virtual void _initialize();
		//	assumed that _renderLock was obtained before calling.  assumed that context is non-null and has been set as current GL context before calling.
		virtual void _reshape();
		//	assumed that _renderLock was obtained before calling.  assumed that context is non-null and has been set as current GL context before calling.
		virtual void _renderCleanup();
};




/*!
\relatedalso GLScene
\brief Creates and returns a GLScene.  The scene makes a new GL context which shares the context of the global buffer pool.
*/
inline GLSceneRef CreateGLSceneRef() { return make_shared<GLScene>(); }
/*!
\relatedalso GLScene
\brief Creates and returns a GLScene.  The scene uses the passed GL context to do its drawing.
*/
inline GLSceneRef CreateGLSceneRefUsing(const GLContextRef & inCtx) { return make_shared<GLScene>(inCtx); }




}


#endif /* VVGL_GLScene_hpp */
