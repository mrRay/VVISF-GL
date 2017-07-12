#ifndef VVGLBuffer_hpp
#define VVGLBuffer_hpp

#include <vector>
#include <chrono>
#include "VVGLContext.hpp"

/*
#if ISF_TARGET_MAC
	//#import <TargetConditionals.h>
	//#include <objc/objc.h>
	//#include <IOSurface/IOSurface.h>
	//#import <OpenGL/OpenGL.h>
	//#import <OpenGL/gl.h>
	//#import <OpenGL/glext.h>
	//#import <OpenGL/gl3.h>
	//#import <OpenGL/gl3ext.h>
#elif ISF_TARGET_IOS
	//#import <OpenGLES/EAGL.h>
	#import <OpenGLES/ES3/glext.h>
	//#import <GLKit/GLKit.h>
#elif ISF_TARGET_GLFW
	#include <glad/glad.h>
	#include <GLFW/glfw3.h>
#elif ISF_TARGET_RPI
	#include "bcm_host.h"
	//#include <GLES/gl.h>
	//#include <GLES/glext.h>
	#include <GLES2/gl2.h>
	#include <GLES2/gl2ext.h>
	#include <EGL/egl.h>
	#include <EGL/eglext.h>
#endif
*/



namespace VVGL	{


using namespace std;




class VVGLBuffer	{
	
	public:
		//	this defines a callback that releases the backing
		using BackingReleaseCallback = function<void(VVGLBuffer&, void*)>;
		
		//	enums that define basic attributes of the buffer
		enum Type	{
			Type_RB,
			Type_FBO,
			Type_Tex,
			Type_PBO,
			Type_VBO,
			Type_EBO,
#if ISF_TARGET_GL3PLUS
			Type_VAO,
#endif
		};
		
		//	enums describing the various GL object (usually texture) properties- split up b/c availability depends on platform
#if ISF_TARGET_MAC
		#include "VVGLBuffer_Mac_Enums.h"
#elif ISF_TARGET_RPI
		#include "VVGLBuffer_RPI_Enums.h"
#elif ISF_TARGET_IOS
		#include "VVGLBuffer_IOS_Enums.h"
#elif ISF_TARGET_GLFW
		#include "VVGLBuffer_GLFW_Enums.h"
#endif
		
		
		enum Backing	{
			Backing_None,	//!<	there is no resource
			Backing_Internal,	//!<	the resource was created by this framework (and should be deleted by this framework)
			Backing_External	//!<	the resource was created outside of this framework, and this buffer should be freed immediately when done
		};
		enum BackingID	{	///	The "BackingID" is an arbitrary enum that isn't used functionally by this lib.  This enum- and VVGLBuffer's corresponding "backingID" member- exist to help track where an VVGLBuffer came from (if it was made from pixels, from another object, etc).  this is purely for use by other frameworks/libs.
			BackingID_None,
			BackingID_GWorld,
			BackingID_Pixels,
			BackingID_CVPixBuf,
			BackingID_CVTex,
			BackingID_NSBitImgRep,
			BackingID_RemoteIOSrf,
			BackingID_External
		};
	
	
	//	the Descriptor struct uses the above enums to describe the hardware attributes of a texture.
	public:
		struct Descriptor	{
			Type					type = Type_Tex;
			Target					target = Target_2D;
#if ISF_TARGET_MAC
			InternalFormat			internalFormat = IF_RGBA8;
#else
			InternalFormat			internalFormat = IF_RGBA;
#endif
			PixelFormat				pixelFormat = PF_RGBA;
			PixelType				pixelType = PT_UByte;
			Backing					cpuBackingType = Backing_None;
			Backing					gpuBackingType = Backing_None;
			bool					texRangeFlag = false;
			bool					texClientStorageFlag = false;
			uint32_t				msAmount = 0;
			uint32_t				localSurfaceID = 0;
			
