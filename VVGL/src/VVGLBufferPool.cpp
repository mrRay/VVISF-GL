#include "VVGLBufferPool.hpp"
#include "VVGLBuffer.hpp"
#include "VVGLBufferCopier.hpp"

#include <set>
#include <algorithm>

#if ISF_TARGET_MAC || ISF_TARGET_IOS
#import <CoreVideo/CoreVideo.h>
#endif


#define IDLEBUFFERCOUNT 30




namespace VVGL
{


using namespace std;




//	this is the global buffer pool
static VVGLBufferPoolRef * _globalBufferPool = nullptr;




/*	========================================	*/
#pragma mark --------------------- constructor/destructor


VVGLBufferPool::VVGLBufferPool(const VVGLContextRef & inShareCtx)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	//cout << "\tpassed ctx was " << inShareCtx << endl;
	//context = (inShareCtx==nullptr) ? new VVGLContext() : new VVGLContext(inShareCtx);
	context = (inShareCtx==nullptr) ? make_shared<VVGLContext>() : inShareCtx->newContextSharingMe();
	//cout << "\tcontext is " << *context << endl;
	//cout << "\tmy ctx is " << context << endl;
	freeBuffers.reserve(50);
	
#if ISF_TARGET_MAC
	colorSpace = CGColorSpaceCreateDeviceRGB();
#endif
}
/*
VVGLBufferPool::VVGLBufferPool(const VVGLContext * inShareCtx)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	//cout << "\tpassed ctx was " << inShareCtx << endl;
	//context = (inShareCtx==nullptr) ? new VVGLContext() : new VVGLContext(inShareCtx);
	context = (inShareCtx==nullptr) ? new VVGLContext() : inShareCtx->allocNewContextSharingMe();
	//cout << "\tcontext is " << *context << endl;
	//cout << "\tmy ctx is " << context << endl;
	freeBuffers.reserve(50);
	
#if ISF_TARGET_MAC
	colorSpace = CGColorSpaceCreateDeviceRGB();
#endif
}
*/
VVGLBufferPool::~VVGLBufferPool()	{
	cout << __PRETTY_FUNCTION__ << endl;
	
	lock_guard<recursive_mutex>		lock(contextLock);
	if (context != nullptr)	{
		//delete context;
		context = nullptr;
	}
#if ISF_TARGET_MAC
	CGColorSpaceRelease(colorSpace);
#endif
}


/*	========================================	*/
#pragma mark --------------------- public API


VVGLBufferRef VVGLBufferPool::createBufferRef(const VVGLBuffer::Descriptor & d, const Size & s, const void * b, const Size & bs, const bool & inCreateInCurrentContext)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	if (deleted)
		return nullptr;
	
	VVGLBufferRef		returnMe = nullptr;
	
	//	if i wasn't passed a backing ptr, try to find a free buffer matching the passed descriptor
	if (b==nullptr)
		returnMe = fetchMatchingFreeBuffer(d, s);
	//	if i found an unused buffer, i can return it immediately
	if (returnMe != nullptr)	{
		//cout << "\tfound a matching buffer, returning that instead!\n";
		return returnMe;
	}
	
	//	...if i'm here then i couldn't find a free buffer, and i need to create one
	
	//	make the buffer
	returnMe = make_shared<VVGLBuffer>();
	//	copy the passed descriptor to the buffer i just created
	VVGLBuffer::Descriptor 		&newBufferDesc = returnMe->desc;
	newBufferDesc = d;
	
	//	grab a context lock so we can do stuff with the GL context
	lock_guard<recursive_mutex>		lock(contextLock);
	
#if ISF_TARGET_MAC
	CGLError			err = kCGLNoError;
#endif
	if (!inCreateInCurrentContext)	{
		if (context == nullptr)
			return nullptr;
		//context->makeCurrentIfNull();
		//context->makeCurrent();
		context->makeCurrentIfNotCurrent();
	}
	
	//	cout << "\terr: " << err << " in " << __PRETTY_FUNCTION__ << endl;
#if ISF_TARGET_MAC
	IOSurfaceRef		newSurfaceRef = nullptr;
	uint32_t			cpuBackingSize = returnMe->backingLengthForSize(bs);
	uint32_t			bytesPerRow = cpuBackingSize/bs.height;
#endif
	uint32_t			pixelFormat = 0x00;
	bool				compressedTex = false;
	
#if !ISF_TARGET_IOS && !ISF_TARGET_RPI
	switch (d.internalFormat)	{
	case VVGLBuffer::IF_RGB_DXT1:
	case VVGLBuffer::IF_RGBA_DXT5:
	//case IF_YCoCg_DXT5:	//	???
	case VVGLBuffer::IF_A_RGTC:
		compressedTex = true;
		break;
	default:
		break;
	}
