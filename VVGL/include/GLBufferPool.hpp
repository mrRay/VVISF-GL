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




//! Buffer pools create and manage GL resources which, on deletion, are either destroyed or returned to the pool for re-use.
/*!
\ingroup VVGL_BASIC

Notes on use:
- GLBufferPool creates GLBuffer/GLBufferRef instances.  You need a GLBufferPool to create a GLBuffer.  If you're using VVGL you're almost certainly going to want to create a buffer pool.
- For convenience, VVGL defines a singleton global buffer pool that you should create while your app is launching and setting up its OpenGL environment using CreateGlobalBufferPool().  The pool can be retrieved later using GetGlobalBufferPool().  It is STRONGLY RECOMMENDED that you create this global buffer pool- if you don't then you'll need to explicitly pass the pool you want to use to the various buffer creation functions.
- GLBufferPool has a member function that creates buffers- but you probably shouldn't use it directly.  Instead, you should use one of the buffer creation functions in (\ref VVGL_BUFFERCREATE).
- Don't forget to call the housekeeping() member function on your buffer pools periodically!
- GLBufferPool requires a GL context on creation- the pool maintains a strong ref to this context, and will use it to create/destroy GL resources unless instructed otherwise via the "createInCurrentContext" variable in the various buffer creation functions.
*/
class VVGL_EXPORT GLBufferPool	{
	
	//	vars
	protected:
		bool				deleted = false;
		mutex				freeBuffersLock;
		vector<GLBufferRef>		freeBuffers;
		
		recursive_mutex		contextLock;
		GLContextRef		context = nullptr;	//	this is the context that the buffer pool will use to create/destroy GL resources
		
		Timestamper			timestamper;
		
#if defined(VVGL_SDK_MAC) || defined(VVGL_SDK_IOS)
		CGColorSpaceRef		colorSpace = nullptr;
#endif
		
	//	methods
	public:
		//GLBufferPool(const GLContext * inShareCtx=nullptr);
		GLBufferPool(const GLContextRef & inCtx=nullptr);
		virtual ~GLBufferPool();
		//	call this method to prep a pool for deletion- this will cause it to purge its contents, refuse to dispense more buffers, and automatically release any buffers that are returned to it
		inline void prepareToBeDeleted() { deleted=true; purge(); }
		
		//!	This member function is how the pool creates buffers.  You probably shouldn't call this function directly- instead use one of the functions in (\ref VVGL_BUFFERCREATE) directly or as a prototype.  They're in %GLBufferPool.hpp/GLBufferPool.cpp.
		GLBufferRef createBufferRef(const GLBuffer::Descriptor & desc, const Size & size={640,480}, const void * backingPtr=nullptr, const Size & backingSize={640,480}, const bool & createInCurrentContext=false);
		GLBufferRef fetchMatchingFreeBuffer(const GLBuffer::Descriptor & desc, const Size & size);
		
