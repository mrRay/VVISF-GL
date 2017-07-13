#ifndef VVGLBufferPool_hpp
#define VVGLBufferPool_hpp

#include <mutex>

#include "VVGLBuffer.hpp"

#if ISF_TARGET_MAC
#import <CoreGraphics/CoreGraphics.h>
#import <TargetConditionals.h>
#elif ISF_TARGET_IOS
#import <CoreGraphics/CoreGraphics.h>
#endif




namespace VVGL	{


using namespace std;




//	this class defines a buffer pool.  the pool maintains its own GL context and creates/destroys the GL resources as needed
class VVGLBufferPool	{
	
	//	vars
	protected:
		bool				deleted = false;
		mutex				freeBuffersLock;
		vector<VVGLBufferRef>		freeBuffers;
		
		recursive_mutex		contextLock;
		VVGLContextRef		context = nullptr;
		
		Timestamper			timestamper;
		
#if ISF_TARGET_MAC || ISF_TARGET_IOS
		CGColorSpaceRef		colorSpace = nullptr;
#endif
		
	//	methods
	public:
		//VVGLBufferPool(const VVGLContext * inShareCtx=nullptr);
		VVGLBufferPool(const VVGLContextRef & inShareCtx=nullptr);
		virtual ~VVGLBufferPool();
		//	call this method to prep a pool for deletion- this will cause it to purge its contents, refuse to dispense more buffers, and automatically release any buffers that are returned to it
		inline void prepareToBeDeleted() { deleted=true; purge(); }
		
		VVGLBufferRef createBufferRef(const VVGLBuffer::Descriptor & desc, const Size & size={640,480}, const void * backingPtr=nullptr, const Size & backingSize={640,480}, const bool & createInCurrentContext=false);
		VVGLBufferRef fetchMatchingFreeBuffer(const VVGLBuffer::Descriptor & desc, const Size & size);
		
		//	call this periodically.  clears any buffers that have been sitting in the pool for a while.
		void housekeeping();
		//	call this to release all inactive buffers in the pool.
		void purge();
		inline Timestamp getTimestamp() const { return timestamper.nowTime(); };
		inline void timestampThisBuffer(const VVGLBufferRef & n) const { if (n == nullptr) return; n->contentTimestamp=timestamper.nowTime(); }
		inline VVGLContextRef & getContext() { return context; }
		inline recursive_mutex & getContextLock() { return contextLock; }
#if ISF_TARGET_MAC || ISF_TARGET_IOS
		inline CGColorSpaceRef getColorSpace() const { return colorSpace; }
#endif
		
		friend ostream & operator<<(ostream & os, const VVGLBufferPool & n);
		void flush();
	
	private:
		void returnBufferToPool(VVGLBuffer * inBuffer);
		void releaseBufferResources(VVGLBuffer * inBuffer);
		
		friend VVGLBuffer::~VVGLBuffer();
};




//	this is how you create a global buffer pool- pass in a context and it'll be used as a shared context, or pass in nothing and all the contexts will be created externally
//VVGLBufferPoolRef CreateGlobalBufferPool(const VVGLContext * inShareCtx=nullptr);
VVGLBufferPoolRef CreateGlobalBufferPool(const VVGLContextRef & inShareCtx=nullptr);
VVGLBufferPoolRef GetGlobalBufferPool();

//	these functions create buffers
VVGLBufferRef CreateRGBATex(const Size & size, const bool & createInCurrentContext=false, const VVGLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
VVGLBufferRef CreateRGBAFloatTex(const Size & size, const bool & createInCurrentContext=false, const VVGLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
VVGLBufferRef CreateRB(const Size & size, const bool & createInCurrentContext=false, const VVGLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
VVGLBufferRef CreateFBO(const bool & createInCurrentContext=false, const VVGLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
VVGLBufferRef CreateDepthBuffer(const Size & size, const bool & createInCurrentContext=false, const VVGLBufferPoolRef & inPoolRef=GetGlobalBufferPool());

VVGLBufferRef CreateFromExistingGLTexture(const int32_t & inTexName, const int32_t & inTexTarget, const int32_t & inTexIntFmt, const int32_t & inTexPxlFmt, const int32_t & inTexPxlType, const Size & inTexSize, const bool & inTexFlipped, const Rect & inImgRectInTex, const void * inReleaseCallbackContext, const VVGLBuffer::BackingReleaseCallback & inReleaseCallback, const VVGLBufferPoolRef & inPoolRef=GetGlobalBufferPool());

#if ISF_TARGET_MAC
VVGLBufferRef CreateRGBARectTex(const Size & size, const bool & createInCurrentContext=false, const VVGLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
#endif

#if !ISF_TARGET_IOS
VVGLBufferRef CreateBGRATex(const Size & size, const bool & createInCurrentContext=false, const VVGLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
VVGLBufferRef CreateBGRAFloatTex(const Size & size, const bool & createInCurrentContext=false, const VVGLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
#endif

//	these methods create VVGLBuffers that aren't actually images at all
VVGLBufferRef CreateVBO(const void * inBytes, const size_t & inByteSize, const int32_t & inUsage, const bool & createInCurrentContext=false, const VVGLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
VVGLBufferRef CreateEBO(const void * inBytes, const size_t & inByteSize, const int32_t & inUsage, const bool & createInCurrentContext=false, const VVGLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
#if ISF_TARGET_GL3PLUS || ISF_TARGET_GLES3
VVGLBufferRef CreateVAO(const bool & createInCurrentContext=false, const VVGLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
#endif



VVGLBufferRef CreateTexFromImage(const string & inPath, const bool & createInCurrentContext=false, const VVGLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
#if ISF_TARGET_MAC || ISF_TARGET_IOS
VVGLBufferRef CreateTexFromCGImageRef(const CGImageRef & n, const bool & createInCurrentContext=false, const VVGLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
#endif

VVGLBufferRef CreateCubeTexFromImagePaths(const vector<string> & inPaths, const bool & createInCurrentContext=false, const VVGLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
#if ISF_TARGET_MAC || ISF_TARGET_IOS
VVGLBufferRef CreateCubeTexFromImages(const vector<CGImageRef> & inPaths, const bool & createInCurrentContext=false, const VVGLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
#endif

#if ISF_TARGET_MAC
VVGLBufferRef CreateRGBATexIOSurface(const Size & inSize, const bool & createInCurrentContext=false, const VVGLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
VVGLBufferRef CreateRGBAFloatTexIOSurface(const Size & inSize, const bool & createInCurrentContext=false, const VVGLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
VVGLBufferRef CreateRGBATexFromIOSurfaceID(const IOSurfaceID & inID, const bool & createInCurrentContext=false, const VVGLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
#endif


#if ISF_TARGET_MAC || ISF_TARGET_IOS
void CGBitmapContextUnpremultiply(CGContextRef ctx);
#endif


}


#endif /* VVGLBufferPool_hpp */