#endif
	
	switch (d.pixelFormat)	{
	case VVGLBuffer::PF_None:
	case VVGLBuffer::PF_Depth:
		break;
#if !ISF_TARGET_IOS && !ISF_TARGET_RPI
	case VVGLBuffer::PF_R:
#endif
	case VVGLBuffer::PF_RGBA:
		//pixelFormat = kCVPixelFormatType_32RGBA;
		pixelFormat = 'RGBA';
		break;
#if !ISF_TARGET_IOS && !ISF_TARGET_RPI
	case VVGLBuffer::PF_BGRA:
		//pixelFormat = kCVPixelFormatType_32BGRA;
		pixelFormat = 'BGRA';
		break;
	case VVGLBuffer::PF_YCbCr_422:
		//pixelFormat = kCVPixelFormatType_422YpCbCr8;
		pixelFormat = '2vuy';
		break;
#elif ISF_TARGET_RPI
	case VVGLBuffer::PF_BGRA:
		pixelFormat = 'BGRA';
		break;
#endif
	}
	
	//	create the GL resources, populating the buffer descriptor where appropriate
	switch (d.type)	{
	case VVGLBuffer::Type_RB:
		//	generate the renderbuffer
		glGenRenderbuffers(1, &(returnMe->name));
		GLERRLOG
		//	bind the renderbuffer, set it up
		glBindRenderbuffer(returnMe->desc.target, returnMe->name);
		GLERRLOG
#if !ISF_TARGET_RPI
		if (returnMe->desc.msAmount > 0)	{
			glRenderbufferStorageMultisample(returnMe->desc.target,
				returnMe->desc.msAmount,
				returnMe->desc.pixelFormat,
				s.width,
				s.height);
			GLERRLOG
		}
#endif
		//	unbind the renderbuffer
		glBindRenderbuffer(returnMe->desc.target, 0);
		GLERRLOG
		//	flush!
		if (!inCreateInCurrentContext)	{
			glFlush();
			GLERRLOG
		}
		break;
	case VVGLBuffer::Type_FBO:
		glGenFramebuffers(1, &(returnMe->name));
		GLERRLOG
		if (!inCreateInCurrentContext)	{
			glFlush();
			GLERRLOG
		}
		break;
	case VVGLBuffer::Type_Tex:
#if ISF_TARGET_MAC
		//	if necessary, create an iosurface ref
		if (newBufferDesc.localSurfaceID != 0)	{
			void				*keys[] = { (void *)kIOSurfaceIsGlobal, (void *)kIOSurfaceWidth, (void *)kIOSurfaceHeight, (void *)kIOSurfaceBytesPerElement, (void *)kIOSurfacePixelFormat };
			uint32_t			tmpBytesPerElement = bytesPerRow/s.width;
			void				*vals[] = { (void *)kCFBooleanTrue, (void *)CFNumberCreate(NULL,kCFNumberFloat64Type,&s.width), (void *)CFNumberCreate(NULL,kCFNumberFloat64Type,&s.height), (void *)CFNumberCreate(NULL,kCFNumberSInt32Type,&tmpBytesPerElement), (void *)CFNumberCreate(NULL,kCFNumberSInt32Type,&pixelFormat) };
			CFDictionaryRef		props = CFDictionaryCreate(NULL, (const void **)&keys, (const void **)&vals, 5, NULL, NULL);
			newSurfaceRef = IOSurfaceCreate(props);
			if (newSurfaceRef == NULL)	{
				cout << "\tERR at IOSurfaceCreate() in " << __PRETTY_FUNCTION__ << endl;
				newBufferDesc.localSurfaceID = 0;
			}
			//	free the stuff i created!
			CFRelease(vals[1]);
			CFRelease(vals[2]);
			CFRelease(vals[3]);
			CFRelease(vals[4]);
			CFRelease(props);
			//	...the localSurfaceID in the descriptor gets updated later, when we call VVGLBuffer::setLocalSurfaceRef()
		}
#endif
		//	enable the tex target, gen the texture, and bind it
		glActiveTexture(GL_TEXTURE0);
		GLERRLOG
#if !ISF_TARGET_IOS && !ISF_TARGET_RPI
		if (context->version <= GLVersion_2)	{
			glEnable(newBufferDesc.target);
			GLERRLOG
		}
#endif
		glGenTextures(1, &(returnMe->name));
		GLERRLOG
		glBindTexture(newBufferDesc.target, returnMe->name);
		GLERRLOG
		
#if ISF_TARGET_MAC
		//	if i want a texture range, there are a couple cases where i can apply it
		if (newBufferDesc.texRangeFlag)	{
			if (b != nullptr)	{
				glTextureRangeAPPLE(newBufferDesc.target, cpuBackingSize, b);
				GLERRLOG
			}
		}
		
		if (newBufferDesc.texClientStorageFlag)	{
			//		client storage hint- my app will keep the (CPU-based) 'pixels' around until 
			//all referencing GL textures have been deleted.  this lets me skip the app->framework 
			//copy IF THE TEXTURE WIDTH IS A MULTIPLE OF 32 BYTES!
			glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_TRUE);
			GLERRLOG
		}
#endif
		
#if !ISF_TARGET_IOS && !ISF_TARGET_RPI
		//	setup basic tex defaults
		glPixelStorei(GL_UNPACK_SKIP_ROWS, GL_FALSE);
		GLERRLOG
		glPixelStorei(GL_UNPACK_SKIP_PIXELS, GL_FALSE);
		GLERRLOG
		glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
		GLERRLOG
