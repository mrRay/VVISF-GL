#ifndef VVGL_GLBufferPool_hpp
#define VVGL_GLBufferPool_hpp

#include "VVGL_Defines.hpp"

#include <mutex>

#include "GLBuffer.hpp"

#if defined(VVGL_SDK_MAC)
#import <CoreGraphics/CoreGraphics.h>
#import <TargetConditionals.h>
#import <CoreMedia/CoreMedia.h>
#import <CoreVideo/CoreVideo.h>
#elif defined(VVGL_SDK_IOS)
#import <CoreGraphics/CoreGraphics.h>
#import <CoreVideo/CoreVideo.h>
#endif




namespace VVGL	{


using namespace std;




/*!
\ingroup VVGL_BASIC
\brief This class defines a buffer pool- an object which creates GL resources, and then optionally pools them for re-use instead of destroying them.

\detail The pool creates and maintains its own GL context so it can create/destroy the GL resources (the default behavior, but note that a pool can also create buffers whose GL contents were created in/by another context).

Notes:
- For convenience, VVGL defines a single global buffer pool.  This global buffer pool is used by most of the examples, and is passed as a default argument to any functions that create buffers.  If you don't create a global buffer pool then you'll need to explicitly create a buffer pool and pass it to the create functions!
- GLBuffer instances are created using the various creation functions listed here \ref VVGL_BUFFERCREATE.  Note that these functions all require a buffer pool- if you have a global buffer pool then using them will be a lot simpler.
- You must call the housekeeping() member function on your buffer pools periodically!
*/
class VVGL_EXPORT GLBufferPool	{
	
	//	vars
	protected:
		bool				deleted = false;
		mutex				freeBuffersLock;
		vector<GLBufferRef>		freeBuffers;
		
		recursive_mutex		contextLock;
		GLContextRef		context = nullptr;
		
		Timestamper			timestamper;
		
#if defined(VVGL_SDK_MAC) || defined(VVGL_SDK_IOS)
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
		
		//!	You must call this periodically (once per render loop after you finish drawing is usually a good time to call this).  This function clears any buffers that have been sitting unused in the pool for "too long".
		void housekeeping();
		//!	If needed you can call this to release all inactive buffers in the pool.
		void purge();
		//!	Returns a timestamp generated for the current time
		inline Timestamp getTimestamp() const { return timestamper.nowTime(); };
		//!	Timestamps the passed buffer with the current time
		inline void timestampThisBuffer(const GLBufferRef & n) const { if (n == nullptr) return; n->contentTimestamp=timestamper.nowTime(); }
		//!	Returns the GLContextRef used by the buffer pool to create and destroy its GL resources.
		inline GLContextRef & getContext() { return context; }
		inline recursive_mutex & getContextLock() { return contextLock; }
#if defined(VVGL_SDK_MAC) || defined(VVGL_SDK_IOS)
		inline CGColorSpaceRef getColorSpace() const { return colorSpace; }
#endif
		
		friend ostream & operator<<(ostream & os, const GLBufferPool & n);
		void flush();
	
	private:
		//	Called by GLBuffer when it's being deallocated if the buffer has determined that it is a candidate for recycling
		void returnBufferToPool(GLBuffer * inBuffer);
		//	Called by GLBuffer when it's being deallocated if the buffer has determined that its GL resources need to be released immediately
		void releaseBufferResources(GLBuffer * inBuffer);
		
