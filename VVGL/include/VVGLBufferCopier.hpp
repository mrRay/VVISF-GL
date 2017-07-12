#ifndef VVGLBufferCopier_hpp
#define VVGLBufferCopier_hpp

#include "VVGLScene.hpp"

#if ISF_TARGET_MAC
#import <TargetConditionals.h>
#endif




namespace VVGL
{




class VVGLBufferCopier : public VVGLScene	{
	private:
#if ISF_TARGET_MAC
		bool			copyToIOSurface = false;
#endif
		bool			copyAndResize = false;
		Size			copySize = { 320., 240. };
		SizingMode		copySizingMode = SizingMode_Stretch;
		
		//VVGLBufferRef	geoXYVBO = nullptr;
		//VVGLBufferRef	geoSTVBO = nullptr;
		
#if ISF_TARGET_GL3PLUS
		VVGLBufferRef	vao = nullptr;	//	"owns" its own VBO, used to draw stuff if we're in GL 3
#elif ISF_TARGET_GLES
		VVGLBufferRef	vbo = nullptr;	//	geometry + tex coords, used to draw stuff if we're in GL ES
#endif
		GLBufferQuadXYZST	vboContents;	//	the VBO owned by 'vao' or the VBO 'vbo' is described by this var.  we check this, and if there's a delta the vao has to make a new vbo/'vbo' has to update its contents
		VVGLCachedAttrib	inputXYZLoc = VVGLCachedAttrib("inXYZ");	//	address of the attribute loc we pass geometry data to
		VVGLCachedAttrib	inputSTLoc = VVGLCachedAttrib("inST");	//	address of the attribute loc we pass tex coord data to
		VVGLCachedUni		inputImageLoc = VVGLCachedUni("inputImage");	//	address of the uniform loc we pass 2D texture IDs to
		VVGLCachedUni		inputImageRectLoc = VVGLCachedUni("inputImageRect");	//	address of the uniform loc we pass RECT texture IDs to
		VVGLCachedUni		isRectTexLoc = VVGLCachedUni("isRectTex");	//	address of the uniform we use to indicate whether the program should sample the 2D or RECT texture
		
	public:
		//	creates a new GL context that shares the global buffer pool's context, uses that to create a new VVGLBufferCopier
		VVGLBufferCopier();
		//	uses the passed GL context to create a new VVGLBufferCopier
		VVGLBufferCopier(const VVGLContextRef & inCtx);
		
		virtual ~VVGLBufferCopier();
		
		virtual void prepareToBeDeleted();
		
#if ISF_TARGET_MAC
		void setCopyToIOSurface(const bool & n);
		bool getCopyToIOSurface();
#endif
		void setCopyAndResize(const bool & n);
		bool getCopyAndResize();
		void setCopySize(const Size & n);
		Size getCopySize();
		void setCopySizingMode(const SizingMode & n);
		SizingMode getCopySizingMode();
		
		///	returns a retained instance of VVGLBuffer which was made by rendering the passed buffer into a new texture of matching dimensions.
		VVGLBufferRef copyToNewBuffer(const VVGLBufferRef & n);
		///	copies the first passed buffer into the second, returns YES if successful- if sizes don't match or either buffer is nil, bails and returns NO!  ignores "copyToIOSurface" and "copyPixFormat"!
		bool copyFromTo(const VVGLBufferRef & a, const VVGLBufferRef & b);
		///	copies the first buffer into the second buffer.  will stretch/squash 'a' to fit into 'b'.
		void sizeVariantCopy(const VVGLBufferRef & a, const VVGLBufferRef & b);
		///	copies the first buffer into the second buffer, completely ignoring sizes- it just draws 'a' in the bottom-left corner of 'b'.  the resulting image may depict 'a' as being "too small" or "cropped".
		void ignoreSizeCopy(const VVGLBufferRef & a, const VVGLBufferRef & b);
		
		void copyBlackFrameTo(const VVGLBufferRef & n);
		void copyOpaqueBlackFrameTo(const VVGLBufferRef & n);
		void copyRedFrameTo(const VVGLBufferRef & n);
	
	protected:
		virtual void _initialize();
		void generalInit();
	
	private:
		//	acquire 'renderLock' and set current context before calling
		//void _drawBuffer(const VVGLBufferRef & inBufferRef, const Rect & inGLSrcRect, const Rect & inDstRect);
		void _drawBuffer(const VVGLBufferRef & inBufferRef, const GLBufferQuadXYZST & inVertexStruct);
};



VVGLBufferCopierRef CreateGlobalBufferCopier();
VVGLBufferCopierRef CreateGlobalBufferCopier(const VVGLContextRef & inCtx);
//inline VVGLBufferCopierRef GetGlobalBufferCopier() { return *_globalBufferCopier; }
VVGLBufferCopierRef GetGlobalBufferCopier();




}

#endif /* VVGLBufferCopier_hpp */