#endif
		
		glTexParameteri(newBufferDesc.target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		GLERRLOG
		glTexParameteri(newBufferDesc.target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		GLERRLOG
		//glTexParameteri(newBufferDesc.target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		//glTexParameteri(newBufferDesc.target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(newBufferDesc.target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		GLERRLOG
		glTexParameteri(newBufferDesc.target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		GLERRLOG
		//glTexParameteri(newBufferDesc.target, GL_TEXTURE_WRAP_S, GL_REPEAT);
		//glTexParameteri(newBufferDesc.target, GL_TEXTURE_WRAP_T, GL_REPEAT);
		
#if !ISF_TARGET_IOS && !ISF_TARGET_RPI
		//if (newBufferDesc.pixelFormat == VVGLBuffer::PF_Depth)	{
		//	glTexParameteri(newBufferDesc.target, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
		//	GLERRLOG
		//}
		
		if (newBufferDesc.pixelFormat == VVGLBuffer::PF_Depth)	{
			glTexParameteri(newBufferDesc.target, GL_TEXTURE_STORAGE_HINT_APPLE, GL_STORAGE_CACHED_APPLE);
			GLERRLOG
		}
		else	{
			glTexParameteri(newBufferDesc.target, GL_TEXTURE_STORAGE_HINT_APPLE, GL_STORAGE_SHARED_APPLE);
			GLERRLOG
		}
#endif
		
#if ISF_TARGET_MAC
		//	if there's a surface ref, set it up as the texture!
		if (newSurfaceRef != nil)	{
			CGLContextObj		cgl_ctx = CGLGetCurrentContext();
			err = CGLTexImageIOSurface2D(cgl_ctx,
				newBufferDesc.target,
				newBufferDesc.internalFormat,
				s.width,
				s.height,
				newBufferDesc.pixelFormat,
				newBufferDesc.pixelType,
				newSurfaceRef,
				0);
			if (b != nil)	{
				glTexSubImage2D(newBufferDesc.target,
					0,
					0,
					0,
					s.width,
					s.height,
					newBufferDesc.pixelFormat,
					newBufferDesc.pixelType,
					b);
				GLERRLOG
			}
		}
#endif
		
		
		//	if there's no surface ref, or there is, but there was a problem associating it with the texture, set it up as a straight-up texture!
#if ISF_TARGET_MAC
		if (newSurfaceRef==nullptr || err!=kCGLNoError)	{
#endif
			if (compressedTex)	{
				glTexImage2D(newBufferDesc.target,
					0,
					newBufferDesc.internalFormat,
					s.width,
					s.height,
					0,
					newBufferDesc.pixelFormat,
					newBufferDesc.pixelType,
					NULL);
				GLERRLOG
			}
			else	{
				//NSLog(@"\t\ttarget is %ld (should be %ld)",newBufferDesc.target,GL_TEXTURE_2D);
				//NSLog(@"\t\tinternal is %ld (should be %ld)",newBufferDesc.internalFormat,GL_RGBA);
				//NSLog(@"\t\tpf is %ld (should be %ld)",newBufferDesc.pixelFormat,GL_RGBA);
				//NSLog(@"\t\tpt is %ld (should be %ld)",newBufferDesc.pixelType,GL_UNSIGNED_BYTE);
				//NSLog(@"\t\tbytes are %p",b);
				glTexImage2D(newBufferDesc.target,
					0,
					newBufferDesc.internalFormat,
					s.width,
					s.height,
					0,
					newBufferDesc.pixelFormat,
					newBufferDesc.pixelType,
					b);
				GLERRLOG
			}
#if ISF_TARGET_MAC
		}
#endif
		

#if ISF_TARGET_MAC
		if (newBufferDesc.texClientStorageFlag)	{
			glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_FALSE);
			GLERRLOG
		}
#endif
		
		//	un-bind the tex and disable the target
		glBindTexture(newBufferDesc.target, 0);
		GLERRLOG
#if !ISF_TARGET_IOS && !ISF_TARGET_RPI
		if (context->version <= GLVersion_2)	{
			glDisable(newBufferDesc.target);
			GLERRLOG
		}
#endif
		if (!inCreateInCurrentContext)	{
			//	flush!
			glFlush();
			GLERRLOG
		}
		
		break;
	case VVGLBuffer::Type_PBO:
		glGenBuffers(1, &(returnMe->name));
		GLERRLOG
		if (!inCreateInCurrentContext)	{
			//	flush!
			glFlush();
			GLERRLOG
		}
		//	"pack" means this PBO will be used to transfer pixel data TO a PBO (glReadPixels(), glGetTexImage())
		//	"unpack" means this PBO will be used to transfer pixel data FROM a PBO (glDrawPixels(), glTexImage2D(), glTexSubImage2D())
		
		//				decoding "GL_STREAM_DRAW, GL_STREAM_READ, etc:
		//	STREAM		write once, read at most a few times
		//	STATIC		write once, read many times
		//	DYNAMIC		write many times, read many times
		//	--------	--------	--------
		//	DRAW		CPU -> GL
		//	READ		GL -> CPU
		//	COPY		GL -> GL
		break;
	case VVGLBuffer::Type_VBO:
	case VVGLBuffer::Type_EBO:
#if ISF_TARGET_GL3PLUS
	case VVGLBuffer::Type_VAO:
#endif
		//	left intentionally blank- VBOs, EBOs, and VAOs are created in their respective factory functions
		break;
	}
	
	//CGLUnlockContext(cgl_ctx);
	
	returnMe->size = s;
	returnMe->srcRect = {0,0,s.width,s.height};
	returnMe->backingSize = bs;
	returnMe->cpuBackingPtr = (void *)b;
	
	//	timestamp the buffer!
	timestampThisBuffer(returnMe);
	
#if ISF_TARGET_MAC
	if (newSurfaceRef != nullptr)	{
		returnMe->setLocalSurfaceRef(newSurfaceRef);
		CFRelease(newSurfaceRef);
		newSurfaceRef = nullptr;
	}
#endif
	
	return returnMe;
}


