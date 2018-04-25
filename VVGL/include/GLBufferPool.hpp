#ifndef VVGL_GLBufferPool_hpp
#define VVGL_GLBufferPool_hpp

#include "VVGL_Defines.hpp"

#include <mutex>

#include "GLBuffer.hpp"

#if defined(ISF_SDK_MAC)
#import <CoreGraphics/CoreGraphics.h>
#import <TargetConditionals.h>
#import <CoreMedia/CoreMedia.h>
#import <CoreVideo/CoreVideo.h>
#elif defined(ISF_SDK_IOS)
#import <CoreGraphics/CoreGraphics.h>
#import <CoreVideo/CoreVideo.h>
#endif




namespace VVGL	{


using namespace std;




//	this class defines a buffer pool.  the pool maintains its own GL context and creates/destroys the GL resources as needed
class GLBufferPool	{
	
	//	vars
	protected:
		bool				deleted = false;
		mutex				freeBuffersLock;
		vector<GLBufferRef>		freeBuffers;
		
		recursive_mutex		contextLock;
		GLContextRef		context = nullptr;
		
		Timestamper			timestamper;
		
#if defined(ISF_SDK_MAC) || defined(ISF_SDK_IOS)
		CGColorSpaceRef		colorSpace = nullptr;
#endif
		
	//	methods
	public:
		//GLBufferPool(const GLContext * inShareCtx=nullptr);
		GLBufferPool(const GLContextRef & inShareCtx=nullptr);
		virtual ~GLBufferPool();
		//	call this method to prep a pool for deletion- this will cause it to purge its contents, refuse to dispense more buffers, and automatically release any buffers that are returned to it
		inline void prepareToBeDeleted() { deleted=true; purge(); }
		
		GLBufferRef createBufferRef(const GLBuffer::Descriptor & desc, const Size & size={640,480}, const void * backingPtr=nullptr, const Size & backingSize={640,480}, const bool & createInCurrentContext=false);
		GLBufferRef fetchMatchingFreeBuffer(const GLBuffer::Descriptor & desc, const Size & size);
		
		//	call this periodically.  clears any buffers that have been sitting in the pool for a while.
		void housekeeping();
		//	call this to release all inactive buffers in the pool.
		void purge();
		inline Timestamp getTimestamp() const { return timestamper.nowTime(); };
		inline void timestampThisBuffer(const GLBufferRef & n) const { if (n == nullptr) return; n->contentTimestamp=timestamper.nowTime(); }
		inline GLContextRef & getContext() { return context; }
		inline recursive_mutex & getContextLock() { return contextLock; }
#if defined(ISF_SDK_MAC) || defined(ISF_SDK_IOS)
		inline CGColorSpaceRef getColorSpace() const { return colorSpace; }
#endif
		
		friend ostream & operator<<(ostream & os, const GLBufferPool & n);
		void flush();
	
	private:
		void returnBufferToPool(GLBuffer * inBuffer);
		void releaseBufferResources(GLBuffer * inBuffer);
		
		friend GLBuffer::~GLBuffer();
};




//	this is how you create a global buffer pool- pass in a context and it'll be used as a shared context, or pass in nothing and all the contexts will be created externally
//GLBufferPoolRef CreateGlobalBufferPool(const GLContext * inShareCtx=nullptr);
GLBufferPoolRef CreateGlobalBufferPool(const GLContextRef & inShareCtx=nullptr);
//GLBufferPoolRef GetGlobalBufferPool();
const GLBufferPoolRef & GetGlobalBufferPool();

//	these methods create GLBuffers that aren't actually images at all
GLBufferRef CreateVBO(const void * inBytes, const size_t & inByteSize, const int32_t & inUsage, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
GLBufferRef CreateEBO(const void * inBytes, const size_t & inByteSize, const int32_t & inUsage, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
GLBufferRef CreateVAO(const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());

//	these functions create buffers that are images/textures
GLBufferRef CreateRGBATex(const Size & size, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
GLBufferRef CreateRGBAFloatTex(const Size & size, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
GLBufferRef CreateRB(const Size & size, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
GLBufferRef CreateFBO(const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
GLBufferRef CreateDepthBuffer(const Size & size, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());

GLBufferRef CreateFromExistingGLTexture(const int32_t & inTexName, const int32_t & inTexTarget, const int32_t & inTexIntFmt, const int32_t & inTexPxlFmt, const int32_t & inTexPxlType, const Size & inTexSize, const bool & inTexFlipped, const Rect & inImgRectInTex, const void * inReleaseCallbackContext, const GLBuffer::BackingReleaseCallback & inReleaseCallback, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());

#if !defined(ISF_SDK_IOS)
GLBufferRef CreateBGRATex(const Size & size, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
GLBufferRef CreateBGRAFloatTex(const Size & size, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
#if !(ISF_SDK_RPI)
GLBufferRef CreateBGRAFloatCPUBackedTex(const Size & size, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
#endif	//	!ISF_SDK_RPI
#endif	//	!ISF_SDK_IOS


GLBufferRef CreateTexFromImage(const string & inPath, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
GLBufferRef CreateCubeTexFromImagePaths(const vector<string> & inPaths, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());


//	Qt-only func
#if defined(ISF_SDK_QT)
GLBufferRef CreateBufferForQImage(QImage * inImg, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
#endif


//	mac-only funcs
#if defined(ISF_SDK_MAC)
//	'inContextRef' must not be NULL- this function will have not work if it is null!
void PushTexRangeBufferRAMtoVRAM(const GLBufferRef & inBufferRef, const GLContextRef & inContextRef);
GLBufferRef CreateRGBATexIOSurface(const Size & inSize, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
GLBufferRef CreateRGBAFloatTexIOSurface(const Size & inSize, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
GLBufferRef CreateRGBATexFromIOSurfaceID(const IOSurfaceID & inID, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
GLBufferRef CreateBufferForCVPixelBuffer(CVPixelBufferRef & inCVPB, const bool & inTexRange, const bool & inIOSurface, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
GLBufferRef CreateTexRangeFromCMSampleBuffer(CMSampleBufferRef & n, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
GLBufferRef CreateRGBARectTex(const Size & size, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
#endif


//	mac & ios funcs
#if defined(ISF_SDK_MAC) || defined(ISF_SDK_IOS)
GLBufferRef CreateTexFromCGImageRef(const CGImageRef & n, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
GLBufferRef CreateCubeTexFromImages(const vector<CGImageRef> & inImgs, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
void CGBitmapContextUnpremultiply(CGContextRef ctx);
#endif

#if defined(ISF_SDK_MAC)
GLBufferRef CreateBufferForCVGLTex(CVOpenGLTextureRef & inTexRef, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
#elif defined(ISF_SDK_IOS)
GLBufferRef CreateBufferForCVGLTex(CVOpenGLESTextureRef & inTexRef, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
#endif


}


#endif /* VVGL_GLBufferPool_hpp */
