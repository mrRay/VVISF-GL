#ifndef VVGL_GLBuffer_hpp
#define VVGL_GLBuffer_hpp

#include "VVGL_Defines.hpp"

#include <vector>
#include <chrono>
#include "GLContext.hpp"




namespace VVGL	{


using namespace std;




class VVGL_EXPORT GLBuffer	{
	
	public:
		//	this defines a callback that releases the backing
		using BackingReleaseCallback = function<void(GLBuffer&, void*)>;
		
		//	enums that define basic attributes of the buffer
		enum Type	{
			Type_RB,
			Type_FBO,
			Type_Tex,
			Type_PBO,
			Type_VBO,
			Type_EBO,
			Type_VAO,
		};
		
		//	enums describing the various GL object (usually texture) properties- split up b/c availability depends on platform
#if defined(VVGL_SDK_MAC)
		#include "GLBuffer_Mac_Enums.h"
#elif defined(VVGL_SDK_RPI)
		#include "GLBuffer_RPI_Enums.h"
#elif defined(VVGL_SDK_IOS)
		#include "GLBuffer_IOS_Enums.h"
#elif defined(VVGL_SDK_GLFW)
		#include "GLBuffer_GLFW_Enums.h"
#elif defined(VVGL_SDK_QT)
		#include "GLBuffer_Qt_Enums.h"
#endif
		
		
		enum Backing	{
			Backing_None,	//!<	there is no resource
			Backing_Internal,	//!<	the resource was created by this framework (and should be deleted by this framework)
			Backing_External	//!<	the resource was created outside of this framework, and this buffer should be freed immediately when done
		};
		enum BackingID	{	///	The "BackingID" is an arbitrary enum that isn't used functionally by this lib.  This enum- and GLBuffer's corresponding "backingID" member- exist to help track where an GLBuffer came from (if it was made from pixels, from another object, etc).  this is purely for use by other frameworks/libs.
			BackingID_None,
			BackingID_GWorld,
			BackingID_Pixels,
			BackingID_CVPixBuf,
			BackingID_CVTex,
			BackingID_NSBitImgRep,
			BackingID_RemoteIOSrf,
			BackingID_External,
			BackingID_QImage
		};
	
	
	//	the Descriptor struct uses the above enums to describe the hardware attributes of a texture.
	public:
		struct Descriptor	{
			Type					type = Type_Tex;
			Target		target = Target_2D;
#if defined(VVGL_SDK_MAC)
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
	
	
	//	most vars are public for ease of access- the GLBuffer class is, for the most part, read-only (member vars are populated when the underlying GL resource is created)
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
		BackingID				backingID = BackingID_None;	//	totally optional, used to describe where the backing came from. sometimes you want to know what kind of backing a GLBuffer has, and access it.  there's an enum describing some of the more common sources, and you can define and use your own values here.
		void					*cpuBackingPtr = nullptr;
	
	private:
#if defined(VVGL_SDK_MAC)
		//id						userInfo = nullptr;	//	RETAINED, nil by default.  not used by this class- stick whatever you want here and it will be retained for the lifetime of this buffer.  retained if you copy the buffer!
		IOSurfaceRef			localSurfaceRef = nullptr;	//	RETAINED, nil by default.  the "local" surface ref was created by this process.
		IOSurfaceRef			remoteSurfaceRef = nullptr;	//	RETAINED, nil by default.  the "remote" surface ref was created by another process (so this should probably be released immediately).
#endif
	
	public:
		GLBufferPoolRef		parentBufferPool = nullptr;
		GLBufferRef			copySourceBuffer = nullptr;
		int						idleCount = 0;
	
	
	//	public methods
	public:
		GLBuffer() = default;
		GLBuffer(GLBufferPoolRef inParentPool);
		GLBuffer(const GLBuffer &);
		virtual ~GLBuffer();
		friend ostream & operator<<(ostream & os, const GLBuffer & n);
		
		//	copy assignment operators are disabled to prevent accidents
		GLBuffer& operator=(const GLBuffer&) = delete;
		//GLBuffer& operator=(GLBuffer&) = delete;
		//GLBuffer (GLBuffer&&) = default;
		
		//	use this to create a shallow copy (memberwise copy)
		GLBuffer * allocShallowCopy();
		
#if defined(VVGL_SDK_MAC)
		//	getter/setters
		//id getUserInfo() const;
		//void setUserInfo(id n);
		IOSurfaceRef getLocalSurfaceRef() const;
		void setLocalSurfaceRef(const IOSurfaceRef & n);
		IOSurfaceRef getRemoteSurfaceRef() const;
		void setRemoteSurfaceRef(const IOSurfaceRef & n);
#endif
		
		//	member methods
		bool isComparableForRecycling(const GLBuffer::Descriptor & n) const;
		uint32_t backingLengthForSize(Size s) const;
		Rect glReadySrcRect() const;
		/*
		Rect croppedSrcRect(Rect & cropRect, bool & takeFlipIntoAccount) const;
		*/
		bool isFullFrame() const;
		bool isNPOT2DTex() const;
		bool isPOT2DTex() const;
#if defined(VVGL_SDK_MAC)
		bool safeToPublishToSyphon() const;
#endif
		bool isContentMatch(GLBuffer & n) const;
		//void draw(const Rect & dst) const;
		string getDescriptionString() const;
};




//	GLBufferCopy returns an GLBufferRef of a new GLBufferRef instance that shares the same GL/graphic resources as the passed buffer.  one of its uses is for texture atlases- with this function you can create GLBuffers that refer to a region of a large GLBuffer/texture, which is explicitly retained.
VVGL_EXPORT GLBufferRef GLBufferCopy(const GLBufferRef & n);




}




#endif /* VVGL_GLBuffer_hpp */