			uint32_t backingLengthForSize(const Size & s) const;
		};
	
	
	//	most vars are public for ease of access- the VVGLBuffer class is, for the most part, read-only (member vars are populated when the underlying GL resource is created)
	public:
		Descriptor				desc;
		
		uint32_t				name = 0;
		bool					preferDeletion = false;
		Size					size = { 0, 0 };
		Rect					srcRect = { 0, 0, 0, 0 };
		bool					flipped = false;
		Size					backingSize = { 0, 0 };
		Timestamp				contentTimestamp = { (uint64_t)0, (uint32_t)0 };
		
		BackingReleaseCallback	backingReleaseCallback = nullptr;	//	this function is called when the image resources need to be released
		void					*backingContext = nullptr;	//	weak ref. this is an arbitrary data pointer that is stored with a buffer for use with the buffer release callback
		BackingID				backingID = BackingID_None;	//	totally optional, used to describe where the backing came from. sometimes you want to know what kind of backing a VVGLBuffer has, and access it.  there's an enum describing some of the more common sources, and you can define and use your own values here.
		void					*cpuBackingPtr = nullptr;
	
	private:
#if ISF_TARGET_MAC
		//id						userInfo = nullptr;	//	RETAINED, nil by default.  not used by this class- stick whatever you want here and it will be retained for the lifetime of this buffer.  retained if you copy the buffer!
		IOSurfaceRef			localSurfaceRef = nullptr;	//	RETAINED, nil by default.  the "local" surface ref was created by this process.
		IOSurfaceRef			remoteSurfaceRef = nullptr;	//	RETAINED, nil by default.  the "remote" surface ref was created by another process (so this should probably be released immediately).
#endif
	
	public:
		VVGLBufferPoolRef		parentBufferPool = nullptr;
		VVGLBufferRef			copySourceBuffer = nullptr;
		int						idleCount = 0;
	
	
	//	public methods
	public:
		VVGLBuffer() = default;
		VVGLBuffer(VVGLBufferPoolRef inParentPool);
		VVGLBuffer(const VVGLBuffer &);
		virtual ~VVGLBuffer();
		friend ostream & operator<<(ostream & os, const VVGLBuffer & n);
		
		//	copy assignment operators are disabled to prevent accidents
		VVGLBuffer& operator=(const VVGLBuffer&) = delete;
		VVGLBuffer& operator=(VVGLBuffer&) = delete;
		//VVGLBuffer (VVGLBuffer&&) = default;
		
#if ISF_TARGET_MAC
		//	getter/setters
		//id getUserInfo() const;
		//void setUserInfo(id n);
		IOSurfaceRef getLocalSurfaceRef() const;
		void setLocalSurfaceRef(const IOSurfaceRef & n);
		IOSurfaceRef getRemoteSurfaceRef() const;
		void setRemoteSurfaceRef(const IOSurfaceRef & n);
#endif
		
		//	member methods
		bool isComparableForRecycling(const VVGLBuffer::Descriptor & n) const;
		uint32_t backingLengthForSize(Size s) const;
		Rect glReadySrcRect() const;
		/*
		Rect croppedSrcRect(Rect & cropRect, bool & takeFlipIntoAccount) const;
		*/
		bool isFullFrame() const;
		bool isNPOT2DTex() const;
		bool isPOT2DTex() const;
#if ISF_TARGET_MAC
		bool safeToPublishToSyphon() const;
#endif
		bool isContentMatch(VVGLBuffer & n) const;
		//void draw(const Rect & dst) const;
		string getDescriptionString() const;
};




//	VVGLBufferCopy returns an VVGLBufferRef of a new VVGLBufferRef instance that shares the same GL/graphic resources as the passed buffer.  one of its uses is for texture atlases- with this function you can create VVGLBuffers that refer to a region of a large VVGLBuffer/texture, which is explicitly retained.
VVGLBufferRef VVGLBufferCopy(const VVGLBufferRef & n);




}




#endif /* VVGLBuffer_hpp */
