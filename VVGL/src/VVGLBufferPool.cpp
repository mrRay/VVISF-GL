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
static VVGLBufferPoolRef _nullGlobalBufferPool = nullptr;




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
	
#if ISF_TARGET_MAC || ISF_TARGET_IOS
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
	
#if ISF_TARGET_MAC || ISF_TARGET_IOS
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
#if ISF_TARGET_MAC || ISF_TARGET_IOS
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
#if ISF_TARGET_GL3PLUS || ISF_TARGET_GLES3
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
#if ISF_TARGET_GL3PLUS || ISF_TARGET_GLES3
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
#if ISF_TARGET_GL3PLUS || ISF_TARGET_GLES3
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
/*
VVGLBufferPoolRef GetGlobalBufferPool()	{
	if (_globalBufferPool==nullptr)
		return nullptr;
	return *_globalBufferPool;
}
*/
const VVGLBufferPoolRef & GetGlobalBufferPool()	{
	//cout << __PRETTY_FUNCTION__ << endl;
	if (_globalBufferPool==nullptr)	{
		return _nullGlobalBufferPool;
	}
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


#if !ISF_TARGET_IOS
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
#if !ISF_TARGET_RPI
VVGLBufferRef CreateBGRAFloatCPUBackedTex(const Size & size, const bool & createInCurrentContext, const VVGLBufferPoolRef & inPoolRef)	{
	if (inPoolRef == nullptr)
		return nullptr;
	
	VVGLBuffer::Descriptor		desc;
	desc.type = VVGLBuffer::Type_Tex;
	desc.target = VVGLBuffer::Target_2D;
	desc.internalFormat = VVGLBuffer::IF_RGBA32F;
	desc.pixelFormat = VVGLBuffer::PF_BGRA;
	desc.pixelType = VVGLBuffer::PT_Float;
	desc.cpuBackingType = VVGLBuffer::Backing_Internal;
	desc.gpuBackingType = VVGLBuffer::Backing_Internal;
	desc.texRangeFlag = true;
	desc.texClientStorageFlag = true;
	desc.msAmount = 0;
	desc.localSurfaceID = 0;
	
	VVGLBufferRef	returnMe = inPoolRef->fetchMatchingFreeBuffer(desc, size);
	if (returnMe != nullptr)
		return returnMe;
	
	void			*bufferMemory = malloc(desc.backingLengthForSize(size));
	returnMe = inPoolRef->createBufferRef(desc, size, bufferMemory, size, createInCurrentContext);
	returnMe->parentBufferPool = inPoolRef;
	returnMe->backingID = VVGLBuffer::BackingID_Pixels;
	returnMe->backingContext = bufferMemory;
	returnMe->backingReleaseCallback = [](VVGLBuffer& inBuffer, void* inReleaseContext)	{
		free(inReleaseContext);
	};
	
	return returnMe;

}
void PushTexRangeBufferRAMtoVRAM(const VVGLBufferRef & inBufferRef, const VVGLContextRef & inContextRef)	{
	if (inBufferRef == nullptr)
		return;
	VVGLBuffer::Descriptor		&desc = inBufferRef->desc;
	if (desc.type != VVGLBuffer::Type_Tex)
		return;
	if (desc.cpuBackingType == VVGLBuffer::Backing_None)
		return;
	if (desc.gpuBackingType != VVGLBuffer::Backing_Internal)
		return;
	if (!desc.texRangeFlag)
		return;
	
	VVGL::Size		bSize = inBufferRef->srcRect.size;
	void			*pixels = inBufferRef->cpuBackingPtr;
	bool			doCompressedUpload = false;
	
	if (inContextRef != nullptr)
		inContextRef->makeCurrentIfNotCurrent();
	
	glActiveTexture(GL_TEXTURE0);
	glEnable(desc.target);
	glBindTexture(desc.target, inBufferRef->name);
	
	if (desc.texClientStorageFlag)
		glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_TRUE);
	
	switch (desc.internalFormat)	{
	case VVGLBuffer::IF_None:
	case VVGLBuffer::IF_R:
	case VVGLBuffer::IF_RGB:
	case VVGLBuffer::IF_RGBA:
	case VVGLBuffer::IF_RGBA8:
	case VVGLBuffer::IF_RGBA32F:
	case VVGLBuffer::IF_Depth24:
		doCompressedUpload = false;
		bSize = inBufferRef->size;
		break;
	case VVGLBuffer::IF_RGB_DXT1:
	case VVGLBuffer::IF_RGBA_DXT5:
	//case VVGLBuffer::IF_YCoCg_DXT5:	//	(flagged as duplicate case if un-commented, because both RGBA_DXT5 and YCoCg_DXT5 evaluate to the same internal format)
	case VVGLBuffer::IF_A_RGTC:
		doCompressedUpload = true;
		bSize = inBufferRef->backingSize;
		break;
	}
	
	if (!doCompressedUpload)	{
		//NSLog(@"\t\tuncompressed upload");
		glTexSubImage2D(desc.target,
			0,
			0,
			0,
			bSize.width,
			bSize.height,
			desc.pixelFormat,
			desc.pixelType,
			pixels);
		//NSLog(@"\t\tfinished uncompressed upload");
		//NSLog(@"\t\target is %ld, should be %ld",desc->target,GL_TEXTURE_RECTANGLE_EXT);
		//NSLog(@"\t\twidth/height is %f x %f",bSize.width,bSize.height);
		//NSLog(@"\t\tpixelFormat is %ld, should be %ld",desc->pixelFormat,GL_YCBCR_422_APPLE);
		//NSLog(@"\t\tpixelType is %ld, should be %ld",desc->pixelType,GL_UNSIGNED_SHORT_8_8_APPLE);
	}
	else	{
		//NSLog(@"\t\tcompressed upload! %s",__func__);
		//int					rowBytes = (pmHandle==nil) ? 0 : ((*pmHandle)->rowBytes)&0x7FFF;
		//int				rowBytes = VVBufferDescriptorCalculateCPUBackingForSize([b descriptorPtr], [b size]);
		//int				rowBytes = VVBufferDescriptorCalculateCPUBackingForSize([b descriptorPtr], bSize);
		//unsigned long		cpuBufferLength = VVBufferDescriptorCalculateCPUBackingForSize([b descriptorPtr], bSize);
		unsigned long		cpuBufferLength = desc.backingLengthForSize(bSize);
		//bSize = [b backingSize];
		glCompressedTexSubImage2D(desc.target,
			0,
			0,
			0,
			bSize.width,
			bSize.height,
			desc.internalFormat,
			//rowBytes * bSize.height,
			(GLsizei)cpuBufferLength,
			pixels);
		//NSLog(@"\t\tfinished compressed upload");
	}
	/*
	if (desc->backingType == VVBufferBack_GWorld)	{
		//if (pmHandle != nil)
		//	UnlockPixels(pmHandle);
	}
	*/
	if (desc.texClientStorageFlag)
		glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_FALSE);
	
	glBindTexture(desc.target, 0);
	//glDisable(desc->target);
	glFlush();
	
	//	timestamp the buffer, so we know a new frame has been pushed to it!
	//[VVBufferPool timestampThisBuffer:b];
	const VVGLBufferPoolRef	&bp = GetGlobalBufferPool();
	if (bp != nullptr)
		bp->timestampThisBuffer(inBufferRef);
}
#endif	//	!ISF_TARGET_RPI
#endif	//	!ISF_TARGET_IOS


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
#if ISF_TARGET_GL3PLUS || ISF_TARGET_GLES3
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