VVGLBufferRef VVGLBufferPool::fetchMatchingFreeBuffer(const VVGLBuffer::Descriptor & desc, const Size & size)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	if (deleted)
		return nullptr;
	
	//	get a lock on the array of free buffers
	lock_guard<mutex>		lock(freeBuffersLock);
	
	VVGLBufferRef			returnMe = nullptr;
	
	vector<VVGLBufferRef>::iterator		it;
	int						tmpIndex = 0;
	for (it=freeBuffers.begin(); it!=freeBuffers.end(); ++it)	{
		//	if this buffer is comparable to the passed descriptor and can be used for recycling...
		VVGLBuffer			*bufferPtr = (*it).get();
		if (bufferPtr->isComparableForRecycling(desc))	{
			//	check to make sure that the dimensions of this buffer are compatible...
			bool				sizeIsOK = false;
			VVGLBuffer::Descriptor & desc = bufferPtr->desc;
			//VVGLBuffer::Descriptor	*desc = &(bufferPtr->desc);
			switch (desc.type)	{
			case VVGLBuffer::Type_FBO:
				sizeIsOK = true;
				break;
			case VVGLBuffer::Type_RB:
			case VVGLBuffer::Type_Tex:
			case VVGLBuffer::Type_PBO:
				if (bufferPtr->size == size)
					sizeIsOK = true;
				break;
			case VVGLBuffer::Type_VBO:
			case VVGLBuffer::Type_EBO:
#if ISF_TARGET_GL3PLUS
			case VVGLBuffer::Type_VAO:
#endif
				break;
			}
			
			//	if the size of this buffer is compatible with what's requested...
			if (sizeIsOK)	{
#if ISF_TARGET_MAC
				//	check to make sure that the IOSurface-related aspects of this buffer are compatible
				IOSurfaceRef		srf = bufferPtr->getLocalSurfaceRef();
				if ((desc.localSurfaceID!=0 && srf!=nullptr)	||
				(desc.localSurfaceID==0 && srf==nullptr))	{
#endif
					//	if i'm here, this buffer is a match and i want to use it
					returnMe = *it;
					//	remove the buffer from the array
					freeBuffers.erase(freeBuffers.begin()+tmpIndex);
					//	reset the idleCount to 0 so it's "fresh" (so it gets returned to the pool when it's no longer needed)
					(*returnMe).idleCount = 0;
					//	break out of the for loop
					break;
#if ISF_TARGET_MAC
				}
#endif
			}
		}
		++tmpIndex;
	}
	
	//	timestamp the buffer
	if (returnMe != nullptr)
		timestampThisBuffer(returnMe);
	
	return returnMe;
}

void VVGLBufferPool::housekeeping()	{
	//cout << __PRETTY_FUNCTION__ << endl;
	//cout << "\tthis is " << this << endl;
	
	lock_guard<mutex>		lock(freeBuffersLock);
	
	bool			needsToClearStuff = false;
	for_each(freeBuffers.begin(), freeBuffers.end(), [&](const VVGLBufferRef & n)	{
		(*n).idleCount++;
		if ((*n).idleCount >= IDLEBUFFERCOUNT)
			needsToClearStuff = true;
	});
	
	//	if there are indices that need to be removed...
	if (needsToClearStuff)	{
		auto		removeIt = remove_if(freeBuffers.begin(), freeBuffers.end(), [&](VVGLBufferRef n){ return (*n).idleCount >= IDLEBUFFERCOUNT; });
		freeBuffers.erase(removeIt, freeBuffers.end());
	}
}
void VVGLBufferPool::purge()	{
	{
		lock_guard<mutex>		lock(freeBuffersLock);
		for_each(freeBuffers.begin(), freeBuffers.end(), [&](const VVGLBufferRef & n)	{
			n->idleCount = (IDLEBUFFERCOUNT+1);
		});
	}
	housekeeping();
}
ostream & operator<<(ostream & os, const VVGLBufferPool & n)	{
	os << "<VVGLBufferPool " << &n << ">";
	return os;
}

void VVGLBufferPool::flush()	{
	lock_guard<recursive_mutex>		lock(contextLock);
	if (context != nullptr)	{
		//context->makeCurrentIfNull();
		//context->makeCurrent();
		context->makeCurrentIfNotCurrent();
		glFlush();
		GLERRLOG
	}
}



/*	========================================	*/
#pragma mark --------------------- protected methods (backend)


void VVGLBufferPool::returnBufferToPool(VVGLBuffer * inBuffer)	{
	//cout << __PRETTY_FUNCTION__ << ", " << *inBuffer << endl;
	
	if (inBuffer == nullptr)
		return;
	
	//	if we've been flagged for deletion we're just waiting for the buffers still "in the wild" to get released, and as such we should just release this regardless.
	if (deleted)	{
		releaseBufferResources(inBuffer);
		return;
	}
	
	//	get a lock for the freeBuffers array
	lock_guard<mutex>		lock(freeBuffersLock);
	
	//	if 'freeBuffers' is at capacity, increase its capacity
	if (freeBuffers.size() == freeBuffers.capacity())
		freeBuffers.reserve(freeBuffers.capacity()+25);
	
	//	make a shared ptr for the passed buffer, stick it in the vector
	freeBuffers.emplace_back(make_shared<VVGLBuffer>(*inBuffer));
	
	//	now clear out some vars in the passed buffer- we don't want to release a backing if we're putting it back in the pool
	inBuffer->backingReleaseCallback = nullptr;
	inBuffer->backingContext = nullptr;
	
}