		friend GLBuffer::~GLBuffer();
};




/*!
\brief This is how you create the global buffer pool.
\param inShareCtx The buffer pool that is created will share this context (textures and buffers created by the buffer pool will be compatible with all other GL contexts in the same sharegroup).
*/
VVGL_EXPORT GLBufferPoolRef CreateGlobalBufferPool(const GLContextRef & inShareCtx=nullptr);
/*!
\brief This is how you retrieve the global buffer pool.
*/
VVGL_EXPORT const GLBufferPoolRef & GetGlobalBufferPool();




//	these methods create GLBuffers that aren't actually images at all
/*!
\ingroup VVGL_BUFFERCREATE
\brief Creates and returns a framebuffer object (FBO).
\param createInCurrentContext If true, the GL resource will be created in the current context (assumes that a GL context is active in the current thread).  If false, the GL resource will be created by the GL context owned by the buffer pool.
\param inPoolRef The pool that the GLBuffer should be created with.  When the GLBuffer is freed, its underlying GL resources will be returned to this pool (where they will be either freed or recycled).
*/
VVGL_EXPORT GLBufferRef CreateFBO(const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
/*!
\ingroup VVGL_BUFFERCREATE
\brief Creates and returns a vertex buffer object (VBO).
\param inBytes The memory at this pointer will be used to populate the VBO.
\param inByteSize The size of the memory available at 'inBytes'.
\param inUsage How the VBO is intended to be used.  One of several GL vars, like GL_STATIC_DRAW, GL_STREAM_DRAW, etc.
\param createInCurrentContext If true, the GL resource will be created in the current context (assumes that a GL context is active in the current thread).  If false, the GL resource will be created by the GL context owned by the buffer pool.
\param inPoolRef The pool that the GLBuffer should be created with.  When the GLBuffer is freed, its underlying GL resources will be returned to this pool (where they will be either freed or recycled).
*/
VVGL_EXPORT GLBufferRef CreateVBO(const void * inBytes, const size_t & inByteSize, const int32_t & inUsage, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
/*!
\ingroup VVGL_BUFFERCREATE
\brief Creates and returns an element buffer object (EBO).
\param inBytes The memory at this pointer will be used to populate the VBO.
\param inByteSize The size of the memory available at 'inBytes'.
\param inUsage How the VBO is intended to be used.  One of several GL vars, like GL_STATIC_DRAW, GL_STREAM_DRAW, etc.
\param createInCurrentContext If true, the GL resource will be created in the current context (assumes that a GL context is active in the current thread).  If false, the GL resource will be created by the GL context owned by the buffer pool.
\param inPoolRef The pool that the GLBuffer should be created with.  When the GLBuffer is freed, its underlying GL resources will be returned to this pool (where they will be either freed or recycled).
*/
VVGL_EXPORT GLBufferRef CreateEBO(const void * inBytes, const size_t & inByteSize, const int32_t & inUsage, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
/*!
\ingroup VVGL_BUFFERCREATE
\brief Creates and returns a vertex attribte object (VAO).
\param createInCurrentContext If true, the GL resource will be created in the current context (assumes that a GL context is active in the current thread).  If false, the GL resource will be created by the GL context owned by the buffer pool.
\param inPoolRef The pool that the GLBuffer should be created with.  When the GLBuffer is freed, its underlying GL resources will be returned to this pool (where they will be either freed or recycled).
*/
VVGL_EXPORT GLBufferRef CreateVAO(const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());




//	these functions create buffers that are images/textures
/*!
\ingroup VVGL_BUFFERCREATE
\brief Creates and returns an OpenGL texture that has an internal RGBA format and is 8 bits per component (32 bit color).
\param size The size of the buffer to create (in pixels).
\param createInCurrentContext If true, the GL resource will be created in the current context (assumes that a GL context is active in the current thread).  If false, the GL resource will be created by the GL context owned by the buffer pool.
\param inPoolRef The pool that the GLBuffer should be created with.  When the GLBuffer is freed, its underlying GL resources will be returned to this pool (where they will be either freed or recycled).
*/
VVGL_EXPORT GLBufferRef CreateRGBATex(const Size & size, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
/*!
\ingroup VVGL_BUFFERCREATE
\brief Creates and returns an OpenGL texture that has an internal RGBA format and is 32 bits per component (128 bit color).
\param size The size of the buffer to create (in pixels).
\param createInCurrentContext If true, the GL resource will be created in the current context (assumes that a GL context is active in the current thread).  If false, the GL resource will be created by the GL context owned by the buffer pool.
\param inPoolRef The pool that the GLBuffer should be created with.  When the GLBuffer is freed, its underlying GL resources will be returned to this pool (where they will be either freed or recycled).
*/
VVGL_EXPORT GLBufferRef CreateRGBAFloatTex(const Size & size, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
/*!
\ingroup VVGL_BUFFERCREATE
\brief Creates and returns an OpenGL renderbuffer.
\param size The size of the buffer to create (in pixels).
\param createInCurrentContext If true, the GL resource will be created in the current context (assumes that a GL context is active in the current thread).  If false, the GL resource will be created by the GL context owned by the buffer pool.
\param inPoolRef The pool that the GLBuffer should be created with.  When the GLBuffer is freed, its underlying GL resources will be returned to this pool (where they will be either freed or recycled).
*/
VVGL_EXPORT GLBufferRef CreateRB(const Size & size, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
/*!
\ingroup VVGL_BUFFERCREATE
\brief Creates and returns an OpenGL texture configured to be a 16 bit per pixel depth buffer
\param size The size of the buffer to create (in pixels).
\param createInCurrentContext If true, the GL resource will be created in the current context (assumes that a GL context is active in the current thread).  If false, the GL resource will be created by the GL context owned by the buffer pool.
\param inPoolRef The pool that the GLBuffer should be created with.  When the GLBuffer is freed, its underlying GL resources will be returned to this pool (where they will be either freed or recycled).
*/
VVGL_EXPORT GLBufferRef CreateDepthBuffer(const Size & size, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
/*!
\ingroup VVGL_BUFFERCREATE
\brief Creates and returns a GLBuffer that wraps an existing OpenGL texture that was created by another SDK.  This does not take ownership of the texture- instead, you supply a backing release callback that will be executed when the GLBuffer is freed.
\param inTexName The name of the OpenGL texture that will be used to populate the GLBuffer.
\param inTexTarget The texture target of the OpenGL texture (GLBuffer::Target)
\param inTexIntFmt The internal format of the OpenGL texture.  Not as important to get right, used primarily for creating the texture.
\param inTexPxlFmt The pixel format of the OpenGL texture.  Not as important to get right, used primarily for creating the texture.
\param inTexPxlType The pixel type of the OpenGL texture.  Not as important to get right, used primarily for creating the texture.
\param inTexSize The Size of the OpenGL texture, in pixels.
\param inTexFlipped Whether or not the image in the OpenGL texture is flipped.
\param inImgRectInTex The region of the texture (bottom-left origin, in pixels) that contains the image.
\param inReleaseCallbackContext An arbitrary pointer stored (weakly) with the GLBuffer- this pointer is passed to the release callback.  If you want to store a pointer from another SDK, this is the appropriate place to do so.
\param inReleaseCallback A callback function or lambda that will be executed when the GLBuffer is deallocated.  If the GLBuffer needs to release any other resources when it's freed, this is the appropriate place to do so.
\param inPoolRef The pool that the GLBuffer should be created with.  When the GLBuffer is freed, its underlying GL resources will be returned to this pool (where they will be either freed or recycled).
*/
VVGL_EXPORT GLBufferRef CreateFromExistingGLTexture(const int32_t & inTexName, const GLBuffer::Target & inTexTarget, const GLBuffer::InternalFormat & inTexIntFmt, const GLBuffer::PixelFormat & inTexPxlFmt, const GLBuffer::PixelType & inTexPxlType, const Size & inTexSize, const bool & inTexFlipped, const Rect & inImgRectInTex, const void * inReleaseCallbackContext, const GLBuffer::BackingReleaseCallback & inReleaseCallback, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());

#if !defined(VVGL_SDK_IOS)
/*!
\ingroup VVGL_BUFFERCREATE
\brief Creates and returns an OpenGL texture that has an internal BGRA format (BGRA is optimized on most platforms) and is 8 bits per component.
\param size The size of the buffer to create (in pixels).
\param createInCurrentContext If true, the GL resource will be created in the current context (assumes that a GL context is active in the current thread).  If false, the GL resource will be created by the GL context owned by the buffer pool.
\param inPoolRef The pool that the GLBuffer should be created with.  When the GLBuffer is freed, its underlying GL resources will be returned to this pool (where they will be either freed or recycled).
*/
VVGL_EXPORT GLBufferRef CreateBGRATex(const Size & size, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
/*!
\ingroup VVGL_BUFFERCREATE
\brief Creates and returns an OpenGL texture that has an internal BGRA format and is 32 bits per component.
\param size The size of the buffer to create (in pixels).
\param createInCurrentContext If true, the GL resource will be created in the current context (assumes that a GL context is active in the current thread).  If false, the GL resource will be created by the GL context owned by the buffer pool.
\param inPoolRef The pool that the GLBuffer should be created with.  When the GLBuffer is freed, its underlying GL resources will be returned to this pool (where they will be either freed or recycled).
*/
VVGL_EXPORT GLBufferRef CreateBGRAFloatTex(const Size & size, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
#if !(VVGL_SDK_RPI)
/*!
\ingroup VVGL_BUFFERCREATE
\brief Creates and returns an OpenGL texture that has an internal BGRA format, is 32 bits per component, and has a CPU backing (the CPU backing is highly accelerated and is basically DMA with VRAM as a result of various extensions and optimizations)
\param size The size of the buffer to create (in pixels).
\param createInCurrentContext If true, the GL resource will be created in the current context (assumes that a GL context is active in the current thread).  If false, the GL resource will be created by the GL context owned by the buffer pool.
\param inPoolRef The pool that the GLBuffer should be created with.  When the GLBuffer is freed, its underlying GL resources will be returned to this pool (where they will be either freed or recycled).
*/
VVGL_EXPORT GLBufferRef CreateBGRAFloatCPUBackedTex(const Size & size, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
#endif	//	!VVGL_SDK_RPI
#endif	//	!VVGL_SDK_IOS


/*!
\ingroup VVGL_BUFFERCREATE
\brief Creates and returns an OpenGL texture from the image at the passed path.
\param inPath The path of the image file to create a GLBuffer from.
\param createInCurrentContext If true, the GL resource will be created in the current context (assumes that a GL context is active in the current thread).  If false, the GL resource will be created by the GL context owned by the buffer pool.
\param inPoolRef The pool that the GLBuffer should be created with.  When the GLBuffer is freed, its underlying GL resources will be returned to this pool (where they will be either freed or recycled).
*/
VVGL_EXPORT GLBufferRef CreateTexFromImage(const string & inPath, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
/*!
\ingroup VVGL_BUFFERCREATE
\brief Creates and returns an OpenGL cube texture from the images at the passed paths.
\param inPaths A vector of paths to image files, each of which is one face of the cube.
\param createInCurrentContext If true, the GL resource will be created in the current context (assumes that a GL context is active in the current thread).  If false, the GL resource will be created by the GL context owned by the buffer pool.
\param inPoolRef The pool that the GLBuffer should be created with.  When the GLBuffer is freed, its underlying GL resources will be returned to this pool (where they will be either freed or recycled).
*/
VVGL_EXPORT GLBufferRef CreateCubeTexFromImagePaths(const vector<string> & inPaths, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());


//	Qt-only func
#if defined(VVGL_SDK_QT)
/*!
\ingroup VVGL_BUFFERCREATE
\brief Creates and returns an OpenGL buffer from the passed QImage.
\param inImg The QImage that will be uploaded to the GL texture.  The QImage will not be retained by the GLBuffer.
\param createInCurrentContext If true, the GL resource will be created in the current context (assumes that a GL context is active in the current thread).  If false, the GL resource will be created by the GL context owned by the buffer pool.
\param inPoolRef The pool that the GLBuffer should be created with.  When the GLBuffer is freed, its underlying GL resources will be returned to this pool (where they will be either freed or recycled).
*/
VVGL_EXPORT GLBufferRef CreateBufferForQImage(QImage * inImg, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
#endif


//	mac-only funcs
#if defined(VVGL_SDK_MAC)
//	'inContextRef' must not be NULL- this function will have not work if it is null!
/*!
\ingroup VVGL_BUFFERCREATE
\brief Some GLBuffers are GL textures with a CPU backing- this function pushes the contents of the CPU backing to the GL texture.
\param inBufferRef The GLBuffer whose contents will be uploaded.  This has to be a GLBuffer with a CPU backing or the function won't do anything!
\param inContextRef This is the context that will be used to upload the image data from RAM to VRAM.
*/
void PushTexRangeBufferRAMtoVRAM(const GLBufferRef & inBufferRef, const GLContextRef & inContextRef);
/*!
\ingroup VVGL_BUFFERCREATE
\brief Creates an 8 bit per channel RGBA OpenGL texture backed by an IOSurface that also gets created- the contents of this texture can be shared with other processes by sharing the IOSurface.
\param inSize The size of the buffer to create (in pixels).
\param createInCurrentContext If true, the GL resource will be created in the current context (assumes that a GL context is active in the current thread).  If false, the GL resource will be created by the GL context owned by the buffer pool.
\param inPoolRef The pool that the GLBuffer should be created with.  When the GLBuffer is freed, its underlying GL resources will be returned to this pool (where they will be either freed or recycled).
*/
GLBufferRef CreateRGBATexIOSurface(const Size & inSize, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
/*!
\ingroup VVGL_BUFFERCREATE
\brief Creates a 32 bit per channel RGBA OpenGL texture backed by an IOSurface that also gets created- the contents of this texture can be shared with other processes by sharing the IOSurface.
\param inSize The size of the buffer to create (in pixels).
\param createInCurrentContext If true, the GL resource will be created in the current context (assumes that a GL context is active in the current thread).  If false, the GL resource will be created by the GL context owned by the buffer pool.
\param inPoolRef The pool that the GLBuffer should be created with.  When the GLBuffer is freed, its underlying GL resources will be returned to this pool (where they will be either freed or recycled).
*/
GLBufferRef CreateRGBAFloatTexIOSurface(const Size & inSize, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
/*!
\ingroup VVGL_BUFFERCREATE
\brief Creates and returns a GLBuffer representing a texture that is backed by an IOSurface that was created by another process and passed to this process using the passed IOSurfaceID.
\param inID An IOSurfaceID (basically an integer) that was passed to the process.
\param createInCurrentContext If true, the GL resource will be created in the current context (assumes that a GL context is active in the current thread).  If false, the GL resource will be created by the GL context owned by the buffer pool.
\param inPoolRef The pool that the GLBuffer should be created with.  When the GLBuffer is freed, its underlying GL resources will be returned to this pool (where they will be either freed or recycled).
*/
GLBufferRef CreateRGBATexFromIOSurfaceID(const IOSurfaceID & inID, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
/*!
\ingroup VVGL_BUFFERCREATE
\brief Creates and returns an OpenGL texture that is populated with the contents of the passed CVPixelBufferRef.
\param inCVPB The CVPixelBufferRef whose contents will be uploaded to the texture.
\param inTexRange Whether or not the GL texture should be created using the Apple OpenGL texture range (DMA VRAM).
\param inIOSurface Whether or not the GL texture that is created should have an IOSurface backing (IOSurface backed textures are slower but can be shared between processes).
\param createInCurrentContext If true, the GL resource will be created in the current context (assumes that a GL context is active in the current thread).  If false, the GL resource will be created by the GL context owned by the buffer pool.
\param inPoolRef The pool that the GLBuffer should be created with.  When the GLBuffer is freed, its underlying GL resources will be returned to this pool (where they will be either freed or recycled).
*/
GLBufferRef CreateBufferForCVPixelBuffer(CVPixelBufferRef & inCVPB, const bool & inTexRange, const bool & inIOSurface, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
/*!
\ingroup VVGL_BUFFERCREATE
\brief Creates and returns an OpenGL texture that is populated with the contents of the passed CMSampleBufferRef (uses CreateBufferForCVPixelBuffer()).
\param n The CMSampleBufferRef whose contents will be uploaded to the texture. This must have a CVPixelBufferRef or the function won't work!
\param createInCurrentContext If true, the GL resource will be created in the current context (assumes that a GL context is active in the current thread).  If false, the GL resource will be created by the GL context owned by the buffer pool.
\param inPoolRef The pool that the GLBuffer should be created with.  When the GLBuffer is freed, its underlying GL resources will be returned to this pool (where they will be either freed or recycled).
*/
GLBufferRef CreateTexRangeFromCMSampleBuffer(CMSampleBufferRef & n, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
/*!
\ingroup VVGL_BUFFERCREATE
\brief Creates and returns an OpenGL RECT-target texture.
\param size The size of the buffer to create (in pixels).
\param createInCurrentContext If true, the GL resource will be created in the current context (assumes that a GL context is active in the current thread).  If false, the GL resource will be created by the GL context owned by the buffer pool.
\param inPoolRef The pool that the GLBuffer should be created with.  When the GLBuffer is freed, its underlying GL resources will be returned to this pool (where they will be either freed or recycled).
*/
GLBufferRef CreateRGBARectTex(const Size & size, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
#endif


//	mac & ios funcs
#if defined(VVGL_SDK_MAC) || defined(VVGL_SDK_IOS)
/*!
\ingroup VVGL_BUFFERCREATE
\brief Creates and returns an OpenGL texture populated with the contents of the passed CGImageRef.
\param n The CGImageRef whose contents will be uploaded to the texture.
\param createInCurrentContext If true, the GL resource will be created in the current context (assumes that a GL context is active in the current thread).  If false, the GL resource will be created by the GL context owned by the buffer pool.
\param inPoolRef The pool that the GLBuffer should be created with.  When the GLBuffer is freed, its underlying GL resources will be returned to this pool (where they will be either freed or recycled).
*/
GLBufferRef CreateTexFromCGImageRef(const CGImageRef & n, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
/*!
\ingroup VVGL_BUFFERCREATE
\brief Creates and returns a cube-type OpenGL texture populated with the passed vector of CGImageRefs
\param inImgs A vector of CGImageRefs, each of which will be used to populate one of the faces of the cube texture.
\param createInCurrentContext If true, the GL resource will be created in the current context (assumes that a GL context is active in the current thread).  If false, the GL resource will be created by the GL context owned by the buffer pool.
\param inPoolRef The pool that the GLBuffer should be created with.  When the GLBuffer is freed, its underlying GL resources will be returned to this pool (where they will be either freed or recycled).
*/
GLBufferRef CreateCubeTexFromImages(const vector<CGImageRef> & inImgs, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
/*!
\ingroup VVGL_BUFFERCREATE
\brief Unpremultiplies the contents of the bitmap in the passed CGContextRef.
\param ctx The contents of this CGContextRef will be unpremultiplied.
*/
void CGBitmapContextUnpremultiply(CGContextRef ctx);
#endif

#if defined(VVGL_SDK_MAC)
/*!
\ingroup VVGL_BUFFERCREATE
\brief Creates and returns a GLBuffer that represents the texture in the passed CVOpenGLTextureRef.  VVGL won't actually create any GL resources here- it's basically just using GLBuffer as a wrapper around the GL resource owned by the CVOpenGLTextureRef.
\param inTexRef The CVOpenGLTextureRef that will be used to populate the GLBuffer.  The GLBuffer's member variables are populated using the properties from the texture ref- no GL resources are created or destroyed.
\param createInCurrentContext If true, the GL resource will be created in the current context (assumes that a GL context is active in the current thread).  If false, the GL resource will be created by the GL context owned by the buffer pool.
\param inPoolRef The pool that the GLBuffer should be created with.  When the GLBuffer is freed, its underlying GL resources will be returned to this pool (where they will be either freed or recycled).
*/
GLBufferRef CreateBufferForCVGLTex(CVOpenGLTextureRef & inTexRef, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
#elif defined(VVGL_SDK_IOS)
/*!
\ingroup VVGL_BUFFERCREATE
\brief Creates and returns a GLBuffer that represents the texture in the passed CVOpenGLTextureRef.  VVGL won't actually create any GL resources here- it's basically just using GLBuffer as a wrapper around the GL resource owned by the CVOpenGLTextureRef.
\param inTexRef The CVOpenGLTextureRef that will be used to populate the GLBuffer.  The GLBuffer's member variables are populated using the properties from the texture ref- no GL resources are created or destroyed.
\param createInCurrentContext If true, the GL resource will be created in the current context (assumes that a GL context is active in the current thread).  If false, the GL resource will be created by the GL context owned by the buffer pool.
\param inPoolRef The pool that the GLBuffer should be created with.  When the GLBuffer is freed, its underlying GL resources will be returned to this pool (where they will be either freed or recycled).
*/
GLBufferRef CreateBufferForCVGLTex(CVOpenGLESTextureRef & inTexRef, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
#endif


}


#endif /* VVGL_GLBufferPool_hpp */