#if ISF_TARGET_MAC
VVGLBufferRef CreateRGBATexIOSurface(const Size & inSize, const bool & inCreateInCurrentContext, const VVGLBufferPoolRef & inPoolRef)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	if (inPoolRef == nullptr)
		return nullptr;
	
	VVGLBuffer::Descriptor	desc;
	
	desc.type = VVGLBuffer::Type_Tex;
	desc.target = VVGLBuffer::Target_Rect;
	desc.internalFormat = VVGLBuffer::IF_RGBA8;
	desc.pixelFormat = VVGLBuffer::PF_BGRA;
	desc.pixelType = VVGLBuffer::PT_UInt_8888_Rev;
	desc.cpuBackingType = VVGLBuffer::Backing_None;
	desc.gpuBackingType = VVGLBuffer::Backing_Internal;
	desc.texRangeFlag = false;
	desc.texClientStorageFlag = false;
	desc.msAmount = 0;
	desc.localSurfaceID = 1;
	
	VVGLBufferRef	returnMe = inPoolRef->createBufferRef(desc, inSize, nullptr, Size(), inCreateInCurrentContext);
	returnMe->parentBufferPool = inPoolRef;
	
	return returnMe;
}
VVGLBufferRef CreateRGBAFloatTexIOSurface(const Size & inSize, const bool & inCreateInCurrentContext, const VVGLBufferPoolRef & inPoolRef)	{
	if (inPoolRef == nullptr)
		return nullptr;
	
	VVGLBuffer::Descriptor	desc;
	
	desc.type = VVGLBuffer::Type_Tex;
	desc.target = VVGLBuffer::Target_Rect;
	desc.internalFormat = VVGLBuffer::IF_RGBA32F;
	desc.pixelFormat = VVGLBuffer::PF_RGBA;
	desc.pixelType = VVGLBuffer::PT_Float;
	desc.cpuBackingType = VVGLBuffer::Backing_None;
	desc.gpuBackingType = VVGLBuffer::Backing_Internal;
	desc.texRangeFlag = false;
	desc.texClientStorageFlag = false;
	desc.msAmount = 0;
	desc.localSurfaceID = 1;
	
	VVGLBufferRef	returnMe = inPoolRef->createBufferRef(desc, inSize, nullptr, Size(), inCreateInCurrentContext);
	returnMe->parentBufferPool = inPoolRef;
	
	return returnMe;
}
VVGLBufferRef CreateRGBATexFromIOSurfaceID(const IOSurfaceID & inID, const bool & inCreateInCurrentContext, const VVGLBufferPoolRef & inPoolRef)	{
	if (inPoolRef == nullptr)
		return nullptr;
	
	//	look up the surface for the ID i was passed, bail if i can't
	IOSurfaceRef		newSurface = IOSurfaceLookup(inID);
	if (newSurface == NULL)
		return nullptr;
	
	//	figure out how big the IOSurface is and what its pixel format is
	Size			newAssetSize(IOSurfaceGetWidth(newSurface), IOSurfaceGetHeight(newSurface));
	uint32_t		pixelFormat = IOSurfaceGetPixelFormat(newSurface);
	size_t			bytesPerRow = IOSurfaceGetBytesPerRow(newSurface);
	bool			isRGBAFloatTex = (bytesPerRow >= (32*4*newAssetSize.width/8)) ? true : false;
	//	make the buffer i'll be returning, set up as much of it as i can
	VVGLBufferRef		returnMe = make_shared<VVGLBuffer>(inPoolRef);
	inPoolRef->timestampThisBuffer(returnMe);
	returnMe->desc.type = VVGLBuffer::Type_Tex;
	returnMe->desc.target = VVGLBuffer::Target_Rect;
	
	switch (pixelFormat)	{
	case kCVPixelFormatType_32BGRA:	//	'BGRA'
	case VVGLBuffer::PF_BGRA:
		returnMe->desc.pixelFormat = VVGLBuffer::PF_BGRA;
		if (isRGBAFloatTex)	{
			returnMe->desc.internalFormat = VVGLBuffer::IF_RGBA32F;
			returnMe->desc.pixelType = VVGLBuffer::PT_Float;
		}
		else	{
			returnMe->desc.internalFormat = VVGLBuffer::IF_RGBA8;
			returnMe->desc.pixelType = VVGLBuffer::PT_UInt_8888_Rev;
		}
		break;
	case kCVPixelFormatType_32RGBA:	//	'RGBA'
	case VVGLBuffer::PF_RGBA:
		returnMe->desc.pixelFormat = VVGLBuffer::PF_RGBA;
		if (isRGBAFloatTex)	{
			returnMe->desc.internalFormat = VVGLBuffer::IF_RGBA32F;
			returnMe->desc.pixelType = VVGLBuffer::PT_Float;
		}
		else	{
			returnMe->desc.internalFormat = VVGLBuffer::IF_RGBA8;
			returnMe->desc.pixelType = VVGLBuffer::PT_UInt_8888_Rev;
		}
		break;
	case kCVPixelFormatType_422YpCbCr8:	//	'2vuy'
	case VVGLBuffer::PF_YCbCr_422:
		returnMe->desc.internalFormat = VVGLBuffer::IF_RGB;
		returnMe->desc.pixelFormat = VVGLBuffer::PF_YCbCr_422;
		returnMe->desc.pixelType = VVGLBuffer::PT_UShort88;
		break;
	default:
		cout << "\tERR: unknown pixel format (" << pixelFormat << ") in " << __PRETTY_FUNCTION__ << endl;
		returnMe->desc.internalFormat = VVGLBuffer::IF_RGBA8;
		returnMe->desc.pixelFormat = VVGLBuffer::PF_BGRA;
		returnMe->desc.pixelType = VVGLBuffer::PT_UInt_8888_Rev;
		break;
	}
	
	returnMe->desc.cpuBackingType = VVGLBuffer::Backing_None;
	returnMe->desc.gpuBackingType = VVGLBuffer::Backing_Internal;
	returnMe->desc.texRangeFlag = false;
	returnMe->desc.texClientStorageFlag = false;
	returnMe->desc.msAmount = 0;
	returnMe->desc.localSurfaceID = inID;
	
	returnMe->name = 0;
	returnMe->preferDeletion = true;
	returnMe->size = newAssetSize;
	returnMe->srcRect = Rect(0,0,newAssetSize.width,newAssetSize.height);
	//returnMe->flipped = 
	returnMe->backingSize = newAssetSize;
	//returnMe->backingReleaseCallback = 
	//returnMe->backingContext = 
	returnMe->backingID = VVGLBuffer::BackingID_RemoteIOSrf;
	
	returnMe->setRemoteSurfaceRef(newSurface);
	
	//	we can free the surface now that the buffer we'll be returning has retained it
	CFRelease(newSurface);
	
	//	...now that i've created and set up the VVGLBuffer, take care of the GL resource setup...
	
	//	grab a context lock so we can do stuff with the GL context
	lock_guard<recursive_mutex>		lock(inPoolRef->getContextLock());
	if (!inCreateInCurrentContext)	{
		VVGLContextRef					context = (inPoolRef==nullptr) ? nullptr : inPoolRef->getContext();
		context->makeCurrentIfNotCurrent();
	}
	CGLContextObj		cglCtx = CGLGetCurrentContext();
	if (cglCtx != NULL)	{
		glActiveTexture(GL_TEXTURE0);
		GLERRLOG
		if (inPoolRef->getContext()->version <= GLVersion_2)	{
			glEnable(returnMe->desc.target);
			GLERRLOG
		}
		glGenTextures(1,&(returnMe->name));
		GLERRLOG
		glBindTexture(returnMe->desc.target, returnMe->name);
		GLERRLOG
		CGLError		err = CGLTexImageIOSurface2D(cglCtx,
			returnMe->desc.target,
			returnMe->desc.internalFormat,
			newAssetSize.width,
			newAssetSize.height,
			returnMe->desc.pixelFormat,
			returnMe->desc.pixelType,
			newSurface,
			0);
		if (err != noErr)
			cout << "\tERR: " << err << " at CGLTexImageIOSurface2D() in " << __PRETTY_FUNCTION__ << endl;
		glTexParameteri(returnMe->desc.target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		GLERRLOG
		glTexParameteri(returnMe->desc.target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		GLERRLOG
		glTexParameteri(returnMe->desc.target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		GLERRLOG
		glTexParameteri(returnMe->desc.target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		GLERRLOG
		glFlush();
		GLERRLOG
		glBindTexture(returnMe->desc.target, 0);
		GLERRLOG
		if (inPoolRef->getContext()->version <= GLVersion_2)	{
			glDisable(returnMe->desc.target);
			GLERRLOG
		}
	}
	
	return returnMe;
}
VVGLBufferRef CreateBufferForCVPixelBuffer(CVPixelBufferRef & inCVPB, const bool & inTexRange, const bool & inIOSurface, const bool & createInCurrentContext, const VVGLBufferPoolRef & inPoolRef)	{
	if (inPoolRef==nullptr || inCVPB==NULL)
		return nullptr;
	
	VVGLBuffer::Descriptor	desc;
	
	desc.type = VVGLBuffer::Type_Tex;
	desc.target = VVGLBuffer::Target_Rect;
	desc.internalFormat = VVGLBuffer::IF_RGBA8;
	desc.pixelFormat = VVGLBuffer::PF_BGRA;
	desc.pixelType = VVGLBuffer::PT_UInt_8888_Rev;
	desc.cpuBackingType = VVGLBuffer::Backing_External;
	desc.gpuBackingType = VVGLBuffer::Backing_Internal;
	desc.texRangeFlag = inTexRange;
	desc.texClientStorageFlag = inTexRange;
	desc.msAmount = 0;
	desc.localSurfaceID = (inIOSurface) ? 1 : 0;
	
	//	get some basic properties of the CVPixelBufferRef
	unsigned long	cvpb_bytesPerRow = CVPixelBufferGetBytesPerRow(inCVPB);
	int				bitsPerPixel = 32;
	OSType			pixelFormat = CVPixelBufferGetPixelFormatType(inCVPB);
	
	//FourCCLog(@"\t\tpixel format is",pixelFormat);
	switch (pixelFormat)	{
		//case FOURCC_PACK('B','G','R','A'):
		case 'BGRA':
			//cout << "\t\tBGRA\n";
			bitsPerPixel = 32;
			break;
		//case FOURCC_PACK('2','v','u','y'):
		case '2vuy':
			//cout << "\t\t2vuy\n";
			bitsPerPixel = 16;
			desc.internalFormat = VVGLBuffer::IF_RGB;
			desc.pixelFormat = VVGLBuffer::PF_YCbCr_422;
			desc.pixelType = VVGLBuffer::PT_UShort88;
			break;
		//case FOURCC_PACK('D','X','t','1'):
		case 'DXt1':
			break;
		//case FOURCC_PACK('D','X','T','5'):
		case 'DXT5':
			break;
		//case FOURCC_PACK('D','Y','t','5'):
		case 'DYt5':
			break;
		default:
			{
				char			charPtr[5];
				charPtr[4] = 0;
				VVUnpackFourCC_toChar(pixelFormat,charPtr);
				//NSLog(@"\t\terr: unrecognized pixel format: %s, in %s",charPtr,__func__);
				cout << "\terr: unrecognized pixel format: " << charPtr << ", in " << __PRETTY_FUNCTION__ << endl;
				return nil;
			}
			break;
	}
	
	//NSRect			cvpb_srcRect = NSMakeRect(0,0,CVPixelBufferGetWidth(inCVPB),CVPixelBufferGetHeight(inCVPB));
	CGRect			cvpb_srcRect = CVImageBufferGetCleanRect(inCVPB);
	VVGL::Size		cvpb_backingSize = VVGL::Size(cvpb_bytesPerRow/(bitsPerPixel/8), CVPixelBufferGetDataSize(inCVPB)/cvpb_bytesPerRow);
	//NSLog(@"\t\ttotal size is %ld",CVPixelBufferGetDataSize(inCVPB));
	//NSLog(@"\t\tbytesPerRow is %ld",cvpb_bytesPerRow);
	//NSSizeLog(@"\t\tbackingSize is",cvpb_backingSize);
	//NSRectLog(@"\t\tsrcRect is",cvpb_srcRect);
	
	CVPixelBufferLockBaseAddress(inCVPB, kCVPixelBufferLock_ReadOnly);
	void			*cvpb_baseAddr = CVPixelBufferGetBaseAddress(inCVPB);
	
	VVGLBufferRef	returnMe = inPoolRef->createBufferRef(desc, cvpb_backingSize, cvpb_baseAddr, cvpb_backingSize, createInCurrentContext);
	returnMe->parentBufferPool = inPoolRef;
	
	//CVPixelBufferUnlockBaseAddress(inCVPB, 0);	//	don't unlock- we'll unlock when we release
	CVPixelBufferRetain(inCVPB);	//	the CVPixelBuffer is retained by the VVBuffer for the backing release callback context, here's where the retain count is incremented
	
	returnMe->backingID = VVGLBuffer::BackingID_CVPixBuf;
	returnMe->backingContext = inCVPB;
	returnMe->backingReleaseCallback = [](VVGLBuffer& inBuffer, void* inReleaseContext) {
		CVPixelBufferRef	tmpRef = (CVPixelBufferRef)inReleaseContext;
		if (tmpRef != nil)	{
			//NSLog(@"\t\tunlocking %p",tmpRef);
			CVPixelBufferUnlockBaseAddress(tmpRef, 0);
			CVPixelBufferRelease(tmpRef);
		}
	};
	returnMe->preferDeletion = true;
	returnMe->srcRect = VVGL::Rect(cvpb_srcRect.origin.x, cvpb_srcRect.origin.y, cvpb_srcRect.size.width, cvpb_srcRect.size.height);
	returnMe->flipped = (CVImageBufferIsFlipped(inCVPB)) ? true : false;
	
	return returnMe;
}
VVGLBufferRef CreateTexRangeFromCMSampleBuffer(CMSampleBufferRef & n, const bool & createInCurrentContext, const VVGLBufferPoolRef & inPoolRef)	{
	if (inPoolRef==nullptr || n==NULL)
		return nullptr;
	CMFormatDescriptionRef	cmDesc = CMSampleBufferGetFormatDescription(n);
	if (cmDesc == NULL)	{
		//NSLog(@"\t\terr: bailing, desc NULL, %s",__func__);
		cout << "\terr: bailing, desc NULL, " << __PRETTY_FUNCTION__ << endl;
		if (!CMSampleBufferIsValid(n))	{
			//NSLog(@"\t\terr: as a note, the sample buffer wasn't valid in %s",__func__);
			cout << "\terr: as a note, the sample buffer wasnt valid in " << __PRETTY_FUNCTION__ << endl;
		}
		return nullptr;
	}
	//	get the CVImage
	CVImageBufferRef		cvImg = CMSampleBufferGetImageBuffer(n);	//	don't need to free this
	VVGLBufferRef			returnMe = nullptr;
	if (cvImg!=NULL && (CFGetTypeID(cvImg) == CVPixelBufferGetTypeID()))	{
		//	make the actual buffer i'll be returning
		//returnMe = [self allocBufferForCVPixelBuffer:cvImg texRange:YES ioSurface:NO];
		returnMe = CreateBufferForCVPixelBuffer(cvImg, true, false, createInCurrentContext, inPoolRef);
		//	get the CMSampleBuffer timing info, apply it to the buffer
		CMTime				bufferTime = CMSampleBufferGetPresentationTimeStamp(n);
		returnMe->contentTimestamp = VVGL::Timestamp((uint64_t)(bufferTime.value), bufferTime.timescale);
		//double				timeInSec = (!CMTIME_IS_VALID(bufferTime)) ? 0. : CMTimeGetSeconds(bufferTime);
		//uint64_t			tmpTimestamp = SWatchAbsTimeUnitForTime(timeInSec);
		//[returnMe setContentTimestampFromPtr:&tmpTimestamp];
	}
	else	{
		if (cvImg == NULL)	{
			//NSLog(@"\t\terr: img null in %s",__func__);
			cout << "\terr: img null in " << __PRETTY_FUNCTION__ << endl;
		}
		else	{
			//NSLog(@"\t\terr: img not of expected type (%ld) in %s",CFGetTypeID(cvImg),__func__);
			cout << "\terr: img not of expected type (" << CFGetTypeID(cvImg) << ") in " << __PRETTY_FUNCTION__ << endl;
		}
	}
	return returnMe;
}
#endif




#if ISF_TARGET_MAC
VVGLBufferRef CreateBufferForCVGLTex(CVOpenGLTextureRef & inTexRef, const bool & createInCurrentContext, const VVGLBufferPoolRef & inPoolRef)
#elif ISF_TARGET_IOS
VVGLBufferRef CreateBufferForCVGLTex(CVOpenGLESTextureRef & inTexRef, const bool & createInCurrentContext, const VVGLBufferPoolRef & inPoolRef)
#endif
{
	//cout << __PRETTY_FUNCTION__ << " ... " << CVOpenGLTextureGetName(inTexRef) << endl;
	if (inTexRef == NULL)	{
		//NSLog(@"\t\terr: passed nil tex %s",__func__);
		cout << "\terr: passed nil tex in " << __PRETTY_FUNCTION__ << endl;
		return nullptr;
	}
#if !ISF_TARGET_IOS
	GLuint			texName = CVOpenGLTextureGetName(inTexRef);
#else	//	NOT !ISF_TARGET_IOS
	GLuint			texName = CVOpenGLESTextureGetName(inTexRef);
#endif	//	!ISF_TARGET_IOS
	if (texName <= 0)	{
		//NSLog(@"\t\terr: passed invalid tex num %s",__func__);
		cout << "\terr: passed invalid tex num " << __PRETTY_FUNCTION__ << endl;
		return nil;
	}
#if !ISF_TARGET_IOS
	if (CFGetTypeID(inTexRef) != CVOpenGLTextureGetTypeID())
#else	//	NOT !ISF_TARGET_IOS
	if (CFGetTypeID(inTexRef) != CVOpenGLESTextureGetTypeID())
#endif	//	!ISF_TARGET_IOS
	{
		//NSLog(@"\t\terr: CFTypeID of passed tex doesn't match expected %s",__func__);
		cout << "\terr: CFTypeID of passed tex doesnt match expected " << __PRETTY_FUNCTION__ << endl;
		return nil;
	}
	
	//VVBuffer			*returnMe = [[VVBuffer alloc] initWithPool:self];
	VVGLBufferRef		returnMe = make_shared<VVGLBuffer>(inPoolRef);
	//[VVBufferPool timestampThisBuffer:returnMe];
	GetGlobalBufferPool()->timestampThisBuffer(returnMe);
	//VVGLBuffer::Descriptor	&desc = returnMe->desc;
	//if (desc == nil)	{
	//	VVRELEASE(returnMe);
	//	return nil;
	//}
	VVGLBuffer::Descriptor	&desc = returnMe->desc;
	//desc->type = VVBufferType_Tex;
	desc.type = VVGLBuffer::Type_Tex;
#if !ISF_TARGET_IOS
	//desc->target = CVOpenGLTextureGetTarget(inTexRef);
	desc.target = (VVGL::VVGLBuffer::Target)CVOpenGLTextureGetTarget(inTexRef);
	//desc->internalFormat = VVBufferIF_RGBA8;
	desc.internalFormat = VVGLBuffer::IF_RGBA8;
	//desc->pixelFormat = VVBufferPF_BGRA;
	desc.pixelFormat = VVGLBuffer::PF_BGRA;
	//desc->pixelType = VVBufferPT_U_Int_8888_Rev;
	desc.pixelType = VVGLBuffer::PT_UInt_8888_Rev;
#else	//	NOT !ISF_TARGET_IOS
	//desc->target = CVOpenGLESTextureGetTarget(inTexRef);
	desc.target = CVOpenGLESTextureGetTarget(inTexRef);
	//desc->internalFormat = VVBufferIF_RGBA;
	desc.internalFormat = VVGLBuffer::IF_RGBA;
	//desc->pixelFormat = VVBufferPF_BGRA;
	desc.pixelFormat = VVGLBuffer::PF_BGRA;
	//desc->pixelType = VVBufferPT_U_Byte;
	desc.pixelType = VVGLBuffer::PT_UByte;
#endif	//	!ISF_TARGET_IOS
	//desc->cpuBackingType = VVBufferCPUBack_None;
	desc.cpuBackingType = VVGLBuffer::Backing_None;
	//desc->gpuBackingType = VVBufferGPUBack_External;
	desc.gpuBackingType = VVGLBuffer::Backing_External;
	//desc->name = texName;
	returnMe->name = texName;
	//desc->texRangeFlag = NO;
	desc.texRangeFlag = false;
	//desc->texClientStorageFlag = NO;
	desc.texClientStorageFlag = false;
	//desc->msAmount = 0;
	desc.msAmount = 0;
	//desc->localSurfaceID = 0;
	desc.localSurfaceID = 0;
	
	//CGSize				texSize = CVImageBufferGetDisplaySize(inTexRef);
	CGSize				texSize = CVImageBufferGetEncodedSize(inTexRef);
	//VVSIZE					texSize = VVMAKESIZE(CVPixelBufferGetWidth(inTexRef), CVPixelBufferGetHeight(inTexRef));
	//VVSIZE					texSize = VVMAKESIZE(1920,1080);
	//CGRect				cleanRect = CVImageBufferGetCleanRect(inTexRef);
	//NSSizeLog(@"\t\ttexSize is",texSize);
	//NSRectLog(@"\t\tcleanRect is",cleanRect);
	//[returnMe setPreferDeletion:YES];
	returnMe->preferDeletion = true;
	//[returnMe setSize:VVMAKESIZE(texSize.width,texSize.height)];
	returnMe->size = VVGL::Size(texSize.width, texSize.height);
	//[returnMe setSrcRect:VVMAKERECT(0,0,texSize.width,texSize.height)];
	returnMe->srcRect = VVGL::Rect(0,0,texSize.width,texSize.height);
#if !ISF_TARGET_IOS
	//[returnMe setFlipped:CVOpenGLTextureIsFlipped(inTexRef)];
	returnMe->flipped = (CVOpenGLTextureIsFlipped(inTexRef)) ? true : false;
#else	//	NOT !ISF_TARGET_IOS
	//[returnMe setFlipped:CVOpenGLESTextureIsFlipped(inTexRef)];
	returnMe->flipped = (CVOpenGLESTextureIsFlipped(inTexRef)) ? true : false;
#endif	//	!ISF_TARGET_IOS
	//[returnMe setBackingSize:[returnMe size]];
	returnMe->backingSize = returnMe->size;
	
	//[returnMe setBackingID:VVBufferBackID_CVTex];
	returnMe->backingID = VVGLBuffer::BackingID_CVTex;
#if !ISF_TARGET_IOS
	CVOpenGLTextureRetain(inTexRef);
#else	//	NOT !ISF_TARGET_IOS
	//CVOpenGLESTextureRetain(inTexRef);
	CVBufferRetain(inTexRef);
#endif	//	!ISF_TARGET_IOS
	//[returnMe setBackingReleaseCallback:VVBuffer_ReleaseCVGLT];
#if !ISF_TARGET_IOS
	returnMe->backingReleaseCallback = [](VVGLBuffer& inBuffer, void* inReleaseContext)	{
		CVOpenGLTextureRef	tmpRef = (CVOpenGLTextureRef)inReleaseContext;
		if (tmpRef != NULL)
			CVOpenGLTextureRelease(tmpRef);
	};
#else
	returnMe->backingReleaseCallback = [](VVGLBuffer& inBuffer, void* inReleaseContext)	{
		CVOpenGLESTextureRef	tmpRef = (CVOpenGLESTextureRef)inReleaseContext;
		if (tmpRef != NULL)
			CVOpenGLESTextureRelease(tmpRef);
	};
#endif
	
	//[returnMe setBackingReleaseCallbackContext:inTexRef];
	returnMe->backingContext = inTexRef;
	return returnMe;
}




}