void VVGLBufferPool::releaseBufferResources(VVGLBuffer * inBuffer)	{
	//cout << __PRETTY_FUNCTION__ << "... " << *inBuffer << endl;
	
	if (inBuffer == nullptr)
		return;
	
	lock_guard<recursive_mutex>		lock(contextLock);
	if (context == nullptr)
		return;
	
	//context->makeCurrentIfNull();
	//context->makeCurrent();
	context->makeCurrentIfNotCurrent();
	
	switch (inBuffer->desc.type)	{
	case VVGLBuffer::Type_RB:
		glDeleteRenderbuffers(1, &inBuffer->name);
		GLERRLOG
		break;
	case VVGLBuffer::Type_FBO:
		glDeleteFramebuffers(1, &inBuffer->name);
		GLERRLOG
		break;
	case VVGLBuffer::Type_Tex:
		glDeleteTextures(1, &inBuffer->name);
		GLERRLOG
		break;
	case VVGLBuffer::Type_PBO:
		glDeleteBuffers(1, &inBuffer->name);
		GLERRLOG
		break;
	case VVGLBuffer::Type_VBO:
		glDeleteBuffers(1, &inBuffer->name);
		GLERRLOG
		break;
	case VVGLBuffer::Type_EBO:
		glDeleteBuffers(1, &inBuffer->name);
		GLERRLOG
		break;
#if ISF_TARGET_GL3PLUS
	case VVGLBuffer::Type_VAO:
		glDeleteVertexArrays(1, &inBuffer->name);
		GLERRLOG
		break;
#endif
	}
	glFlush();
	GLERRLOG
}




/*	========================================	*/
#pragma mark --------------------- friends





/*	========================================	*/
#pragma mark *************** non-member functions ***************


/*
VVGLBufferPoolRef CreateGlobalBufferPool(const VVGLContext * inShareCtx)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	
	VVGLBufferPoolRef		returnMe = make_shared<VVGLBufferPool>(inShareCtx);
	if (_globalBufferPool != nullptr)	{
		delete _globalBufferPool;
		_globalBufferPool = nullptr;
	}
	_globalBufferPool = new shared_ptr<VVGLBufferPool>(returnMe);
	
	//	create the global buffer copier (it will use the global buffer pool for its shared ctx/pxl fmt)
	CreateGlobalBufferCopier();
	
	return returnMe;
}
*/
VVGLBufferPoolRef CreateGlobalBufferPool(const VVGLContextRef & inShareCtx)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	
	VVGLBufferPoolRef		returnMe = make_shared<VVGLBufferPool>(inShareCtx);
	if (_globalBufferPool != nullptr)	{
		delete _globalBufferPool;
		_globalBufferPool = nullptr;
	}
	_globalBufferPool = new shared_ptr<VVGLBufferPool>(returnMe);
	
	//	create the global buffer copier (it will use the global buffer pool for its shared ctx/pxl fmt)
	CreateGlobalBufferCopier();
	
	return returnMe;
}
VVGLBufferPoolRef GetGlobalBufferPool()	{
	//cout << __PRETTY_FUNCTION__ << endl;
	if (_globalBufferPool==nullptr)
		return nullptr;
	return *_globalBufferPool;
}


/*	========================================	*/
#pragma mark --------------------- buffer creation methods


VVGLBufferRef CreateRGBATex(const Size & size, const bool & inCreateInCurrentContext, const VVGLBufferPoolRef & inPoolRef)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	if (inPoolRef == nullptr)
		return nullptr;
	
	VVGLBuffer::Descriptor	desc;
	
	desc.type = VVGLBuffer::Type_Tex;
	desc.target = VVGLBuffer::Target_2D;
#if ISF_TARGET_MAC
	desc.internalFormat = VVGLBuffer::IF_RGBA8;
#else
	desc.internalFormat = VVGLBuffer::IF_RGBA;
#endif
	desc.pixelFormat = VVGLBuffer::PF_RGBA;
	desc.pixelType = VVGLBuffer::PT_UByte;
	desc.cpuBackingType = VVGLBuffer::Backing_None;
	desc.gpuBackingType = VVGLBuffer::Backing_Internal;
	desc.texRangeFlag = false;
	desc.texClientStorageFlag = false;
	desc.msAmount = 0;
	desc.localSurfaceID = 0;
	
	VVGLBufferRef	returnMe = inPoolRef->createBufferRef(desc, size, nullptr, Size(), inCreateInCurrentContext);
	returnMe->parentBufferPool = inPoolRef;
	
	return returnMe;
}
VVGLBufferRef CreateRGBAFloatTex(const Size & size, const bool & inCreateInCurrentContext, const VVGLBufferPoolRef & inPoolRef)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	if (inPoolRef == nullptr)
		return nullptr;
	
	VVGLBuffer::Descriptor	desc;
	
	desc.type = VVGLBuffer::Type_Tex;
	desc.target = VVGLBuffer::Target_2D;
#if !ISF_TARGET_RPI
	desc.internalFormat = VVGLBuffer::IF_RGBA32F;
	desc.pixelFormat = VVGLBuffer::PF_RGBA;
	desc.pixelType = VVGLBuffer::PT_Float;
#else
	desc.internalFormat = VVGLBuffer::IF_RGBA;
	desc.pixelFormat = VVGLBuffer::PF_RGBA;
	//desc.pixelType = VVGLBuffer::PT_HalfFloat;
	desc.pixelType = VVGLBuffer::PT_UByte;
