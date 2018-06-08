#ifndef VVGL_GLTexToTexCopier_hpp
#define VVGL_GLTexToTexCopier_hpp

#include "VVGL_Defines.hpp"

#include "GLScene.hpp"

#if defined(VVGL_SDK_MAC)
#import <TargetConditionals.h>
#endif




namespace VVGL
{




//! Copies the contents of one texture-type GLBuffer to another texture-type GLBuffer, capable of basic resizing.
/*!
\ingroup VVGL_BASIC
This object copies the image data in a GLBuffer by drawing it while another GLBuffer is bound as the render target.  This performs GL rendering- GLTexToTexCopier is a subclass of GLScene, so it has a GL context it can use.  If you require GLTexToTexCopier to use an existing GLContext to draw, use the constructor that accepts a GLContextRef (much like the GLScene constructor with the same signature)
*/
class VVGL_EXPORT GLTexToTexCopier : public GLScene	{
	private:
		bool			copyToIOSurface = false;
		bool			copyAndResize = false;
		Size			copySize = { 320., 240. };
		SizingMode		copySizingMode = SizingMode_Stretch;
		
		//GLBufferRef	geoXYVBO = nullptr;
		//GLBufferRef	geoSTVBO = nullptr;
		
#if defined(VVGL_TARGETENV_GL3PLUS) || defined(VVGL_TARGETENV_GLES3)
		GLBufferRef	vao = nullptr;	//	"owns" its own VBO, used to draw stuff if we're in GL 3
#elif defined(VVGL_TARGETENV_GLES)
		GLBufferRef	vbo = nullptr;	//	geometry + tex coords, used to draw stuff if we're in GL ES
#endif
		Quad<VertXYZST>		vboContents;	//	the VBO owned by 'vao' or the VBO 'vbo' is described by this var.  we check this, and if there's a delta the vao has to make a new vbo/'vbo' has to update its contents
		GLCachedAttrib	inputXYZLoc = GLCachedAttrib("inXYZ");	//	address of the attribute loc we pass geometry data to
		GLCachedAttrib	inputSTLoc = GLCachedAttrib("inST");	//	address of the attribute loc we pass tex coord data to
		GLCachedUni		inputImageLoc = GLCachedUni("inputImage");	//	address of the uniform loc we pass 2D texture IDs to
		GLCachedUni		inputImageRectLoc = GLCachedUni("inputImageRect");	//	address of the uniform loc we pass RECT texture IDs to
		GLCachedUni		isRectTexLoc = GLCachedUni("isRectTex");	//	address of the uniform we use to indicate whether the program should sample the 2D or RECT texture
		
	public:
		//!	Creates a new OpenGL context that shares the global buffer pool's context, uses that to create a new GLTexToTexCopier instance
		GLTexToTexCopier();
		//!	Uses the passed GL context to create a new GLTexToTexCopier.  No new OpenGL context is created- the buffer copier/scene will use the passed context to do its rendering.
		GLTexToTexCopier(const GLContextRef & inCtx);
		
		virtual ~GLTexToTexCopier();
		
		virtual void prepareToBeDeleted();
		
		//!	Sets the copyToIOSurface flag (Mac SDK only).  If true, buffers created by the copier will be backed by IOSurfaces (and can thus be shared with other processes)
		void setCopyToIOSurface(const bool & n);
		//!	Gets the copyToIOSurface flag (Mac SDK only).
		bool getCopyToIOSurface();
		//!	Sets the copyAndResize flag.  If true, the buffer copier will resize the buffers it copies to 'copySize'
		void setCopyAndResize(const bool & n);
		//!	Gets the copyAndResize flag.
		bool getCopyAndResize();
		//!	Sets the copySize value.  If 'copyAndResize' is true, the buffers the receiver copies will be resized to this size.
		void setCopySize(const Size & n);
		//!	Gets the copySize value.
		Size getCopySize();
		//!	The copy sizing mode governs how the buffers are resized if 'copySize' is true and the aspect ratio of source buffer doesn't match the target size's aspect ratio.
		void setCopySizingMode(const SizingMode & n);
		//!	Gets the copy sizing mode.
		SizingMode getCopySizingMode();
		
		//!	Returns a new GLBuffer which was made by rendering the passed buffer into a new texture of matching dimensions.
		GLBufferRef copyToNewBuffer(const GLBufferRef & n);
		//!	Copies the first passed buffer into the second, returns YES if successful- if sizes don't match or either buffer is nil, bails and returns NO!  ignores "copyToIOSurface" and "copyPixFormat"!
		bool copyFromTo(const GLBufferRef & a, const GLBufferRef & b);
		//!	Copies the first buffer into the second buffer.  will stretch/squash 'a' to fit into 'b'.
		void sizeVariantCopy(const GLBufferRef & a, const GLBufferRef & b);
		//!	Copies the first buffer into the second buffer, completely ignoring sizes- it just draws 'a' in the bottom-left corner of 'b'.  the resulting image may depict 'a' as being "too small" or "cropped".
		void ignoreSizeCopy(const GLBufferRef & a, const GLBufferRef & b);
		
		//!	Fills the passed buffer with transparent black (0., 0., 0., 0.)
		void copyBlackFrameTo(const GLBufferRef & n);
		//!	Fills the passed buffer with opaque black (0., 0., 0., 1.)
		void copyOpaqueBlackFrameTo(const GLBufferRef & n);
		//!	Fills the passed buffer with opaque red (1., 0., 0., 1.)
		void copyRedFrameTo(const GLBufferRef & n);
	
	protected:
		virtual void _initialize();
		void generalInit();
	
	private:
		//	acquire 'renderLock' and set current context before calling
		void _drawBuffer(const GLBufferRef & inBufferRef, const Quad<VertXYZST> & inVertexStruct);
};




/*!
\relatedalso GLTexToTexCopier
\brief Creates and returns a GLTexToTexCopier.  The scene makes a new GL context which shares the context of the global buffer pool.
*/
inline GLTexToTexCopierRef CreateGLTexToTexCopierRef() { return make_shared<GLTexToTexCopier>(); }
/*!
\relatedalso GLTexToTexCopier
\brief Creates and returns a GLTexToTexCopier.  The downloader uses the passed GL context to perform its GL operations.
*/
inline GLTexToTexCopierRef CreateGLTexToTexCopierRefUsing(const GLContextRef & inCtx) { return make_shared<GLTexToTexCopier>(inCtx); }




}

#endif /* VVGL_GLTexToTexCopier_hpp */
