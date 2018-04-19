#ifndef VVGL_GLBufferCopier_hpp
#define VVGL_GLBufferCopier_hpp

#include "VVGL_Defines.hpp"

#include "GLScene.hpp"

#if ISF_SDK_MAC
#import <TargetConditionals.h>
#endif




namespace VVGL
{




class GLBufferCopier : public GLScene	{
	private:
		bool			copyToIOSurface = false;
		bool			copyAndResize = false;
		Size			copySize = { 320., 240. };
		SizingMode		copySizingMode = SizingMode_Stretch;
		
		//GLBufferRef	geoXYVBO = nullptr;
		//GLBufferRef	geoSTVBO = nullptr;
		
#if defined(ISF_TARGETENV_GL3PLUS) || defined(ISF_TARGETENV_GLES3)
		GLBufferRef	vao = nullptr;	//	"owns" its own VBO, used to draw stuff if we're in GL 3
#elif defined(ISF_TARGETENV_GLES)
		GLBufferRef	vbo = nullptr;	//	geometry + tex coords, used to draw stuff if we're in GL ES
#endif
		Quad<VertXYZST>		vboContents;	//	the VBO owned by 'vao' or the VBO 'vbo' is described by this var.  we check this, and if there's a delta the vao has to make a new vbo/'vbo' has to update its contents
		GLCachedAttrib	inputXYZLoc = GLCachedAttrib("inXYZ");	//	address of the attribute loc we pass geometry data to
		GLCachedAttrib	inputSTLoc = GLCachedAttrib("inST");	//	address of the attribute loc we pass tex coord data to
		GLCachedUni		inputImageLoc = GLCachedUni("inputImage");	//	address of the uniform loc we pass 2D texture IDs to
		GLCachedUni		inputImageRectLoc = GLCachedUni("inputImageRect");	//	address of the uniform loc we pass RECT texture IDs to
		GLCachedUni		isRectTexLoc = GLCachedUni("isRectTex");	//	address of the uniform we use to indicate whether the program should sample the 2D or RECT texture
		
	public:
		//	creates a new GL context that shares the global buffer pool's context, uses that to create a new GLBufferCopier
		GLBufferCopier();
		//	uses the passed GL context to create a new GLBufferCopier
		GLBufferCopier(const GLContextRef & inCtx);
		
		virtual ~GLBufferCopier();
		
		virtual void prepareToBeDeleted();
		
		void setCopyToIOSurface(const bool & n);
		bool getCopyToIOSurface();
		void setCopyAndResize(const bool & n);
		bool getCopyAndResize();
		void setCopySize(const Size & n);
		Size getCopySize();
		void setCopySizingMode(const SizingMode & n);
		SizingMode getCopySizingMode();
		
		///	returns a retained instance of GLBuffer which was made by rendering the passed buffer into a new texture of matching dimensions.
		GLBufferRef copyToNewBuffer(const GLBufferRef & n);
		///	copies the first passed buffer into the second, returns YES if successful- if sizes don't match or either buffer is nil, bails and returns NO!  ignores "copyToIOSurface" and "copyPixFormat"!
		bool copyFromTo(const GLBufferRef & a, const GLBufferRef & b);
		///	copies the first buffer into the second buffer.  will stretch/squash 'a' to fit into 'b'.
		void sizeVariantCopy(const GLBufferRef & a, const GLBufferRef & b);
		///	copies the first buffer into the second buffer, completely ignoring sizes- it just draws 'a' in the bottom-left corner of 'b'.  the resulting image may depict 'a' as being "too small" or "cropped".
		void ignoreSizeCopy(const GLBufferRef & a, const GLBufferRef & b);
		
		void copyBlackFrameTo(const GLBufferRef & n);
		void copyOpaqueBlackFrameTo(const GLBufferRef & n);
		void copyRedFrameTo(const GLBufferRef & n);
	
	protected:
		virtual void _initialize();
		void generalInit();
	
	private:
		//	acquire 'renderLock' and set current context before calling
		void _drawBuffer(const GLBufferRef & inBufferRef, const Quad<VertXYZST> & inVertexStruct);
};



GLBufferCopierRef CreateGlobalBufferCopier();
GLBufferCopierRef CreateGlobalBufferCopier(const GLContextRef & inCtx);
//inline GLBufferCopierRef GetGlobalBufferCopier() { return *_globalBufferCopier; }
GLBufferCopierRef GetGlobalBufferCopier();




}

#endif /* VVGL_GLBufferCopier_hpp */