#endif
	//desc.pixelFormat = VVGLBuffer::PF_RGBA;
	//desc.pixelType = VVGLBuffer::PT_Float;
	desc.cpuBackingType = VVGLBuffer::Backing_None;
	desc.gpuBackingType = VVGLBuffer::Backing_Internal;
	desc.texRangeFlag = false;
	desc.texClientStorageFlag = false;
	desc.msAmount = 0;
	desc.localSurfaceID = 0;
	
	VVGLBufferRef	returnMe = inPoolRef->createBufferRef(desc, size, nullptr, Size(), inCreateInCurrentContext);
	returnMe->parentBufferPool = inPoolRef;
	
	return returnMe;
}
VVGLBufferRef CreateBGRATex(const Size & size, const bool & inCreateInCurrentContext, const VVGLBufferPoolRef & inPoolRef)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	if (inPoolRef == nullptr)
		return nullptr;
	
	VVGLBuffer::Descriptor	desc;
	
	desc.type = VVGLBuffer::Type_Tex;
	desc.target = VVGLBuffer::Target_2D;
#if ISF_TARGET_MAC
	desc.internalFormat = VVGLBuffer::IF_RGBA8;
#else
	desc.internalFormat = VVGLBuffer::IF_RGBA;
#endif
	desc.pixelFormat = VVGLBuffer::PF_BGRA;
	desc.pixelType = VVGLBuffer::PT_UByte;
	desc.cpuBackingType = VVGLBuffer::Backing_None;
	desc.gpuBackingType = VVGLBuffer::Backing_Internal;
	desc.texRangeFlag = false;
	desc.texClientStorageFlag = false;
	desc.msAmount = 0;
	desc.localSurfaceID = 0;
	
	VVGLBufferRef	returnMe = inPoolRef->createBufferRef(desc, size, nullptr, Size(), inCreateInCurrentContext);
	returnMe->parentBufferPool = inPoolRef;
	
	return returnMe;
}
VVGLBufferRef CreateBGRAFloatTex(const Size & size, const bool & inCreateInCurrentContext, const VVGLBufferPoolRef & inPoolRef)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	if (inPoolRef == nullptr)
		return nullptr;
	
	VVGLBuffer::Descriptor	desc;
	
	desc.type = VVGLBuffer::Type_Tex;
	desc.target = VVGLBuffer::Target_2D;
#if !ISF_TARGET_RPI
	desc.internalFormat = VVGLBuffer::IF_RGBA32F;
	desc.pixelFormat = VVGLBuffer::PF_BGRA;
	desc.pixelType = VVGLBuffer::PT_Float;
#else
	desc.internalFormat = VVGLBuffer::IF_RGBA;
	desc.pixelFormat = VVGLBuffer::PF_BGRA;
	//desc.pixelType = VVGLBuffer::PT_HalfFloat;
	desc.pixelType = VVGLBuffer::PT_UByte;
#endif
	//desc.pixelFormat = VVGLBuffer::PF_RGBA;
	//desc.pixelType = VVGLBuffer::PT_Float;
	desc.cpuBackingType = VVGLBuffer::Backing_None;
	desc.gpuBackingType = VVGLBuffer::Backing_Internal;
	desc.texRangeFlag = false;
	desc.texClientStorageFlag = false;
	desc.msAmount = 0;
	desc.localSurfaceID = 0;
	
	VVGLBufferRef	returnMe = inPoolRef->createBufferRef(desc, size, nullptr, Size(), inCreateInCurrentContext);
	returnMe->parentBufferPool = inPoolRef;
	
	return returnMe;
}
VVGLBufferRef CreateRB(const Size & size, const bool & inCreateInCurrentContext, const VVGLBufferPoolRef & inPoolRef)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	
	if (inPoolRef == nullptr)
		return nullptr;
	
	VVGLBuffer::Descriptor	desc;
	
	desc.type = VVGLBuffer::Type_RB;
	desc.target = VVGLBuffer::Target_2D;
#if !ISF_TARGET_RPI
	desc.internalFormat = VVGLBuffer::IF_Depth24;
#else
	desc.internalFormat = VVGLBuffer::IF_Depth16;
#endif
	desc.pixelFormat = VVGLBuffer::PF_Depth;
	desc.pixelType = VVGLBuffer::PT_UByte;
	desc.cpuBackingType = VVGLBuffer::Backing_None;
	desc.gpuBackingType = VVGLBuffer::Backing_Internal;
	desc.texRangeFlag = false;
	desc.texClientStorageFlag = false;
	desc.msAmount = 0;
	desc.localSurfaceID = 0;
	
	VVGLBufferRef	returnMe = inPoolRef->createBufferRef(desc, size, nullptr, Size(), inCreateInCurrentContext);
	returnMe->parentBufferPool = inPoolRef;
	
	return returnMe;
}
VVGLBufferRef CreateFBO(const bool & inCreateInCurrentContext, const VVGLBufferPoolRef & inPoolRef)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	
	if (inPoolRef == nullptr)
		return nullptr;
	
	VVGLBuffer::Descriptor	desc;
	
	desc.type = VVGLBuffer::Type_FBO;
	//desc.target = VVGLBuffer::Target_2D;
	//desc.internalFormat = VVGLBuffer::IF_Depth24;
	//desc.pixelFormat = VVGLBuffer::PF_Depth;
	//desc.pixelType = VVGLBuffer::PT_UByte;
	desc.cpuBackingType = VVGLBuffer::Backing_None;
	desc.gpuBackingType = VVGLBuffer::Backing_Internal;
	//desc.texRangeFlag = false;
	//desc.texClientStorageFlag = false;
	//desc.msAmount = 0;
	//desc.localSurfaceID = 0;
	
	VVGLBufferRef	returnMe = inPoolRef->createBufferRef(desc, Size(), nullptr, Size(), inCreateInCurrentContext);
	returnMe->parentBufferPool = inPoolRef;
	
	return returnMe;
}
VVGLBufferRef CreateDepthBuffer(const Size & size, const bool & inCreateInCurrentContext, const VVGLBufferPoolRef & inPoolRef)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	
	if (inPoolRef == nullptr)
		return nullptr;
	
	VVGLBuffer::Descriptor	desc;
	
	desc.type = VVGLBuffer::Type_Tex;
	desc.target = VVGLBuffer::Target_2D;