		//!	You must call this periodically (once per render loop after you finish drawing is usually a good time to call this).  This function frees any buffers that have been sitting unused in the pool for "too long".
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
\relatesalso GLBufferPool
\brief This is how you create the global buffer pool.
\param inPoolCtx The buffer pool that is created will share this context (textures and buffers created by the buffer pool will be compatible with all other GL contexts in the same sharegroup).
*/
VVGL_EXPORT GLBufferPoolRef CreateGlobalBufferPool(const GLContextRef & inPoolCtx=nullptr);
/*!
\relatesalso GLBufferPool
\brief This is how you retrieve the global buffer pool.
*/
VVGL_EXPORT const GLBufferPoolRef & GetGlobalBufferPool();




/*!
\name Non-Image buffer creation functions
\brief These functions create GLBuffers that aren't actually images at all.
*/
///@{

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

///@}




/*!
\name Renderbuffer creation functions
\brief These functions create GLBuffers that are renderbuffers (not textures).
*/
///@{

/*!
\ingroup VVGL_BUFFERCREATE
\brief Creates and returns an OpenGL renderbuffer.
\param size The size of the buffer to create (in pixels).
\param createInCurrentContext If true, the GL resource will be created in the current context (assumes that a GL context is active in the current thread).  If false, the GL resource will be created by the GL context owned by the buffer pool.
\param inPoolRef The pool that the GLBuffer should be created with.  When the GLBuffer is freed, its underlying GL resources will be returned to this pool (where they will be either freed or recycled).
*/
VVGL_EXPORT GLBufferRef CreateRB(const Size & size, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());

///@}




/*!
\name CPU-based buffer creation functions
\brief These functions create GLBuffers that are entirely CPU-based- no GL resources.  Some of these functions allocate memory- others just create a GLBuffer representation of an existing block of memory that was created by another API.
*/
///@{

/*!
\ingroup VVGL_BUFFERCREATE
\brief Creates and returns a GLBuffer that uses a 32 bit per pixel non-planar RGBA CPU buffer as its backing.  This GLBuffer doesn't have any GL resources.  The CPU buffer is allocated by this function.
\param size The size of the buffer to create (in pixels).
\param inPoolRef The pool that the GLBuffer should be created with.  This is a CPU-based buffer, so there aren't any GL resources that need to be freed by a GL context, but it has a pool regardless.
*/
VVGL_EXPORT GLBufferRef CreateRGBACPUBuffer(const Size & size, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
/*!
\ingroup VVGL_BUFFERCREATE
\brief Creates and returns a GLBuffer that uses a 128 bit per pixel non-planar RGBA CPU buffer as its backing.  This GLBuffer doesn't have any GL resources.  The CPU buffer is allocated by this function.
\param size The size of the buffer to create (in pixels).
\param inPoolRef The pool that the GLBuffer should be created with.  This is a CPU-based buffer, so there aren't any GL resources that need to be freed by a GL context, but it has a pool regardless.
*/
VVGL_EXPORT GLBufferRef CreateRGBAFloatCPUBuffer(const Size & size, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
/*!
\ingroup VVGL_BUFFERCREATE
\brief Creates and returns a GLBuffer that uses a 32 bit per pixel non-planar BGRA CPU buffer as its backing.  This GLBuffer doesn't have any GL resources.  The CPU buffer is allocated by this function.
\param size The size of the buffer to create (in pixels).
\param inPoolRef The pool that the GLBuffer should be created with.  This is a CPU-based buffer, so there aren't any GL resources that need to be freed by a GL context, but it has a pool regardless.
*/
VVGL_EXPORT GLBufferRef CreateBGRACPUBuffer(const Size & size, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
/*!
\ingroup VVGL_BUFFERCREATE
\brief Creates and returns a GLBuffer that uses a 128 bit per pixel non-planar BGRA CPU buffer as its backing.  This GLBuffer doesn't have any GL resources.  The CPU buffer is allocated by this function.
\param size The size of the buffer to create (in pixels).
\param inPoolRef The pool that the GLBuffer should be created with.  This is a CPU-based buffer, so there aren't any GL resources that need to be freed by a GL context, but it has a pool regardless.
*/
VVGL_EXPORT GLBufferRef CreateBGRAFloatCPUBuffer(const Size & size, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());

/*!
\ingroup VVGL_BUFFERCREATE
\brief Creates and returns a GLBuffer that describes a non-planar RGBA image (32 bits per pixel) in CPU memory- doesn't create any GL resources, and also doesn't allocate the CPU memory required for the image.  This function is a good way to create a GLBuffer using a block of memory from another API- the release callback and release callback context can be used to ensure that the GLBuffer has a strong reference to any memory structures that need to exist for the duration of the GLBuffer's lifetime.  This function does not copy any memory.
\param inCPUBufferSizeInPixels The size of the buffer at inCPUBackingPtr, expressed in pixels.  If the image is padded, this size needs to include the padding.  If the padding is a non-integral value, you can't use this function and will instead have to create a CPU buffer (using CreateRGBACPUBuffer() or another function that allocates memory) and copy the image data to it.
\param inCPUBackingPtr A pointer to the memory containing the image data- this is the data that will eventually get uploaded to OpenGL (the bitmap data).  Must not be null.
\param inImageSizeInPixels The size of the image at inCPUBackingPtr, expressed in pixels.  This is the size of the image- not the size of the buffer- and as such, it should not include any padding.
\param inReleaseCallbackContext An arbitrary pointer you supply.  A weak ref to this pointer is maintained by the GLBuffer, and it is passed to the buffer's release callback (which is executed when the buffer is freed).
\param inReleaseCallback This function or block will be executed when the GLBuffer is destroyed- the release callback context from inReleaseCallbackContext is passed to this function.
\param inPoolRef The pool that the GLBuffer should be created with.  This is a CPU-based buffer, so there aren't any GL resources that need to be freed by a GL context, but it has a pool regardless.
*/
VVGL_EXPORT GLBufferRef CreateRGBACPUBufferUsing(const Size & inCPUBufferSizeInPixels, const void * inCPUBackingPtr, const Size & inImageSizeInPixels, const void * inReleaseCallbackContext, const GLBuffer::BackingReleaseCallback & inReleaseCallback, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
/*!
\ingroup VVGL_BUFFERCREATE
\brief Creates and returns a GLBuffer that describes a non-planar RGBA image (128 bits per pixel, or 32 bits per channel) in CPU memory- doesn't create any GL resources, and also doesn't allocate the CPU memory required for the image.  This function is a good way to create a GLBuffer using a block of memory from another API- the release callback and release callback context can be used to ensure that the GLBuffer has a strong reference to any memory structures that need to exist for the duration of the GLBuffer's lifetime.  This function does not copy any memory.
\param inCPUBufferSizeInPixels The size of the buffer at inCPUBackingPtr, expressed in pixels.  If the image is padded, this size needs to include the padding.  If the padding is a non-integral value, you can't use this function and will instead have to create a CPU buffer (using CreateRGBACPUBuffer() or another function that allocates memory) and copy the image data to it.
\param inCPUBackingPtr A pointer to the memory containing the image data- this is the data that will eventually get uploaded to OpenGL (the bitmap data).  Must not be null.
\param inImageSizeInPixels The size of the image at inCPUBackingPtr, expressed in pixels.  This is the size of the image- not the size of the buffer- and as such, it should not include any padding.
\param inReleaseCallbackContext An arbitrary pointer you supply.  A weak ref to this pointer is maintained by the GLBuffer, and it is passed to the buffer's release callback (which is executed when the buffer is freed).
\param inReleaseCallback This function or block will be executed when the GLBuffer is destroyed- the release callback context from inReleaseCallbackContext is passed to this function.
\param inPoolRef The pool that the GLBuffer should be created with.  This is a CPU-based buffer, so there aren't any GL resources that need to be freed by a GL context, but it has a pool regardless.
*/
VVGL_EXPORT GLBufferRef CreateRGBAFloatCPUBufferUsing(const Size & inCPUBufferSizeInPixels, const void * inCPUBackingPtr, const Size & inImageSizeInPixels, const void * inReleaseCallbackContext, const GLBuffer::BackingReleaseCallback & inReleaseCallback, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
/*!
\ingroup VVGL_BUFFERCREATE
\brief Creates and returns a GLBuffer that describes a non-planar BGRA image (32 bits per pixel) in CPU memory- doesn't create any GL resources, and also doesn't allocate the CPU memory required for the image.  This function is a good way to create a GLBuffer using a block of memory from another API- the release callback and release callback context can be used to ensure that the GLBuffer has a strong reference to any memory structures that need to exist for the duration of the GLBuffer's lifetime.  This function does not copy any memory.
\param inCPUBufferSizeInPixels The size of the buffer at inCPUBackingPtr, expressed in pixels.  If the image is padded, this size needs to include the padding.  If the padding is a non-integral value, you can't use this function and will instead have to create a CPU buffer (using CreateRGBACPUBuffer() or another function that allocates memory) and copy the image data to it.
\param inCPUBackingPtr A pointer to the memory containing the image data- this is the data that will eventually get uploaded to OpenGL (the bitmap data).  Must not be null.
\param inImageSizeInPixels The size of the image at inCPUBackingPtr, expressed in pixels.  This is the size of the image- not the size of the buffer- and as such, it should not include any padding.
\param inReleaseCallbackContext An arbitrary pointer you supply.  A weak ref to this pointer is maintained by the GLBuffer, and it is passed to the buffer's release callback (which is executed when the buffer is freed).
\param inReleaseCallback This function or block will be executed when the GLBuffer is destroyed- the release callback context from inReleaseCallbackContext is passed to this function.
\param inPoolRef The pool that the GLBuffer should be created with.  This is a CPU-based buffer, so there aren't any GL resources that need to be freed by a GL context, but it has a pool regardless.
*/
VVGL_EXPORT GLBufferRef CreateBGRACPUBufferUsing(const Size & inCPUBufferSizeInPixels, const void * inCPUBackingPtr, const Size & inImageSizeInPixels, const void * inReleaseCallbackContext, const GLBuffer::BackingReleaseCallback & inReleaseCallback, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
/*!
\ingroup VVGL_BUFFERCREATE
\brief Creates and returns a GLBuffer that describes a non-planar BGRA image (128 bits per pixel, or 32 bits per channel) in CPU memory- doesn't create any GL resources, and also doesn't allocate the CPU memory required for the image.  This function is a good way to create a GLBuffer using a block of memory from another API- the release callback and release callback context can be used to ensure that the GLBuffer has a strong reference to any memory structures that need to exist for the duration of the GLBuffer's lifetime.  This function does not copy any memory.
\param inCPUBufferSizeInPixels The size of the buffer at inCPUBackingPtr, expressed in pixels.  If the image is padded, this size needs to include the padding.  If the padding is a non-integral value, you can't use this function and will instead have to create a CPU buffer (using CreateRGBACPUBuffer() or another function that allocates memory) and copy the image data to it.
\param inCPUBackingPtr A pointer to the memory containing the image data- this is the data that will eventually get uploaded to OpenGL (the bitmap data).  Must not be null.
\param inImageSizeInPixels The size of the image at inCPUBackingPtr, expressed in pixels.  This is the size of the image- not the size of the buffer- and as such, it should not include any padding.
\param inReleaseCallbackContext An arbitrary pointer you supply.  A weak ref to this pointer is maintained by the GLBuffer, and it is passed to the buffer's release callback (which is executed when the buffer is freed).
\param inReleaseCallback This function or block will be executed when the GLBuffer is destroyed- the release callback context from inReleaseCallbackContext is passed to this function.
\param inPoolRef The pool that the GLBuffer should be created with.  This is a CPU-based buffer, so there aren't any GL resources that need to be freed by a GL context, but it has a pool regardless.
*/
VVGL_EXPORT GLBufferRef CreateBGRAFloatCPUBufferUsing(const Size & inCPUBufferSizeInPixels, const void * inCPUBackingPtr, const Size & inImageSizeInPixels, const void * inReleaseCallbackContext, const GLBuffer::BackingReleaseCallback & inReleaseCallback, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());

#if !defined(VVGL_TARGETENV_GLES) && !defined(VVGL_TARGETENV_GLES3)
/*!
\ingroup VVGL_BUFFERCREATE
\brief Creates and returns a GLBuffer that describes a non-planar YCbCr image (16 bits per pixel) in CPU memory- doesn't create any GL resources, and also doesn't allocate the CPU memory required for the image.  This function is a good way to create a GLBuffer using a block of memory from another API- the release callback and release callback context can be used to ensure that the GLBuffer has a strong reference to any memory structures that need to exist for the duration of the GLBuffer's lifetime.  This function does not copy any memory.
\param inCPUBufferSizeInPixels The size of the buffer at inCPUBackingPtr, expressed in pixels.  If the image is padded, this size needs to include the padding.  If the padding is a non-integral value, you can't use this function and will instead have to create a CPU buffer (using CreateRGBACPUBuffer() or another function that allocates memory) and copy the image data to it.
\param inCPUBackingPtr A pointer to the memory containing the image data- this is the data that will eventually get uploaded to OpenGL (the bitmap data).  Must not be null.
\param inImageSizeInPixels The size of the image at inCPUBackingPtr, expressed in pixels.  This is the size of the image- not the size of the buffer- and as such, it should not include any padding.
\param inReleaseCallbackContext An arbitrary pointer you supply.  A weak ref to this pointer is maintained by the GLBuffer, and it is passed to the buffer's release callback (which is executed when the buffer is freed).
\param inReleaseCallback This function or block will be executed when the GLBuffer is destroyed- the release callback context from inReleaseCallbackContext is passed to this function.
\param inPoolRef The pool that the GLBuffer should be created with.  This is a CPU-based buffer, so there aren't any GL resources that need to be freed by a GL context, but it has a pool regardless.
*/
VVGL_EXPORT GLBufferRef CreateYCbCrCPUBufferUsing(const Size & inCPUBufferSizeInPixels, const void * inCPUBackingPtr, const Size & inImageSizeInPixels, const void * inReleaseCallbackContext, const GLBuffer::BackingReleaseCallback & inReleaseCallback, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
#endif	//	!defined(VVGL_TARGETENV_GLES) && !defined(VVGL_TARGETENV_GLES3)

///@}




/*!
\name Texture Buffers
\brief These functions create GLBuffers that are OpenGL textures
*/
///@{

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

#if !defined(VVGL_TARGETENV_GLES) && !defined(VVGL_TARGETENV_GLES3)
/*!
\ingroup VVGL_BUFFERCREATE
\brief Creates and returns an OpenGL texture that has an internal RGB (no alhpa!) format and is 32 bits per component (128 bit color).  Internally, the data store for this texture is expected to be packed (non-planar) YCbCr data (uyvy or 2vuy).
\param size The size of the buffer to create (in pixels).
\param createInCurrentContext If true, the GL resource will be created in the current context (assumes that a GL context is active in the current thread).  If false, the GL resource will be created by the GL context owned by the buffer pool.
\param inPoolRef The pool that the GLBuffer should be created with.  When the GLBuffer is freed, its underlying GL resources will be returned to this pool (where they will be either freed or recycled).
*/
VVGL_EXPORT GLBufferRef CreateYCbCrTex(const Size & size, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
#endif	//	!defined(VVGL_TARGETENV_GLES) && !defined(VVGL_TARGETENV_GLES3)

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

#if defined(VVGL_SDK_MAC)
/*!
\name CPU-backed Texture Image Buffers
\brief These functions create GLBuffers that are OpenGL textures backed by a CPU buffer.  Changes made to the CPU buffer can be quickly pushed to the GL texture.
*/
///@{

/*!
\ingroup VVGL_BUFFERCREATE
\brief Creates and returns an OpenGL texture that has an internal RGBA format, is 8 bits per component, and has a CPU backing (the CPU backing is highly accelerated and is basically DMA with VRAM as a result of various extensions and optimizations)
\param size The size of the buffer to create (in pixels).
\param createInCurrentContext If true, the GL resource will be created in the current context (assumes that a GL context is active in the current thread).  If false, the GL resource will be created by the GL context owned by the buffer pool.
\param inPoolRef The pool that the GLBuffer should be created with.  When the GLBuffer is freed, its underlying GL resources will be returned to this pool (where they will be either freed or recycled).
*/
VVGL_EXPORT GLBufferRef CreateRGBACPUBackedTex(const Size & size, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
/*!
\ingroup VVGL_BUFFERCREATE
\brief Creates and returns an OpenGL texture that has an internal RGBA format, is 32 bits per component, and has a CPU backing (the CPU backing is highly accelerated and is basically DMA with VRAM as a result of various extensions and optimizations)
\param size The size of the buffer to create (in pixels).
\param createInCurrentContext If true, the GL resource will be created in the current context (assumes that a GL context is active in the current thread).  If false, the GL resource will be created by the GL context owned by the buffer pool.
\param inPoolRef The pool that the GLBuffer should be created with.  When the GLBuffer is freed, its underlying GL resources will be returned to this pool (where they will be either freed or recycled).
*/
VVGL_EXPORT GLBufferRef CreateRGBAFloatCPUBackedTex(const Size & size, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
/*!
\ingroup VVGL_BUFFERCREATE
\brief Creates and returns an OpenGL texture that has an internal BGRA format, is 8 bits per component, and has a CPU backing (the CPU backing is highly accelerated and is basically DMA with VRAM as a result of various extensions and optimizations)
\param size The size of the buffer to create (in pixels).
\param createInCurrentContext If true, the GL resource will be created in the current context (assumes that a GL context is active in the current thread).  If false, the GL resource will be created by the GL context owned by the buffer pool.
\param inPoolRef The pool that the GLBuffer should be created with.  When the GLBuffer is freed, its underlying GL resources will be returned to this pool (where they will be either freed or recycled).
*/
VVGL_EXPORT GLBufferRef CreateBGRACPUBackedTex(const Size & size, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
/*!
\ingroup VVGL_BUFFERCREATE
\brief Creates and returns an OpenGL texture that has an internal BGRA format, is 32 bits per component, and has a CPU backing (the CPU backing is highly accelerated and is basically DMA with VRAM as a result of various extensions and optimizations)
\param size The size of the buffer to create (in pixels).
\param createInCurrentContext If true, the GL resource will be created in the current context (assumes that a GL context is active in the current thread).  If false, the GL resource will be created by the GL context owned by the buffer pool.
\param inPoolRef The pool that the GLBuffer should be created with.  When the GLBuffer is freed, its underlying GL resources will be returned to this pool (where they will be either freed or recycled).
*/
VVGL_EXPORT GLBufferRef CreateBGRAFloatCPUBackedTex(const Size & size, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());

///@}

#endif	//	VVGL_SDK_MAC
#endif	//	!VVGL_SDK_IOS

///@}




/*!
\name Image file to buffer functions
\brief These functions create GLBuffers from image files on disk
*/
///@{

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

///@}




//	none of this stuff should be available if we're running ES
#if !defined(VVGL_TARGETENV_GLES) && !defined(VVGL_TARGETENV_GLES3)

/*!
\name PBO creation functions
\brief These functions create GLBuffers that represent PBOs, which are used primarily to stream image data to/from GPUs.
*/
///@{

/*!
\ingroup VVGL_BUFFERCREATE
\brief Creates a GLBufferRef that represents a PBO (pixel buffer object).  These functions are used by GLTexToCPUCopier and GLBufferTexUploader to move memory between RAM and VRAM.
\param inTarget GL_PIXEL_UNPACK_BUFFER or GL_PIXEL_PACK_BUFFER.  "pack" means this PBO will be used to transfer pixel data TO a PBO (glReadPixels(), glGetTexImage()).  "unpack" means this PBO will be used to transfer pixel data FROM a PBO (glDrawPixels(), glTexImage2D(), glTexSubImage2D()).
\param inUsage GL_STREAM_DRAW, GL_STREAM_READ, etc.
\param inSize The dimensions of the image that this PBO will need to contain.
\param inData A pointer to the data in CPU memory, or null.  If this data is padded, 'inSize' needs to describe the size of the image including the padding.  Must be at least as large as the value returned by backingLengthForSize() for the appropriate GLBuffer::Descriptor.
\param inCreateInCurrentContext If true, the GL resource will be created in the current context (assumes that a GL context is active in the current thread).  If false, the GL resource will be created by the GL context owned by the buffer pool.
\param inPoolRef The pool that the GLBuffer should be created with.  When the GLBuffer is freed, its underlying GL resources will be returned to this pool (where they will be either freed or recycled).

- "pack" means this PBO will be used to transfer pixel data TO a PBO (glReadPixels(), glGetTexImage())
- "unpack" means this PBO will be used to transfer pixel data FROM a PBO (glDrawPixels(), glTexImage2D(), glTexSubImage2D())
<BR>

decoding "GL_STREAM_DRAW, GL_STREAM_READ, etc:
- STREAM		write once, read at most a few times
- STATIC		write once, read many times
- DYNAMIC		write many times, read many times
<BR><BR>
- DRAW		CPU -> GL
- READ		GL -> CPU
- COPY		GL -> GL
*/
VVGL_EXPORT GLBufferRef CreateRGBAPBO(const GLBuffer::Target & inTarget, const int32_t & inUsage, const Size & inSize, const void * inData=nullptr, const bool & inCreateInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
/*!
\ingroup VVGL_BUFFERCREATE
\brief Creates a GLBufferRef that represents a PBO.  See the documentation of CreateRGBAPBO() for more information- PBOs do not contain inherently typed data, these functions only differ in the contents of the GLBuffer::Descriptor they populate.
*/
VVGL_EXPORT GLBufferRef CreateBGRAPBO(const int32_t & inTarget, const int32_t & inUsage, const Size & inSize, const void * inData=nullptr, const bool & inCreateInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
/*!
\ingroup VVGL_BUFFERCREATE
\brief Creates a GLBufferRef that represents a PBO.  See the documentation of CreateRGBAPBO() for more information- PBOs do not contain inherently typed data, these functions only differ in the contents of the GLBuffer::Descriptor they populate.
*/
VVGL_EXPORT GLBufferRef CreateRGBAFloatPBO(const int32_t & inTarget, const int32_t & inUsage, const Size & inSize, const void * inData=nullptr, const bool & inCreateInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
/*!
\ingroup VVGL_BUFFERCREATE
\brief Creates a GLBufferRef that represents a PBO.  See the documentation of CreateRGBAPBO() for more information- PBOs do not contain inherently typed data, these functions only differ in the contents of the GLBuffer::Descriptor they populate.
*/
VVGL_EXPORT GLBufferRef CreateBGRAFloatPBO(const int32_t & inTarget, const int32_t & inUsage, const Size & inSize, const void * inData=nullptr, const bool & inCreateInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
/*!
\ingroup VVGL_BUFFERCREATE
\brief Creates a GLBufferRef that represents a PBO.  See the documentation of CreateRGBAPBO() for more information- PBOs do not contain inherently typed data, these functions only differ in the contents of the GLBuffer::Descriptor they populate.
*/
VVGL_EXPORT GLBufferRef CreateYCbCrPBO(const int32_t & inTarget, const int32_t & inUsage, const Size & inSize, const void * inData=nullptr, const bool & inCreateInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());

///@}

#endif	//	!defined(VVGL_TARGETENV_GLES) && !defined(VVGL_TARGETENV_GLES3)




#if defined(VVGL_SDK_QT)
/*!
\name Qt-only buffer creation functions
\brief These functions create GLBuffers that aren't actually images at all.
*/
///@{

/*!
\ingroup VVGL_BUFFERCREATE
\brief Creates and returns an OpenGL buffer from the passed QImage.
\param inImg The QImage that will be uploaded to the GL texture.  The QImage will not be retained by the GLBuffer.
\param createInCurrentContext If true, the GL resource will be created in the current context (assumes that a GL context is active in the current thread).  If false, the GL resource will be created by the GL context owned by the buffer pool.
\param inPoolRef The pool that the GLBuffer should be created with.  When the GLBuffer is freed, its underlying GL resources will be returned to this pool (where they will be either freed or recycled).
*/
VVGL_EXPORT GLBufferRef CreateBufferForQImage(QImage * inImg, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());

///@}
#endif




#if defined(VVGL_SDK_MAC)
/*!
\name Mac-only buffer creation functions
\brief These functions create GLBuffers that aren't actually images at all.
*/
///@{

/*!
\ingroup VVGL_BUFFERCREATE
\brief Some GLBuffers are GL textures with a CPU backing- this function pushes the contents of the CPU backing to the GL texture.
\param inBufferRef The GLBuffer whose contents will be uploaded.  This has to be a GLBuffer with a CPU backing or the function won't do anything!
\param inContextRef This is the context that will be used to upload the image data from RAM to VRAM.  This must not be null!
*/
VVGL_EXPORT void PushTexRangeBufferRAMtoVRAM(const GLBufferRef & inBufferRef, const GLContextRef & inContextRef);
/*!
\ingroup VVGL_BUFFERCREATE
\brief Creates an 8 bit per channel RGBA OpenGL texture backed by an IOSurface that also gets created- the contents of this texture can be shared with other processes by sharing the IOSurface.
\param inSize The size of the buffer to create (in pixels).
\param createInCurrentContext If true, the GL resource will be created in the current context (assumes that a GL context is active in the current thread).  If false, the GL resource will be created by the GL context owned by the buffer pool.
\param inPoolRef The pool that the GLBuffer should be created with.  When the GLBuffer is freed, its underlying GL resources will be returned to this pool (where they will be either freed or recycled).
*/
VVGL_EXPORT GLBufferRef CreateRGBATexIOSurface(const Size & inSize, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
/*!
\ingroup VVGL_BUFFERCREATE
\brief Creates a 32 bit per channel RGBA OpenGL texture backed by an IOSurface that also gets created- the contents of this texture can be shared with other processes by sharing the IOSurface.
\param inSize The size of the buffer to create (in pixels).
\param createInCurrentContext If true, the GL resource will be created in the current context (assumes that a GL context is active in the current thread).  If false, the GL resource will be created by the GL context owned by the buffer pool.
\param inPoolRef The pool that the GLBuffer should be created with.  When the GLBuffer is freed, its underlying GL resources will be returned to this pool (where they will be either freed or recycled).
*/
VVGL_EXPORT GLBufferRef CreateRGBAFloatTexIOSurface(const Size & inSize, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
/*!
\ingroup VVGL_BUFFERCREATE
\brief Creates and returns a GLBuffer representing a texture that is backed by an IOSurface that was created by another process and passed to this process using the passed IOSurfaceID.
\param inID An IOSurfaceID (basically an integer) that was passed to the process.
\param createInCurrentContext If true, the GL resource will be created in the current context (assumes that a GL context is active in the current thread).  If false, the GL resource will be created by the GL context owned by the buffer pool.
\param inPoolRef The pool that the GLBuffer should be created with.  When the GLBuffer is freed, its underlying GL resources will be returned to this pool (where they will be either freed or recycled).
*/
VVGL_EXPORT GLBufferRef CreateRGBATexFromIOSurfaceID(const IOSurfaceID & inID, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
/*!
\ingroup VVGL_BUFFERCREATE
\brief Creates and returns an OpenGL texture that is populated with the contents of the passed CVPixelBufferRef.
\param inCVPB The CVPixelBufferRef whose contents will be uploaded to the texture.
\param inTexRange Whether or not the GL texture should be created using the Apple OpenGL texture range (DMA VRAM).
\param inIOSurface Whether or not the GL texture that is created should have an IOSurface backing (IOSurface backed textures are slower but can be shared between processes).
\param createInCurrentContext If true, the GL resource will be created in the current context (assumes that a GL context is active in the current thread).  If false, the GL resource will be created by the GL context owned by the buffer pool.
\param inPoolRef The pool that the GLBuffer should be created with.  When the GLBuffer is freed, its underlying GL resources will be returned to this pool (where they will be either freed or recycled).
*/
VVGL_EXPORT GLBufferRef CreateBufferForCVPixelBuffer(CVPixelBufferRef & inCVPB, const bool & inTexRange, const bool & inIOSurface, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
/*!
\ingroup VVGL_BUFFERCREATE
\brief Creates and returns an OpenGL texture that is populated with the contents of the passed CMSampleBufferRef (uses CreateBufferForCVPixelBuffer()).
\param n The CMSampleBufferRef whose contents will be uploaded to the texture. This must have a CVPixelBufferRef or the function won't work!
\param createInCurrentContext If true, the GL resource will be created in the current context (assumes that a GL context is active in the current thread).  If false, the GL resource will be created by the GL context owned by the buffer pool.
\param inPoolRef The pool that the GLBuffer should be created with.  When the GLBuffer is freed, its underlying GL resources will be returned to this pool (where they will be either freed or recycled).
*/
VVGL_EXPORT GLBufferRef CreateTexRangeFromCMSampleBuffer(CMSampleBufferRef & n, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
/*!
\ingroup VVGL_BUFFERCREATE
\brief Creates and returns an OpenGL RECT-target texture.
\param size The size of the buffer to create (in pixels).
\param createInCurrentContext If true, the GL resource will be created in the current context (assumes that a GL context is active in the current thread).  If false, the GL resource will be created by the GL context owned by the buffer pool.
\param inPoolRef The pool that the GLBuffer should be created with.  When the GLBuffer is freed, its underlying GL resources will be returned to this pool (where they will be either freed or recycled).
*/
VVGL_EXPORT GLBufferRef CreateRGBARectTex(const Size & size, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());

///@}
#endif




#if defined(VVGL_SDK_MAC) || defined(VVGL_SDK_IOS)
/*!
\name Mac- and iOS-only buffer creation functions
\brief These functions create GLBuffers that aren't actually images at all.
*/
///@{

/*!
\ingroup VVGL_BUFFERCREATE
\brief Creates and returns an OpenGL texture populated with the contents of the passed CGImageRef.
\param n The CGImageRef whose contents will be uploaded to the texture.
\param createInCurrentContext If true, the GL resource will be created in the current context (assumes that a GL context is active in the current thread).  If false, the GL resource will be created by the GL context owned by the buffer pool.
\param inPoolRef The pool that the GLBuffer should be created with.  When the GLBuffer is freed, its underlying GL resources will be returned to this pool (where they will be either freed or recycled).
*/
VVGL_EXPORT GLBufferRef CreateTexFromCGImageRef(const CGImageRef & n, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
/*!
\ingroup VVGL_BUFFERCREATE
\brief Creates and returns a cube-type OpenGL texture populated with the passed vector of CGImageRefs
\param inImgs A vector of CGImageRefs, each of which will be used to populate one of the faces of the cube texture.
\param createInCurrentContext If true, the GL resource will be created in the current context (assumes that a GL context is active in the current thread).  If false, the GL resource will be created by the GL context owned by the buffer pool.
\param inPoolRef The pool that the GLBuffer should be created with.  When the GLBuffer is freed, its underlying GL resources will be returned to this pool (where they will be either freed or recycled).
*/
VVGL_EXPORT GLBufferRef CreateCubeTexFromImages(const vector<CGImageRef> & inImgs, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
/*!
\ingroup VVGL_BUFFERCREATE
\brief Unpremultiplies the contents of the bitmap in the passed CGContextRef.
\param ctx The contents of this CGContextRef will be unpremultiplied.
*/
VVGL_EXPORT void CGBitmapContextUnpremultiply(CGContextRef ctx);

///@}
#endif




/*!
\name Mac- and iOS-only buffer creation functions
\brief These functions create GLBuffers that aren't actually images at all.
*/
///@{
#if defined(VVGL_SDK_MAC)
/*!
\ingroup VVGL_BUFFERCREATE
\brief Creates and returns a GLBuffer that represents the texture in the passed CVOpenGLTextureRef.  VVGL won't actually create any GL resources here- it's basically just using GLBuffer as a wrapper around the GL resource owned by the CVOpenGLTextureRef.
\param inTexRef The CVOpenGLTextureRef that will be used to populate the GLBuffer.  The GLBuffer's member variables are populated using the properties from the texture ref- no GL resources are created or destroyed.
\param createInCurrentContext If true, the GL resource will be created in the current context (assumes that a GL context is active in the current thread).  If false, the GL resource will be created by the GL context owned by the buffer pool.
\param inPoolRef The pool that the GLBuffer should be created with.  When the GLBuffer is freed, its underlying GL resources will be returned to this pool (where they will be either freed or recycled).
*/
VVGL_EXPORT GLBufferRef CreateBufferForCVGLTex(CVOpenGLTextureRef & inTexRef, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
#elif defined(VVGL_SDK_IOS)
/*!
\ingroup VVGL_BUFFERCREATE
\brief Creates and returns a GLBuffer that represents the texture in the passed CVOpenGLTextureRef.  VVGL won't actually create any GL resources here- it's basically just using GLBuffer as a wrapper around the GL resource owned by the CVOpenGLTextureRef.
\param inTexRef The CVOpenGLTextureRef that will be used to populate the GLBuffer.  The GLBuffer's member variables are populated using the properties from the texture ref- no GL resources are created or destroyed.
\param createInCurrentContext If true, the GL resource will be created in the current context (assumes that a GL context is active in the current thread).  If false, the GL resource will be created by the GL context owned by the buffer pool.
\param inPoolRef The pool that the GLBuffer should be created with.  When the GLBuffer is freed, its underlying GL resources will be returned to this pool (where they will be either freed or recycled).
*/
VVGL_EXPORT GLBufferRef CreateBufferForCVGLTex(CVOpenGLESTextureRef & inTexRef, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
#endif
///@}




}


#endif /* VVGL_GLBufferPool_hpp */
