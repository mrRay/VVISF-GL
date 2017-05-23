#ifndef VVGLBufferCopier_hpp
#define VVGLBufferCopier_hpp

#if !ISF_TARGET_RPI
#include "VVGLScene.hpp"
#else
#include "VVGLShaderScene.hpp"
#endif

#if ISF_TARGET_MAC
#import <TargetConditionals.h>
#endif




namespace VVGL
{




class VVGLBufferCopier : 
#if !ISF_TARGET_RPI
public VVGLScene
#else
public VVGLShaderScene
#endif
{
	private:
#if ISF_TARGET_MAC
		bool			copyToIOSurface = false;
#endif
		bool			copyAndResize = false;
		Size			copySize = { 320., 240. };
		SizingMode		copySizingMode = SizingMode_Stretch;
		
		VVGLBufferRef	geoXYVBO = nullptr;
		VVGLBufferRef	geoSTVBO = nullptr;
	
	public:
		//	creates a new GL context that shares the global buffer pool's context, uses that to create a new VVGLBufferCopier
		VVGLBufferCopier();
		//	uses the passed GL context to create a new VVGLBufferCopier
		VVGLBufferCopier(const VVGLContextRef & inCtx);
		
		virtual ~VVGLBufferCopier();
		
		virtual void prepareToBeDeleted();
		
		void setCopyToIOSurface(const bool & n);
		bool getCopyToIOSurface();
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
		void _drawBuffer(const VVGLBufferRef & inBufferRef, const Rect & inGLSrcRect, const Rect & inDstRect);
};



VVGLBufferCopierRef CreateGlobalBufferCopier();
//inline VVGLBufferCopierRef GetGlobalBufferCopier() { return *_globalBufferCopier; }
VVGLBufferCopierRef GetGlobalBufferCopier();




}

#endif /* VVGLBufferCopier_hpp */