#if !ISF_TARGET_RPI
	desc.internalFormat = VVGLBuffer::IF_Depth24;
#else
	desc.internalFormat = VVGLBuffer::IF_Depth16;
#endif
	desc.pixelFormat = VVGLBuffer::PF_Depth;
	desc.pixelType = VVGLBuffer::PT_UByte;
	desc.cpuBackingType = VVGLBuffer::Backing_None;
	desc.gpuBackingType = VVGLBuffer::Backing_Internal;
	//desc.texRangeFlag = false;
	//desc.texClientStorageFlag = false;
	//desc.msAmount = 0;
	//desc.localSurfaceID = 0;
	
	VVGLBufferRef	returnMe = inPoolRef->createBufferRef(desc, size, nullptr, Size(), inCreateInCurrentContext);
	returnMe->parentBufferPool = inPoolRef;
	
	return returnMe;
}

VVGLBufferRef CreateFromExistingGLTexture(const int32_t & inTexName, const int32_t & inTexTarget, const int32_t & inTexIntFmt, const int32_t & inTexPxlFmt, const int32_t & inTexPxlType, const Size & inTexSize, const bool & inTexFlipped, const Rect & inImgRectInTex, const void * inReleaseCallbackContext, const VVGLBuffer::BackingReleaseCallback & inReleaseCallback, const VVGLBufferPoolRef & inPoolRef)	{
	VVGLBufferRef		returnMe = make_shared<VVGLBuffer>();
	returnMe->desc.type = VVGLBuffer::Type_Tex;
	returnMe->desc.target = static_cast<VVGLBuffer::Target>(inTexTarget);
	returnMe->desc.internalFormat = static_cast<VVGLBuffer::InternalFormat>(inTexIntFmt);
	returnMe->desc.pixelFormat = static_cast<VVGLBuffer::PixelFormat>(inTexPxlFmt);
	returnMe->desc.pixelType = static_cast<VVGLBuffer::PixelType>(inTexPxlType);
	returnMe->desc.cpuBackingType = VVGLBuffer::Backing_External;
	returnMe->desc.gpuBackingType = VVGLBuffer::Backing_External;
	returnMe->name = inTexName;
	returnMe->preferDeletion = true;
	returnMe->size = inTexSize;
	returnMe->srcRect = inImgRectInTex;
	returnMe->flipped = inTexFlipped;
	returnMe->backingReleaseCallback = inReleaseCallback;
	returnMe->backingContext = const_cast<void*>(inReleaseCallbackContext);
	
	if (inPoolRef != nullptr)
		inPoolRef->timestampThisBuffer(returnMe);
	
	return returnMe;
}


#if ISF_TARGET_MAC
VVGLBufferRef CreateRGBARectTex(const Size & size, const bool & inCreateInCurrentContext, const VVGLBufferPoolRef & inPoolRef)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	if (inPoolRef == nullptr)
		return nullptr;
	
	VVGLBuffer::Descriptor	desc;
	
	desc.type = VVGLBuffer::Type_Tex;
	desc.target = VVGLBuffer::Target_Rect;
//#if ISF_TARGET_MAC
	desc.internalFormat = VVGLBuffer::IF_RGBA8;
//#else
//	desc.internalFormat = VVGLBuffer::IF_RGBA;
//#endif
	desc.pixelFormat = VVGLBuffer::PF_RGBA;
	desc.pixelType = VVGLBuffer::PT_UByte;
	desc.cpuBackingType = VVGLBuffer::Backing_None;
	desc.gpuBackingType = VVGLBuffer::Backing_Internal;
	desc.texRangeFlag = false;
	desc.texClientStorageFlag = false;
	desc.msAmount = 0;
	desc.localSurfaceID = 0;
	
	VVGLBufferRef	returnMe = inPoolRef->createBufferRef(desc, size, nullptr, Size(), inCreateInCurrentContext);
	returnMe->parentBufferPool = inPoolRef;
	
	return returnMe;
}
#endif


VVGLBufferRef CreateVBO(const void * inBytes, const size_t & inByteSize, const int32_t & inUsage, const bool & inCreateInCurrentContext, const VVGLBufferPoolRef & inPoolRef)	{
	if (inPoolRef == nullptr)
		return nullptr;
	VVGLBufferRef		returnMe = make_shared<VVGLBuffer>(inPoolRef);
	VVGLBuffer::Descriptor &	desc = returnMe->desc;
	
	desc.type = VVGLBuffer::Type_VBO;
	desc.target = VVGLBuffer::Target_None;
	desc.internalFormat = VVGLBuffer::IF_None;
	desc.pixelFormat = VVGLBuffer::PF_None;
	desc.pixelType = VVGLBuffer::PT_Float;
	desc.cpuBackingType = VVGLBuffer::Backing_None;
	desc.gpuBackingType = VVGLBuffer::Backing_Internal;
	desc.texRangeFlag = false;
	desc.texClientStorageFlag = false;
	desc.msAmount = 0;
	desc.localSurfaceID = 0;
	
	if (!inCreateInCurrentContext)	{
		if (inPoolRef->getContext() != nullptr)	{
			//inPoolRef->context->makeCurrentIfNull();
			//inPoolRef->context->makeCurrent();
			inPoolRef->getContext()->makeCurrentIfNotCurrent();
		}
	}
	
	glGenBuffers(1, &(returnMe->name));
	GLERRLOG
	glBindBuffer(GL_ARRAY_BUFFER, returnMe->name);
	GLERRLOG
	glBufferData(GL_ARRAY_BUFFER, inByteSize, inBytes, inUsage);
	GLERRLOG
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	GLERRLOG
	if (!inCreateInCurrentContext)	{
		glFlush();
		GLERRLOG
	}
	
	returnMe->size = {0.,0.};
	returnMe->srcRect = {0., 0., 0., 0.};
	inPoolRef->timestampThisBuffer(returnMe);
	returnMe->preferDeletion = true;
	
	return returnMe;
}
VVGLBufferRef CreateEBO(const void * inBytes, const size_t & inByteSize, const int32_t & inUsage, const bool & inCreateInCurrentContext, const VVGLBufferPoolRef & inPoolRef)	{
	if (inPoolRef == nullptr)
		return nullptr;
	VVGLBufferRef		returnMe = make_shared<VVGLBuffer>(inPoolRef);
	VVGLBuffer::Descriptor &	desc = returnMe->desc;
	
	desc.type = VVGLBuffer::Type_EBO;
	desc.target = VVGLBuffer::Target_None;
	desc.internalFormat = VVGLBuffer::IF_None;
	desc.pixelFormat = VVGLBuffer::PF_None;
	desc.pixelType = VVGLBuffer::PT_Float;
	desc.cpuBackingType = VVGLBuffer::Backing_None;
	desc.gpuBackingType = VVGLBuffer::Backing_Internal;
	desc.texRangeFlag = false;
	desc.texClientStorageFlag = false;
	desc.msAmount = 0;
	desc.localSurfaceID = 0;
	
	if (!inCreateInCurrentContext)	{
		if (inPoolRef->getContext() != nullptr)	{
			//inPoolRef->context->makeCurrentIfNull();
			//inPoolRef->context->makeCurrent();
			inPoolRef->getContext()->makeCurrentIfNotCurrent();
		}
	}
	
	glGenBuffers(1, &(returnMe->name));
	GLERRLOG
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, returnMe->name);
	GLERRLOG
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, inByteSize, inBytes, inUsage);
	GLERRLOG
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	GLERRLOG
	if (!inCreateInCurrentContext)	{
		glFlush();
		GLERRLOG
	}
	
	returnMe->size = {0.,0.};
	returnMe->srcRect = {0., 0., 0., 0.};
	inPoolRef->timestampThisBuffer(returnMe);
	returnMe->preferDeletion = true;
	
	return returnMe;
}
#if ISF_TARGET_GL3PLUS
VVGLBufferRef CreateVAO(const bool & inCreateInCurrentContext, const VVGLBufferPoolRef & inPoolRef)	{
	if (inPoolRef == nullptr)
		return nullptr;
	VVGLBufferRef		returnMe = make_shared<VVGLBuffer>(inPoolRef);
	VVGLBuffer::Descriptor &	desc = returnMe->desc;
	
	desc.type = VVGLBuffer::Type_VAO;
	desc.target = VVGLBuffer::Target_None;
	desc.internalFormat = VVGLBuffer::IF_None;
	desc.pixelFormat = VVGLBuffer::PF_None;
	desc.pixelType = VVGLBuffer::PT_Float;
	desc.cpuBackingType = VVGLBuffer::Backing_None;
	desc.gpuBackingType = VVGLBuffer::Backing_Internal;
	desc.texRangeFlag = false;
	desc.texClientStorageFlag = false;
	desc.msAmount = 0;
	desc.localSurfaceID = 0;
	
	if (!inCreateInCurrentContext)	{
		if (inPoolRef->getContext() != nullptr)	{
			//inPoolRef->context->makeCurrentIfNull();
			//inPoolRef->context->makeCurrent();
			inPoolRef->getContext()->makeCurrentIfNotCurrent();
		}
	}
	
	glGenVertexArrays(1, &(returnMe->name));
	GLERRLOG
	if (!inCreateInCurrentContext)	{
		glFlush();
		GLERRLOG
	}
	
	returnMe->size = {0.,0.};
	returnMe->srcRect = {0., 0., 0., 0.};
	inPoolRef->timestampThisBuffer(returnMe);
	returnMe->preferDeletion = true;
	
	return returnMe;
}
#endif




#if !ISF_TARGET_MAC && !ISF_TARGET_IOS
VVGLBufferRef CreateTexFromImage(const string & inPath, const bool & inCreateInCurrentContext, const VVGLBufferPoolRef & inPoolRef)	{
	return nullptr;
}
VVGLBufferRef CreateCubeTexFromImagePaths(const vector<string> & inPaths, const bool & inCreateInCurrentContext, const VVGLBufferPoolRef & inPoolRef)	{
	return nullptr;
}
#endif	//	#if !ISF_TARGET_MAC && !ISF_TARGET_IOS




}
