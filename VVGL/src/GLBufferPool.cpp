#include "GLBufferPool.hpp"
#include "GLBuffer.hpp"
#include "GLTexToTexCopier.hpp"

#include "VVGL_Base.hpp"

#include <set>
#include <algorithm>

#if defined(VVGL_SDK_QT)
#include <QImage>
#endif


#define IDLEBUFFERCOUNT 30




namespace VVGL
{


using namespace std;




//	this is the global buffer pool
static GLBufferPoolRef * _globalBufferPool = nullptr;
static GLBufferPoolRef _nullGlobalBufferPool = nullptr;




/*	========================================	*/
#pragma mark --------------------- constructor/destructor


GLBufferPool::GLBufferPool(const GLContextRef & inCtx)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	//cout << "\tpassed ctx was " << inCtx << endl;
	//context = (inShareCtx==nullptr) ? new GLContext() : new GLContext(inShareCtx);
	//context = (inShareCtx==nullptr) ? CreateNewGLContextRef() : inShareCtx->newContextSharingMe();
	context = (inCtx==nullptr) ? CreateNewGLContextRef() : inCtx;
	//cout << "\tcontext is " << *context << endl;
	//cout << "\tmy ctx is " << context << endl;
	freeBuffers.reserve(50);
	
#if defined(VVGL_SDK_MAC) || defined(VVGL_SDK_IOS)
	colorSpace = CGColorSpaceCreateDeviceRGB();
#endif
}
GLBufferPool::~GLBufferPool()	{
	//cout << __PRETTY_FUNCTION__ << endl;
	
	lock_guard<recursive_mutex>		lock(contextLock);
	if (context != nullptr)	{
		//delete context;
		context = nullptr;
	}
#if defined(VVGL_SDK_MAC) || defined(VVGL_SDK_IOS)
	CGColorSpaceRelease(colorSpace);
#endif
}


/*	========================================	*/
#pragma mark --------------------- public API


GLBufferRef GLBufferPool::createBufferRef(const GLBuffer::Descriptor & d, const Size & s, const void * b, const Size & bs, const bool & inCreateInCurrentContext)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	if (deleted)
		return nullptr;
	
	GLBufferRef		returnMe = nullptr;
	
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
	returnMe = make_shared<GLBuffer>();
	//	copy the passed descriptor to the buffer i just created
	GLBuffer::Descriptor 		&newBufferDesc = returnMe->desc;
	newBufferDesc = d;
	
	//	grab a context lock so we can do stuff with the GL context
	lock_guard<recursive_mutex>		lock(contextLock);
	
#if defined(VVGL_SDK_MAC)
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
#if defined(VVGL_SDK_MAC)
	IOSurfaceRef		newSurfaceRef = nullptr;
	uint32_t			cpuBackingSize = returnMe->backingLengthForSize(bs);
	uint32_t			bytesPerRow = cpuBackingSize/bs.height;
#endif
	uint32_t			pixelFormat = 0x00;
	bool				compressedTex = false;
	
#if defined(VVGL_SDK_MAC)
	switch (d.internalFormat)	{
	case GLBuffer::IF_RGB_DXT1:
	case GLBuffer::IF_RGBA_DXT5:
	//case IF_YCoCg_DXT5:	//	???
	case GLBuffer::IF_A_RGTC:
		compressedTex = true;
		break;
	default:
		break;
	}
#endif
	
	switch (d.pixelFormat)	{
	case GLBuffer::PF_None:
	case GLBuffer::PF_Depth:
		break;
#if !defined(VVGL_SDK_IOS) && !defined(VVGL_SDK_RPI)
	case GLBuffer::PF_R:
#endif
	case GLBuffer::PF_RGBA:
		//pixelFormat = kCVPixelFormatType_32RGBA;
		pixelFormat = 'RGBA';
		break;
#if !defined(VVGL_SDK_IOS) && !defined(VVGL_SDK_RPI)
	case GLBuffer::PF_BGRA:
		//pixelFormat = kCVPixelFormatType_32BGRA;
		pixelFormat = 'BGRA';
		break;
	case GLBuffer::PF_YCbCr_422:
		//pixelFormat = kCVPixelFormatType_422YpCbCr8;
		pixelFormat = '2vuy';
		break;
#else
	case GLBuffer::PF_BGRA:
		pixelFormat = 'BGRA';
		break;
#endif
	}
	
	//	create the GL resources, populating the buffer descriptor where appropriate
	switch (d.type)	{
	case GLBuffer::Type_CPU:
		break;
	case GLBuffer::Type_PBO:
		glGenBuffers(1, &(returnMe->name));
		GLERRLOG
		if (!inCreateInCurrentContext)	{
			//	flush!
			glFlush();
			GLERRLOG
		}
		break;
	case GLBuffer::Type_RB:
		//	generate the renderbuffer
		glGenRenderbuffers(1, &(returnMe->name));
		GLERRLOG
		//	bind the renderbuffer, set it up
		glBindRenderbuffer(returnMe->desc.target, returnMe->name);
		GLERRLOG
#if defined(VVGL_TARGETENV_GL3PLUS) || defined(VVGL_TARGETENV_GLES3) || defined(VVGL_SDK_MAC)
		if (returnMe->desc.msAmount > 0)	{
			glRenderbufferStorageMultisample(returnMe->desc.target,
				static_cast<int>(returnMe->desc.msAmount),
				returnMe->desc.pixelFormat,
				static_cast<int>(round(s.width)),
				static_cast<int>(round(s.height)));
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
	case GLBuffer::Type_FBO:
		glGenFramebuffers(1, &(returnMe->name));
		GLERRLOG
		if (!inCreateInCurrentContext)	{
			glFlush();
			GLERRLOG
		}
		break;
	case GLBuffer::Type_Tex:
#if defined(VVGL_SDK_MAC)
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
			//	...the localSurfaceID in the descriptor gets updated later, when we call GLBuffer::setLocalSurfaceRef()
		}
#endif
		//	enable the tex target, gen the texture, and bind it
		glActiveTexture(GL_TEXTURE0);
		GLERRLOG
		if (context->version <= GLVersion_2)	{
#if defined(VVGL_TARGETENV_GL2)
			glEnable(newBufferDesc.target);
			GLERRLOG
#endif
		}
		glGenTextures(1, &(returnMe->name));
		GLERRLOG
		glBindTexture(newBufferDesc.target, returnMe->name);
		GLERRLOG
		
#if defined(VVGL_SDK_MAC)
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
		
//#if !defined(VVGL_SDK_IOS) && !defined(VVGL_SDK_RPI)
#if !defined(VVGL_TARGETENV_GLES) && !defined(VVGL_TARGETENV_GLES3)
		//	setup basic tex defaults
		glPixelStorei(GL_UNPACK_SKIP_ROWS, GL_FALSE);
		GLERRLOG
		glPixelStorei(GL_UNPACK_SKIP_PIXELS, GL_FALSE);
		GLERRLOG
		glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
		GLERRLOG
#endif
//#endif
		
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
		
//#if !defined(VVGL_SDK_IOS) && !defined(VVGL_SDK_RPI)
#if !defined(VVGL_TARGETENV_GLES) && !defined(VVGL_TARGETENV_GLES3)
		if (context!=nullptr && context->version == GLVersion_2)	{
			if (newBufferDesc.pixelFormat == GLBuffer::PF_Depth)	{
				glTexParameteri(newBufferDesc.target, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
				GLERRLOG
			}
		}
		
		if (newBufferDesc.pixelFormat == GLBuffer::PF_Depth)	{
			glTexParameteri(newBufferDesc.target, GL_TEXTURE_STORAGE_HINT_APPLE, GL_STORAGE_CACHED_APPLE);
			GLERRLOG
		}
		else	{
			glTexParameteri(newBufferDesc.target, GL_TEXTURE_STORAGE_HINT_APPLE, GL_STORAGE_SHARED_APPLE);
			GLERRLOG
		}
#endif
//#endif
		
#if defined(VVGL_SDK_MAC)
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
#if defined(VVGL_SDK_MAC)
		if (newSurfaceRef==nullptr || err!=kCGLNoError)	{
#endif
			if (compressedTex)	{
				glTexImage2D(newBufferDesc.target,
					0,
					newBufferDesc.internalFormat,
					static_cast<int>(round(s.width)),
					static_cast<int>(round(s.height)),
					0,
					newBufferDesc.pixelFormat,
					newBufferDesc.pixelType,
					nullptr);
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
					static_cast<int>(round(s.width)),
					static_cast<int>(round(s.height)),
					0,
					newBufferDesc.pixelFormat,
					newBufferDesc.pixelType,
					b);
				GLERRLOG
			}
#if defined(VVGL_SDK_MAC)
		}
#endif
		

#if defined(VVGL_SDK_MAC)
		if (newBufferDesc.texClientStorageFlag)	{
			glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_FALSE);
			GLERRLOG
		}
#endif
		
		//	un-bind the tex and disable the target
		glBindTexture(newBufferDesc.target, 0);
		GLERRLOG
		if (context->version <= GLVersion_2)	{
#if defined(VVGL_TARGETENV_GL2)
			glDisable(newBufferDesc.target);
			GLERRLOG
#endif
		}
		if (!inCreateInCurrentContext)	{
			//	flush!
			glFlush();
			GLERRLOG
		}
		
		break;
	case GLBuffer::Type_VBO:
	case GLBuffer::Type_EBO:
	case GLBuffer::Type_VAO:
		//	left intentionally blank- VBOs, EBOs, and VAOs are created in their respective factory functions
		break;
	}
	
	//CGLUnlockContext(cgl_ctx);
	
	returnMe->size = s;
	returnMe->srcRect = {0,0,s.width,s.height};
	returnMe->backingSize = bs;
	returnMe->cpuBackingPtr = const_cast<void*>(b);
	
	//	timestamp the buffer!
	timestampThisBuffer(returnMe);
	
#if defined(VVGL_SDK_MAC)
	if (newSurfaceRef != nullptr)	{
		returnMe->setLocalSurfaceRef(newSurfaceRef);
		CFRelease(newSurfaceRef);
		newSurfaceRef = nullptr;
	}
#endif
	
	return returnMe;
}


GLBufferRef GLBufferPool::fetchMatchingFreeBuffer(const GLBuffer::Descriptor & desc, const Size & size)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	if (deleted)
		return nullptr;
	
	//	get a lock on the array of free buffers
	lock_guard<mutex>		lock(freeBuffersLock);
	
	GLBufferRef			returnMe = nullptr;
	
	vector<GLBufferRef>::iterator		it;
	int						tmpIndex = 0;
	for (it=freeBuffers.begin(); it!=freeBuffers.end(); ++it)	{
		//	if this buffer is comparable to the passed descriptor and can be used for recycling...
		GLBuffer			*bufferPtr = (*it).get();
		if (bufferPtr->isComparableForRecycling(desc))	{
			//	check to make sure that the dimensions of this buffer are compatible...
			bool				sizeIsOK = false;
			GLBuffer::Descriptor & desc = bufferPtr->desc;
			//GLBuffer::Descriptor	*desc = &(bufferPtr->desc);
			switch (desc.type)	{
			case GLBuffer::Type_FBO:
				sizeIsOK = true;
				break;
			case GLBuffer::Type_CPU:
			case GLBuffer::Type_RB:
			case GLBuffer::Type_Tex:
			case GLBuffer::Type_PBO:
				if (bufferPtr->size == size)
					sizeIsOK = true;
				break;
			case GLBuffer::Type_VBO:
			case GLBuffer::Type_EBO:
			case GLBuffer::Type_VAO:
				break;
			}
			
			//	if the size of this buffer is compatible with what's requested...
			if (sizeIsOK)	{
#if defined(VVGL_SDK_MAC)
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
#if defined(VVGL_SDK_MAC)
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

void GLBufferPool::housekeeping()	{
	//cout << __PRETTY_FUNCTION__ << endl;
	//cout << "\tthis is " << this << endl;
	
	lock_guard<mutex>		lock(freeBuffersLock);
	
	bool			needsToClearStuff = false;
	for_each(freeBuffers.begin(), freeBuffers.end(), [&](const GLBufferRef & n)	{
		(*n).idleCount++;
		if ((*n).idleCount >= IDLEBUFFERCOUNT)
			needsToClearStuff = true;
	});
	
	//	if there are indices that need to be removed...
	if (needsToClearStuff)	{
		auto		removeIt = remove_if(freeBuffers.begin(), freeBuffers.end(), [&](GLBufferRef n){ return (*n).idleCount >= IDLEBUFFERCOUNT; });
		freeBuffers.erase(removeIt, freeBuffers.end());
	}
}
void GLBufferPool::purge()	{
	{
		lock_guard<mutex>		lock(freeBuffersLock);
		for_each(freeBuffers.begin(), freeBuffers.end(), [&](const GLBufferRef & n)	{
			n->idleCount = (IDLEBUFFERCOUNT+1);
		});
	}
	housekeeping();
}
ostream & operator<<(ostream & os, const GLBufferPool & n)	{
	os << "<GLBufferPool " << &n << ">";
	return os;
}

void GLBufferPool::flush()	{
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


void GLBufferPool::returnBufferToPool(GLBuffer * inBuffer)	{
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
	freeBuffers.emplace_back(make_shared<GLBuffer>(*inBuffer));
	
	//	now clear out some vars in the passed buffer- we don't want to release a backing if we're putting it back in the pool
	inBuffer->backingReleaseCallback = nullptr;
	inBuffer->backingContext = nullptr;
	
}

void GLBufferPool::releaseBufferResources(GLBuffer * inBuffer)	{
	//cout << __PRETTY_FUNCTION__ << "... " << *inBuffer << endl;
	
	if (inBuffer == nullptr)
		return;
	
	lock_guard<recursive_mutex>		lock(contextLock);
	if (context == nullptr)
		return;
	
	//	Qt has thread-specific contexts: you cannot make them current on any other threads or they crash
#if defined(VVGL_SDK_QT)
	//	if we can't make the context current on this thread
	QThread			*currentThread = QThread::currentThread();
	QObject			*qCtxAsObj = (QObject*)context->getContext();
	QThread			*ctxThread = (qCtxAsObj==nullptr) ? nullptr : qCtxAsObj->thread();
	
	if (currentThread != ctxThread)	{
		//cout << "\terr: can't release buffer " << inBuffer->getDescriptionString() << " on this thread..." << endl;
		//	make a new GLBuffer that duplicates the GLBuffer we were passed (which is being freed)
		GLBuffer		*copiedBuffer = inBuffer->allocShallowCopy();
		//	now we want this buffer to get deleted on the context's thread...
		perform_async([=](){
			//cout << "\tshould be deleting the copied buffer now..." << endl;
			delete copiedBuffer;
		}, qCtxAsObj);
		//	return- we can't make the context current or delete anything on this thread
		return;
	}
#endif	//	VVGL_SDK_QT
	
	//context->makeCurrentIfNull();
	//context->makeCurrent();
	context->makeCurrentIfNotCurrent();
	
	switch (inBuffer->desc.type)	{
	case GLBuffer::Type_CPU:
		break;
	case GLBuffer::Type_RB:
		glDeleteRenderbuffers(1, &inBuffer->name);
		GLERRLOG
		break;
	case GLBuffer::Type_FBO:
		glDeleteFramebuffers(1, &inBuffer->name);
		GLERRLOG
		break;
	case GLBuffer::Type_Tex:
		glDeleteTextures(1, &inBuffer->name);
		GLERRLOG
		break;
	case GLBuffer::Type_PBO:
//	none of this stuff should be available if we're running ES
#if !defined(VVGL_TARGETENV_GLES) && !defined(VVGL_TARGETENV_GLES3)
		if (inBuffer->pboMapped)	{
			inBuffer->unmapPBO(true);
		}
#endif	//	!defined(VVGL_TARGETENV_GLES) && !defined(VVGL_TARGETENV_GLES3)
		glDeleteBuffers(1, &inBuffer->name);
		GLERRLOG
		break;
	case GLBuffer::Type_VBO:
		glDeleteBuffers(1, &inBuffer->name);
		GLERRLOG
		break;
	case GLBuffer::Type_EBO:
		glDeleteBuffers(1, &inBuffer->name);
		GLERRLOG
		break;
	case GLBuffer::Type_VAO:
		glDeleteVertexArrays(1, &inBuffer->name);
		GLERRLOG
		break;
	}
	glFlush();
	GLERRLOG
}




/*	========================================	*/
#pragma mark *************** non-member functions ***************


/*
GLBufferPoolRef CreateGlobalBufferPool(const GLContext * inShareCtx)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	
	GLBufferPoolRef		returnMe = make_shared<GLBufferPool>(inShareCtx);
	if (_globalBufferPool != nullptr)	{
		delete _globalBufferPool;
		_globalBufferPool = nullptr;
	}
	_globalBufferPool = new shared_ptr<GLBufferPool>(returnMe);
	
	return returnMe;
}
*/
GLBufferPoolRef CreateGlobalBufferPool(const GLContextRef & inPoolCtx)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	
	GLBufferPoolRef		returnMe = make_shared<GLBufferPool>(inPoolCtx);
	if (_globalBufferPool != nullptr)	{
		delete _globalBufferPool;
		_globalBufferPool = nullptr;
	}
	_globalBufferPool = new shared_ptr<GLBufferPool>(returnMe);
	
	return returnMe;
}
/*
GLBufferPoolRef GetGlobalBufferPool()	{
	if (_globalBufferPool==nullptr)
		return nullptr;
	return *_globalBufferPool;
}
*/
const GLBufferPoolRef & GetGlobalBufferPool()	{
	//cout << __PRETTY_FUNCTION__ << endl;
	if (_globalBufferPool==nullptr)	{
		return _nullGlobalBufferPool;
	}
	return *_globalBufferPool;
}


/*	========================================	*/
#pragma mark --------------------- non-image buffer creation methods


GLBufferRef CreateFBO(const bool & inCreateInCurrentContext, const GLBufferPoolRef & inPoolRef)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	
	if (inPoolRef == nullptr)
		return nullptr;
	
	GLBuffer::Descriptor	desc;
	
	desc.type = GLBuffer::Type_FBO;
	//desc.target = GLBuffer::Target_2D;
	//desc.internalFormat = GLBuffer::IF_Depth24;
	//desc.pixelFormat = GLBuffer::PF_Depth;
	//desc.pixelType = GLBuffer::PT_UByte;
	desc.cpuBackingType = GLBuffer::Backing_None;
	desc.gpuBackingType = GLBuffer::Backing_Internal;
	//desc.texRangeFlag = false;
	//desc.texClientStorageFlag = false;
	//desc.msAmount = 0;
	//desc.localSurfaceID = 0;
	
	GLBufferRef	returnMe = inPoolRef->createBufferRef(desc, Size(), nullptr, Size(), inCreateInCurrentContext);
	returnMe->parentBufferPool = inPoolRef;
	
	return returnMe;
}
GLBufferRef CreateVBO(const void * inBytes, const size_t & inByteSize, const int32_t & inUsage, const bool & inCreateInCurrentContext, const GLBufferPoolRef & inPoolRef)	{
	if (inPoolRef == nullptr)
		return nullptr;
	GLBufferRef		returnMe = make_shared<GLBuffer>(inPoolRef);
	GLBuffer::Descriptor &	desc = returnMe->desc;
	
	desc.type = GLBuffer::Type_VBO;
	desc.target = GLBuffer::Target_None;
	desc.internalFormat = GLBuffer::IF_None;
	desc.pixelFormat = GLBuffer::PF_None;
	desc.pixelType = GLBuffer::PT_Float;
	desc.cpuBackingType = GLBuffer::Backing_None;
	desc.gpuBackingType = GLBuffer::Backing_Internal;
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
	glBufferData(GL_ARRAY_BUFFER, inByteSize, inBytes, static_cast<int>(round(inUsage)));
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
GLBufferRef CreateEBO(const void * inBytes, const size_t & inByteSize, const int32_t & inUsage, const bool & inCreateInCurrentContext, const GLBufferPoolRef & inPoolRef)	{
	if (inPoolRef == nullptr)
		return nullptr;
	GLBufferRef		returnMe = make_shared<GLBuffer>(inPoolRef);
	GLBuffer::Descriptor &	desc = returnMe->desc;
	
	desc.type = GLBuffer::Type_EBO;
	desc.target = GLBuffer::Target_None;
	desc.internalFormat = GLBuffer::IF_None;
	desc.pixelFormat = GLBuffer::PF_None;
	desc.pixelType = GLBuffer::PT_Float;
	desc.cpuBackingType = GLBuffer::Backing_None;
	desc.gpuBackingType = GLBuffer::Backing_Internal;
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
GLBufferRef CreateVAO(const bool & inCreateInCurrentContext, const GLBufferPoolRef & inPoolRef)	{
#if defined(VVGL_TARGETENV_GL3PLUS) || defined(VVGL_TARGETENV_GLES3)
	if (inPoolRef == nullptr)
		return nullptr;
	if (inPoolRef->getContext()->version < GLVersion_ES3)
		return nullptr;
	GLBufferRef		returnMe = make_shared<GLBuffer>(inPoolRef);
	GLBuffer::Descriptor &	desc = returnMe->desc;
	
	desc.type = GLBuffer::Type_VAO;
	desc.target = GLBuffer::Target_None;
	desc.internalFormat = GLBuffer::IF_None;
	desc.pixelFormat = GLBuffer::PF_None;
	desc.pixelType = GLBuffer::PT_Float;
	desc.cpuBackingType = GLBuffer::Backing_None;
	desc.gpuBackingType = GLBuffer::Backing_Internal;
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
#else
	return nullptr;
#endif
}


/*	========================================	*/
#pragma mark --------------------- renderbuffer creation methods


GLBufferRef CreateRB(const Size & size, const bool & inCreateInCurrentContext, const GLBufferPoolRef & inPoolRef)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	
	if (inPoolRef == nullptr)
		return nullptr;
	
	GLBuffer::Descriptor	desc;
	
	desc.type = GLBuffer::Type_RB;
	desc.target = GLBuffer::Target_2D;
#if !defined(VVGL_SDK_RPI)
	desc.internalFormat = GLBuffer::IF_Depth24;
#else
	desc.internalFormat = GLBuffer::IF_Depth16;
#endif
	desc.pixelFormat = GLBuffer::PF_Depth;
	desc.pixelType = GLBuffer::PT_UByte;
	desc.cpuBackingType = GLBuffer::Backing_None;
	desc.gpuBackingType = GLBuffer::Backing_Internal;
	desc.texRangeFlag = false;
	desc.texClientStorageFlag = false;
	desc.msAmount = 0;
	desc.localSurfaceID = 0;
	
	GLBufferRef	returnMe = inPoolRef->createBufferRef(desc, size, nullptr, Size(), inCreateInCurrentContext);
	returnMe->parentBufferPool = inPoolRef;
	
	return returnMe;
}


/*	========================================	*/
#pragma mark --------------------- CPU-only buffer creation methods


GLBufferRef CreateRGBACPUBuffer(const Size & size, const GLBufferPoolRef & inPoolRef)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	if (inPoolRef == nullptr)
		return nullptr;
	
	GLBuffer::Descriptor	desc;
	
	desc.type = GLBuffer::Type_CPU;
	desc.target = GLBuffer::Target_None;
//#if defined(VVGL_SDK_MAC)
//	desc.internalFormat = GLBuffer::IF_RGBA8;
//	desc.pixelType = GLBuffer::PT_UInt_8888_Rev;
//#else
	desc.internalFormat = GLBuffer::IF_RGBA;
	desc.pixelType = GLBuffer::PT_UByte;
//#endif
	desc.pixelFormat = GLBuffer::PF_RGBA;
	//desc.pixelType = GLBuffer::PT_UByte;
	desc.cpuBackingType = GLBuffer::Backing_Internal;
	desc.gpuBackingType = GLBuffer::Backing_None;
	desc.texRangeFlag = false;
	desc.texClientStorageFlag = false;
	desc.msAmount = 0;
	desc.localSurfaceID = 0;
	
	void			*bufferMemory = malloc(desc.backingLengthForSize(size));
	GLBufferRef		returnMe = inPoolRef->createBufferRef(desc, size, bufferMemory, size, false);
	returnMe->parentBufferPool = inPoolRef;
	returnMe->backingID = GLBuffer::BackingID_Pixels;
	returnMe->backingContext = bufferMemory;
	returnMe->backingReleaseCallback = [](GLBuffer & /*inBuffer*/, void* inReleaseContext)	{
		free(inReleaseContext);
	};
	
	return returnMe;
}
GLBufferRef CreateRGBAFloatCPUBuffer(const Size & size, const GLBufferPoolRef & inPoolRef)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	if (inPoolRef == nullptr)
		return nullptr;
	
	GLBuffer::Descriptor	desc;
	
	desc.type = GLBuffer::Type_CPU;
	desc.target = GLBuffer::Target_None;
	desc.internalFormat = GLBuffer::IF_RGBA32F;
	desc.pixelFormat = GLBuffer::PF_RGBA;
	desc.pixelType = GLBuffer::PT_Float;
	desc.cpuBackingType = GLBuffer::Backing_Internal;
	desc.gpuBackingType = GLBuffer::Backing_None;
	desc.texRangeFlag = false;
	desc.texClientStorageFlag = false;
	desc.msAmount = 0;
	desc.localSurfaceID = 0;
	
	void			*bufferMemory = malloc(desc.backingLengthForSize(size));
	GLBufferRef		returnMe = inPoolRef->createBufferRef(desc, size, bufferMemory, size, false);
	returnMe->parentBufferPool = inPoolRef;
	returnMe->backingID = GLBuffer::BackingID_Pixels;
	returnMe->backingContext = bufferMemory;
	returnMe->backingReleaseCallback = [](GLBuffer & /*inBuffer*/, void* inReleaseContext)	{
		free(inReleaseContext);
	};
	
	return returnMe;
}
GLBufferRef CreateBGRACPUBuffer(const Size & size, const GLBufferPoolRef & inPoolRef)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	if (inPoolRef == nullptr)
		return nullptr;
	
	GLBuffer::Descriptor	desc;
	
	desc.type = GLBuffer::Type_CPU;
	desc.target = GLBuffer::Target_None;
//#if defined(VVGL_SDK_MAC)
//	desc.internalFormat = GLBuffer::IF_RGBA8;
//	desc.pixelType = GLBuffer::PT_UInt_8888_Rev;
//#else
	desc.internalFormat = GLBuffer::IF_RGBA;
	desc.pixelType = GLBuffer::PT_UByte;
//#endif
	desc.pixelFormat = GLBuffer::PF_BGRA;
	//desc.pixelType = GLBuffer::PT_UByte;
	desc.cpuBackingType = GLBuffer::Backing_Internal;
	desc.gpuBackingType = GLBuffer::Backing_None;
	desc.texRangeFlag = false;
	desc.texClientStorageFlag = false;
	desc.msAmount = 0;
	desc.localSurfaceID = 0;
	
	void			*bufferMemory = malloc(desc.backingLengthForSize(size));
	GLBufferRef		returnMe = inPoolRef->createBufferRef(desc, size, bufferMemory, size, false);
	returnMe->parentBufferPool = inPoolRef;
	returnMe->backingID = GLBuffer::BackingID_Pixels;
	returnMe->backingContext = bufferMemory;
	returnMe->backingReleaseCallback = [](GLBuffer & /*inBuffer*/, void* inReleaseContext)	{
		free(inReleaseContext);
	};
	
	return returnMe;
}
GLBufferRef CreateBGRAFloatCPUBuffer(const Size & size, const GLBufferPoolRef & inPoolRef)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	if (inPoolRef == nullptr)
		return nullptr;
	
	GLBuffer::Descriptor	desc;
	
	desc.type = GLBuffer::Type_CPU;
	desc.target = GLBuffer::Target_None;
	desc.internalFormat = GLBuffer::IF_RGBA32F;
	desc.pixelFormat = GLBuffer::PF_BGRA;
	desc.pixelType = GLBuffer::PT_Float;
	desc.cpuBackingType = GLBuffer::Backing_Internal;
	desc.gpuBackingType = GLBuffer::Backing_None;
	desc.texRangeFlag = false;
	desc.texClientStorageFlag = false;
	desc.msAmount = 0;
	desc.localSurfaceID = 0;
	
	void			*bufferMemory = malloc(desc.backingLengthForSize(size));
	GLBufferRef		returnMe = inPoolRef->createBufferRef(desc, size, bufferMemory, size, false);
	returnMe->parentBufferPool = inPoolRef;
	returnMe->backingID = GLBuffer::BackingID_Pixels;
	returnMe->backingContext = bufferMemory;
	returnMe->backingReleaseCallback = [](GLBuffer & /*inBuffer*/, void* inReleaseContext)	{
		free(inReleaseContext);
	};
	
	return returnMe;
}
GLBufferRef CreateRGBACPUBufferUsing(const Size & inCPUBufferSizeInPixels, const void * inCPUBackingPtr, const Size & inImageSizeInPixels, const void * inReleaseCallbackContext, const GLBuffer::BackingReleaseCallback & inReleaseCallback, const GLBufferPoolRef & inPoolRef)	{
	GLBufferRef		returnMe = make_shared<GLBuffer>(inPoolRef);
	GLBuffer::Descriptor		&desc = returnMe->desc;
	
	desc.type = GLBuffer::Type_CPU;
	desc.target = GLBuffer::Target_None;
	desc.internalFormat = GLBuffer::IF_RGBA;
	desc.pixelFormat = GLBuffer::PF_RGBA;
	desc.pixelType = GLBuffer::PT_UByte;
	desc.cpuBackingType = GLBuffer::Backing_External;
	desc.gpuBackingType = GLBuffer::Backing_None;
	desc.texRangeFlag = false;
	desc.texClientStorageFlag = false;
	desc.msAmount = 0;
	desc.localSurfaceID = 0;
	
	returnMe->name = 0;
	returnMe->preferDeletion = true;
	returnMe->size = inCPUBufferSizeInPixels;
	returnMe->srcRect = { 0, 0, inImageSizeInPixels.width, inImageSizeInPixels.height };
	returnMe->flipped = false;
	returnMe->backingSize = inCPUBufferSizeInPixels;
	//returnMe->contentTimestamp = XXX;
	
	inPoolRef->timestampThisBuffer(returnMe);
	
	
	returnMe->backingReleaseCallback = inReleaseCallback;
	returnMe->backingContext = const_cast<void*>(inReleaseCallbackContext);
	returnMe->backingID = GLBuffer::BackingID_GenericExternalCPU;
	returnMe->cpuBackingPtr = const_cast<void*>(inCPUBackingPtr);
	
	return returnMe;
}
GLBufferRef CreateRGBAFloatCPUBufferUsing(const Size & inCPUBufferSizeInPixels, const void * inCPUBackingPtr, const Size & inImageSizeInPixels, const void * inReleaseCallbackContext, const GLBuffer::BackingReleaseCallback & inReleaseCallback, const GLBufferPoolRef & inPoolRef)	{
	GLBufferRef		returnMe = make_shared<GLBuffer>(inPoolRef);
	GLBuffer::Descriptor		&desc = returnMe->desc;
	
	desc.type = GLBuffer::Type_CPU;
	desc.target = GLBuffer::Target_None;
	desc.internalFormat = GLBuffer::IF_RGBA;
	desc.pixelFormat = GLBuffer::PF_RGBA;
	desc.pixelType = GLBuffer::PT_Float;
	desc.cpuBackingType = GLBuffer::Backing_External;
	desc.gpuBackingType = GLBuffer::Backing_None;
	desc.texRangeFlag = false;
	desc.texClientStorageFlag = false;
	desc.msAmount = 0;
	desc.localSurfaceID = 0;
	
	returnMe->name = 0;
	returnMe->preferDeletion = true;
	returnMe->size = inCPUBufferSizeInPixels;
	returnMe->srcRect = { 0, 0, inImageSizeInPixels.width, inImageSizeInPixels.height };
	returnMe->flipped = false;
	returnMe->backingSize = inCPUBufferSizeInPixels;
	//returnMe->contentTimestamp = XXX;
	
	inPoolRef->timestampThisBuffer(returnMe);
	
	
	returnMe->backingReleaseCallback = inReleaseCallback;
	returnMe->backingContext = const_cast<void*>(inReleaseCallbackContext);
	returnMe->backingID = GLBuffer::BackingID_GenericExternalCPU;
	returnMe->cpuBackingPtr = const_cast<void*>(inCPUBackingPtr);
	
	return returnMe;
}
GLBufferRef CreateBGRACPUBufferUsing(const Size & inCPUBufferSizeInPixels, const void * inCPUBackingPtr, const Size & inImageSizeInPixels, const void * inReleaseCallbackContext, const GLBuffer::BackingReleaseCallback & inReleaseCallback, const GLBufferPoolRef & inPoolRef)	{
	GLBufferRef		returnMe = make_shared<GLBuffer>(inPoolRef);
	GLBuffer::Descriptor		&desc = returnMe->desc;
	
	desc.type = GLBuffer::Type_CPU;
	desc.target = GLBuffer::Target_None;
	desc.internalFormat = GLBuffer::IF_RGBA;
	desc.pixelFormat = GLBuffer::PF_BGRA;
	desc.pixelType = GLBuffer::PT_UByte;
	desc.cpuBackingType = GLBuffer::Backing_External;
	desc.gpuBackingType = GLBuffer::Backing_None;
	desc.texRangeFlag = false;
	desc.texClientStorageFlag = false;
	desc.msAmount = 0;
	desc.localSurfaceID = 0;
	
	returnMe->name = 0;
	returnMe->preferDeletion = true;
	returnMe->size = inCPUBufferSizeInPixels;
	returnMe->srcRect = { 0, 0, inImageSizeInPixels.width, inImageSizeInPixels.height };
	returnMe->flipped = false;
	returnMe->backingSize = inCPUBufferSizeInPixels;
	//returnMe->contentTimestamp = XXX;
	
	inPoolRef->timestampThisBuffer(returnMe);
	
	
	returnMe->backingReleaseCallback = inReleaseCallback;
	returnMe->backingContext = const_cast<void*>(inReleaseCallbackContext);
	returnMe->backingID = GLBuffer::BackingID_GenericExternalCPU;
	returnMe->cpuBackingPtr = const_cast<void*>(inCPUBackingPtr);
	
	return returnMe;
}
GLBufferRef CreateBGRAFloatCPUBufferUsing(const Size & inCPUBufferSizeInPixels, const void * inCPUBackingPtr, const Size & inImageSizeInPixels, const void * inReleaseCallbackContext, const GLBuffer::BackingReleaseCallback & inReleaseCallback, const GLBufferPoolRef & inPoolRef)	{
	GLBufferRef		returnMe = make_shared<GLBuffer>(inPoolRef);
	GLBuffer::Descriptor		&desc = returnMe->desc;
	
	desc.type = GLBuffer::Type_CPU;
	desc.target = GLBuffer::Target_None;
	desc.internalFormat = GLBuffer::IF_RGBA;
	desc.pixelFormat = GLBuffer::PF_BGRA;
	desc.pixelType = GLBuffer::PT_Float;
	desc.cpuBackingType = GLBuffer::Backing_External;
	desc.gpuBackingType = GLBuffer::Backing_None;
	desc.texRangeFlag = false;
	desc.texClientStorageFlag = false;
	desc.msAmount = 0;
	desc.localSurfaceID = 0;
	
	returnMe->name = 0;
	returnMe->preferDeletion = true;
	returnMe->size = inCPUBufferSizeInPixels;
	returnMe->srcRect = { 0, 0, inImageSizeInPixels.width, inImageSizeInPixels.height };
	returnMe->flipped = false;
	returnMe->backingSize = inCPUBufferSizeInPixels;
	//returnMe->contentTimestamp = XXX;
	
	inPoolRef->timestampThisBuffer(returnMe);
	
	
	returnMe->backingReleaseCallback = inReleaseCallback;
	returnMe->backingContext = const_cast<void*>(inReleaseCallbackContext);
	returnMe->backingID = GLBuffer::BackingID_GenericExternalCPU;
	returnMe->cpuBackingPtr = const_cast<void*>(inCPUBackingPtr);
	
	return returnMe;
}
//	none of this stuff should be available if we're running ES
#if !defined(VVGL_TARGETENV_GLES) && !defined(VVGL_TARGETENV_GLES3)
VVGL_EXPORT GLBufferRef CreateYCbCrCPUBufferUsing(const Size & inCPUBufferSizeInPixels, const void * inCPUBackingPtr, const Size & inImageSizeInPixels, const void * inReleaseCallbackContext, const GLBuffer::BackingReleaseCallback & inReleaseCallback, const GLBufferPoolRef & inPoolRef)	{
	GLBufferRef		returnMe = make_shared<GLBuffer>(inPoolRef);
	GLBuffer::Descriptor		&desc = returnMe->desc;
	
	desc.type = GLBuffer::Type_CPU;
	desc.target = GLBuffer::Target_None;
	desc.internalFormat = GLBuffer::IF_RGB;
	desc.pixelFormat = GLBuffer::PF_YCbCr_422;
	desc.pixelType = GLBuffer::PT_UShort88;
	desc.cpuBackingType = GLBuffer::Backing_External;
	desc.gpuBackingType = GLBuffer::Backing_None;
	desc.texRangeFlag = false;
	desc.texClientStorageFlag = false;
	desc.msAmount = 0;
	desc.localSurfaceID = 0;
	
	returnMe->name = 0;
	returnMe->preferDeletion = true;
	returnMe->size = inCPUBufferSizeInPixels;
	returnMe->srcRect = { 0, 0, inImageSizeInPixels.width, inImageSizeInPixels.height };
	returnMe->flipped = false;
	returnMe->backingSize = inCPUBufferSizeInPixels;
	//returnMe->contentTimestamp = XXX;
	
	inPoolRef->timestampThisBuffer(returnMe);
	
	
	returnMe->backingReleaseCallback = inReleaseCallback;
	returnMe->backingContext = const_cast<void*>(inReleaseCallbackContext);
	returnMe->backingID = GLBuffer::BackingID_GenericExternalCPU;
	returnMe->cpuBackingPtr = const_cast<void*>(inCPUBackingPtr);
	
	return returnMe;
}
#endif	//	!defined(VVGL_TARGETENV_GLES) && !defined(VVGL_TARGETENV_GLES3)


/*	========================================	*/
#pragma mark --------------------- texture buffer creation methods


GLBufferRef CreateRGBATex(const Size & size, const bool & inCreateInCurrentContext, const GLBufferPoolRef & inPoolRef)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	if (inPoolRef == nullptr)
		return nullptr;
	
	GLBuffer::Descriptor	desc;
	
	desc.type = GLBuffer::Type_Tex;
	desc.target = GLBuffer::Target_2D;
#if defined(VVGL_SDK_MAC)
	desc.internalFormat = GLBuffer::IF_RGBA8;
	desc.pixelType = GLBuffer::PT_UInt_8888_Rev;
#else
	desc.internalFormat = GLBuffer::IF_RGBA;
	desc.pixelType = GLBuffer::PT_UByte;
#endif
	desc.pixelFormat = GLBuffer::PF_RGBA;
	//desc.pixelType = GLBuffer::PT_UByte;
	desc.cpuBackingType = GLBuffer::Backing_None;
	desc.gpuBackingType = GLBuffer::Backing_Internal;
	desc.texRangeFlag = false;
	desc.texClientStorageFlag = false;
	desc.msAmount = 0;
	desc.localSurfaceID = 0;
	
	GLBufferRef	returnMe = inPoolRef->createBufferRef(desc, size, nullptr, Size(), inCreateInCurrentContext);
	returnMe->parentBufferPool = inPoolRef;
	
	return returnMe;
}
GLBufferRef CreateRGBAFloatTex(const Size & size, const bool & inCreateInCurrentContext, const GLBufferPoolRef & inPoolRef)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	if (inPoolRef == nullptr)
		return nullptr;
	
	GLBuffer::Descriptor	desc;
	
	desc.type = GLBuffer::Type_Tex;
	desc.target = GLBuffer::Target_2D;
#if !defined(VVGL_SDK_RPI)
	desc.internalFormat = GLBuffer::IF_RGBA32F;
	desc.pixelFormat = GLBuffer::PF_RGBA;
	desc.pixelType = GLBuffer::PT_Float;
#else
	desc.internalFormat = GLBuffer::IF_RGBA;
	desc.pixelFormat = GLBuffer::PF_RGBA;
	//desc.pixelType = GLBuffer::PT_HalfFloat;
	desc.pixelType = GLBuffer::PT_UByte;
#endif
	//desc.pixelFormat = GLBuffer::PF_RGBA;
	//desc.pixelType = GLBuffer::PT_Float;
	desc.cpuBackingType = GLBuffer::Backing_None;
	desc.gpuBackingType = GLBuffer::Backing_Internal;
	desc.texRangeFlag = false;
	desc.texClientStorageFlag = false;
	desc.msAmount = 0;
	desc.localSurfaceID = 0;
	
	GLBufferRef	returnMe = inPoolRef->createBufferRef(desc, size, nullptr, Size(), inCreateInCurrentContext);
	returnMe->parentBufferPool = inPoolRef;
	
	return returnMe;
}
#if !defined(VVGL_TARGETENV_GLES) && !defined(VVGL_TARGETENV_GLES3)
GLBufferRef CreateYCbCrTex(const Size & size, const bool & createInCurrentContext, const GLBufferPoolRef & inPoolRef)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	if (inPoolRef == nullptr)
		return nullptr;
	
	GLBuffer::Descriptor	desc;
	
	desc.type = GLBuffer::Type_Tex;
	desc.target = GLBuffer::Target_2D;
	desc.internalFormat = GLBuffer::IF_RGB;
	desc.pixelFormat = GLBuffer::PF_YCbCr_422;
	desc.pixelType = GLBuffer::PT_UShort88;
	desc.cpuBackingType = GLBuffer::Backing_None;
	desc.gpuBackingType = GLBuffer::Backing_Internal;
	desc.texRangeFlag = false;
	desc.texClientStorageFlag = false;
	desc.msAmount = 0;
	desc.localSurfaceID = 0;
	
	GLBufferRef	returnMe = inPoolRef->createBufferRef(desc, size, nullptr, Size(), createInCurrentContext);
	returnMe->parentBufferPool = inPoolRef;
	
	return returnMe;
}
#endif	//	!defined(VVGL_TARGETENV_GLES) && !defined(VVGL_TARGETENV_GLES3)
GLBufferRef CreateDepthBuffer(const Size & size, const bool & inCreateInCurrentContext, const GLBufferPoolRef & inPoolRef)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	
	if (inPoolRef == nullptr)
		return nullptr;
	
	GLBuffer::Descriptor	desc;
	
	desc.type = GLBuffer::Type_Tex;
	desc.target = GLBuffer::Target_2D;
#if !defined(VVGL_SDK_RPI)
	desc.internalFormat = GLBuffer::IF_Depth24;
#else
	desc.internalFormat = GLBuffer::IF_Depth16;
#endif
	desc.pixelFormat = GLBuffer::PF_Depth;
	desc.pixelType = GLBuffer::PT_UByte;
	desc.cpuBackingType = GLBuffer::Backing_None;
	desc.gpuBackingType = GLBuffer::Backing_Internal;
	//desc.texRangeFlag = false;
	//desc.texClientStorageFlag = false;
	//desc.msAmount = 0;
	//desc.localSurfaceID = 0;
	
	GLBufferRef	returnMe = inPoolRef->createBufferRef(desc, size, nullptr, Size(), inCreateInCurrentContext);
	returnMe->parentBufferPool = inPoolRef;
	
	return returnMe;
}

GLBufferRef CreateFromExistingGLTexture(const int32_t & inTexName, const GLBuffer::Target & inTexTarget, const GLBuffer::InternalFormat & inTexIntFmt, const GLBuffer::PixelFormat & inTexPxlFmt, const GLBuffer::PixelType & inTexPxlType, const Size & inTexSize, const bool & inTexFlipped, const Rect & inImgRectInTex, const void * inReleaseCallbackContext, const GLBuffer::BackingReleaseCallback & inReleaseCallback, const GLBufferPoolRef & inPoolRef)	{
	GLBufferRef		returnMe = make_shared<GLBuffer>();
	returnMe->desc.type = GLBuffer::Type_Tex;
	returnMe->desc.target = static_cast<GLBuffer::Target>(inTexTarget);
	returnMe->desc.internalFormat = static_cast<GLBuffer::InternalFormat>(inTexIntFmt);
	returnMe->desc.pixelFormat = static_cast<GLBuffer::PixelFormat>(inTexPxlFmt);
	returnMe->desc.pixelType = static_cast<GLBuffer::PixelType>(inTexPxlType);
	returnMe->desc.cpuBackingType = GLBuffer::Backing_External;
	returnMe->desc.gpuBackingType = GLBuffer::Backing_External;
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


#if !defined(VVGL_SDK_IOS)
GLBufferRef CreateBGRATex(const Size & size, const bool & inCreateInCurrentContext, const GLBufferPoolRef & inPoolRef)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	if (inPoolRef == nullptr)
		return nullptr;
	
	GLBuffer::Descriptor	desc;
	
	desc.type = GLBuffer::Type_Tex;
	desc.target = GLBuffer::Target_2D;
#if defined(VVGL_SDK_MAC)
	desc.internalFormat = GLBuffer::IF_RGBA8;
#else
	desc.internalFormat = GLBuffer::IF_RGBA;
#endif
	desc.pixelFormat = GLBuffer::PF_BGRA;
	desc.pixelType = GLBuffer::PT_UByte;
	desc.cpuBackingType = GLBuffer::Backing_None;
	desc.gpuBackingType = GLBuffer::Backing_Internal;
	desc.texRangeFlag = false;
	desc.texClientStorageFlag = false;
	desc.msAmount = 0;
	desc.localSurfaceID = 0;
	
	GLBufferRef	returnMe = inPoolRef->createBufferRef(desc, size, nullptr, Size(), inCreateInCurrentContext);
	returnMe->parentBufferPool = inPoolRef;
	
	return returnMe;
}
GLBufferRef CreateBGRAFloatTex(const Size & size, const bool & inCreateInCurrentContext, const GLBufferPoolRef & inPoolRef)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	if (inPoolRef == nullptr)
		return nullptr;
	
	GLBuffer::Descriptor	desc;
	
	desc.type = GLBuffer::Type_Tex;
	desc.target = GLBuffer::Target_2D;
#if !defined(VVGL_SDK_RPI)
	desc.internalFormat = GLBuffer::IF_RGBA32F;
	desc.pixelFormat = GLBuffer::PF_BGRA;
	desc.pixelType = GLBuffer::PT_Float;
#else
	desc.internalFormat = GLBuffer::IF_RGBA;
	desc.pixelFormat = GLBuffer::PF_BGRA;
	//desc.pixelType = GLBuffer::PT_HalfFloat;
	desc.pixelType = GLBuffer::PT_UByte;
#endif
	//desc.pixelFormat = GLBuffer::PF_RGBA;
	//desc.pixelType = GLBuffer::PT_Float;
	desc.cpuBackingType = GLBuffer::Backing_None;
	desc.gpuBackingType = GLBuffer::Backing_Internal;
	desc.texRangeFlag = false;
	desc.texClientStorageFlag = false;
	desc.msAmount = 0;
	desc.localSurfaceID = 0;
	
	GLBufferRef	returnMe = inPoolRef->createBufferRef(desc, size, nullptr, Size(), inCreateInCurrentContext);
	returnMe->parentBufferPool = inPoolRef;
	
	return returnMe;
}


/*	========================================	*/
#pragma mark --------------------- CPU-backed texture buffer creation methods


#if defined(VVGL_SDK_MAC)
GLBufferRef CreateRGBACPUBackedTex(const Size & size, const bool & createInCurrentContext, const GLBufferPoolRef & inPoolRef)	{
	if (inPoolRef == nullptr)
		return nullptr;
	
	GLBuffer::Descriptor		desc;
	desc.type = GLBuffer::Type_Tex;
	desc.target = GLBuffer::Target_2D;
//#if defined(VVGL_SDK_MAC)
	desc.internalFormat = GLBuffer::IF_RGBA8;
	desc.pixelType = GLBuffer::PT_UInt_8888_Rev;
//#else
//	desc.internalFormat = GLBuffer::IF_RGBA;
//	desc.pixelType = GLBuffer::PT_UByte;
//#endif
	desc.pixelFormat = GLBuffer::PF_RGBA;
	//desc.pixelType = GLBuffer::PT_UByte;
	desc.cpuBackingType = GLBuffer::Backing_Internal;
	desc.gpuBackingType = GLBuffer::Backing_Internal;
	desc.texRangeFlag = true;
	desc.texClientStorageFlag = true;
	desc.msAmount = 0;
	desc.localSurfaceID = 0;
	
	GLBufferRef	returnMe = inPoolRef->fetchMatchingFreeBuffer(desc, size);
	if (returnMe != nullptr)
		return returnMe;
	
	void			*bufferMemory = malloc(desc.backingLengthForSize(size));
	returnMe = inPoolRef->createBufferRef(desc, size, bufferMemory, size, createInCurrentContext);
	returnMe->parentBufferPool = inPoolRef;
	returnMe->backingID = GLBuffer::BackingID_Pixels;
	returnMe->backingContext = bufferMemory;
	returnMe->backingReleaseCallback = [](GLBuffer & /*inBuffer*/, void* inReleaseContext)	{
		free(inReleaseContext);
	};
	
	return returnMe;
}
GLBufferRef CreateRGBAFloatCPUBackedTex(const Size & size, const bool & createInCurrentContext, const GLBufferPoolRef & inPoolRef)	{
	if (inPoolRef == nullptr)
		return nullptr;
	
	GLBuffer::Descriptor		desc;
	desc.type = GLBuffer::Type_Tex;
	desc.target = GLBuffer::Target_2D;
	desc.internalFormat = GLBuffer::IF_RGBA32F;
	desc.pixelFormat = GLBuffer::PF_RGBA;
	desc.pixelType = GLBuffer::PT_Float;
	desc.cpuBackingType = GLBuffer::Backing_Internal;
	desc.gpuBackingType = GLBuffer::Backing_Internal;
	desc.texRangeFlag = true;
	desc.texClientStorageFlag = true;
	desc.msAmount = 0;
	desc.localSurfaceID = 0;
	
	GLBufferRef	returnMe = inPoolRef->fetchMatchingFreeBuffer(desc, size);
	if (returnMe != nullptr)
		return returnMe;
	
	void			*bufferMemory = malloc(desc.backingLengthForSize(size));
	returnMe = inPoolRef->createBufferRef(desc, size, bufferMemory, size, createInCurrentContext);
	returnMe->parentBufferPool = inPoolRef;
	returnMe->backingID = GLBuffer::BackingID_Pixels;
	returnMe->backingContext = bufferMemory;
	returnMe->backingReleaseCallback = [](GLBuffer & /*inBuffer*/, void* inReleaseContext)	{
		free(inReleaseContext);
	};
	
	return returnMe;
}
GLBufferRef CreateBGRACPUBackedTex(const Size & size, const bool & createInCurrentContext, const GLBufferPoolRef & inPoolRef)	{
	if (inPoolRef == nullptr)
		return nullptr;
	
	GLBuffer::Descriptor		desc;
	desc.type = GLBuffer::Type_Tex;
	desc.target = GLBuffer::Target_2D;
//#if defined(VVGL_SDK_MAC)
	desc.internalFormat = GLBuffer::IF_RGBA8;
	desc.pixelType = GLBuffer::PT_UInt_8888_Rev;
//#else
//	desc.internalFormat = GLBuffer::IF_RGBA;
//	desc.pixelType = GLBuffer::PT_UByte;
//#endif
	desc.pixelFormat = GLBuffer::PF_BGRA;
	//desc.pixelType = GLBuffer::PT_UByte;
	desc.cpuBackingType = GLBuffer::Backing_Internal;
	desc.gpuBackingType = GLBuffer::Backing_Internal;
	desc.texRangeFlag = true;
	desc.texClientStorageFlag = true;
	desc.msAmount = 0;
	desc.localSurfaceID = 0;
	
	GLBufferRef	returnMe = inPoolRef->fetchMatchingFreeBuffer(desc, size);
	if (returnMe != nullptr)
		return returnMe;
	
	void			*bufferMemory = malloc(desc.backingLengthForSize(size));
	returnMe = inPoolRef->createBufferRef(desc, size, bufferMemory, size, createInCurrentContext);
	returnMe->parentBufferPool = inPoolRef;
	returnMe->backingID = GLBuffer::BackingID_Pixels;
	returnMe->backingContext = bufferMemory;
	returnMe->backingReleaseCallback = [](GLBuffer & /*inBuffer*/, void* inReleaseContext)	{
		free(inReleaseContext);
	};
	
	return returnMe;
}
GLBufferRef CreateBGRAFloatCPUBackedTex(const Size & size, const bool & createInCurrentContext, const GLBufferPoolRef & inPoolRef)	{
	if (inPoolRef == nullptr)
		return nullptr;
	
	GLBuffer::Descriptor		desc;
	desc.type = GLBuffer::Type_Tex;
	desc.target = GLBuffer::Target_2D;
	desc.internalFormat = GLBuffer::IF_RGBA32F;
	desc.pixelFormat = GLBuffer::PF_BGRA;
	desc.pixelType = GLBuffer::PT_Float;
	desc.cpuBackingType = GLBuffer::Backing_Internal;
	desc.gpuBackingType = GLBuffer::Backing_Internal;
	desc.texRangeFlag = true;
	desc.texClientStorageFlag = true;
	desc.msAmount = 0;
	desc.localSurfaceID = 0;
	
	GLBufferRef	returnMe = inPoolRef->fetchMatchingFreeBuffer(desc, size);
	if (returnMe != nullptr)
		return returnMe;
	
	void			*bufferMemory = malloc(desc.backingLengthForSize(size));
	returnMe = inPoolRef->createBufferRef(desc, size, bufferMemory, size, createInCurrentContext);
	returnMe->parentBufferPool = inPoolRef;
	returnMe->backingID = GLBuffer::BackingID_Pixels;
	returnMe->backingContext = bufferMemory;
	returnMe->backingReleaseCallback = [](GLBuffer & /*inBuffer*/, void* inReleaseContext)	{
		free(inReleaseContext);
	};
	
	return returnMe;

}
#endif	//	VVGL_SDK_MAC
#endif	//	!VVGL_SDK_IOS




/*	========================================	*/
#pragma mark --------------------- image file buffer creation methods




#if defined(VVGL_SDK_MAC) || defined(VVGL_SDK_IOS)
//	these functions are defined in the VVGLBufferPool_CocoaAdditions source file
#else
GLBufferRef CreateTexFromImage(const string & /*inPath*/, const bool & /*inCreateInCurrentContext*/, const GLBufferPoolRef & /*inPoolRef*/)	{
	return nullptr;
}
GLBufferRef CreateCubeTexFromImagePaths(const vector<string> & /*inPaths*/, const bool & /*inCreateInCurrentContext*/, const GLBufferPoolRef & /*inPoolRef*/)	{
	return nullptr;
}
#endif	//	#if !VVGL_SDK_MAC && !VVGL_SDK_IOS




/*	========================================	*/
#pragma mark --------------------- PBO funcs




//	none of this stuff should be available if we're running ES
#if !defined(VVGL_TARGETENV_GLES) && !defined(VVGL_TARGETENV_GLES3)

/*
"pack" means this PBO will be used to transfer pixel data TO a PBO (glReadPixels(), glGetTexImage())
"unpack" means this PBO will be used to transfer pixel data FROM a PBO (glDrawPixels(), glTexImage2D(), glTexSubImage2D())

			decoding "GL_STREAM_DRAW, GL_STREAM_READ, etc:
STREAM		write once, read at most a few times
STATIC		write once, read many times
DYNAMIC		write many times, read many times
--------	--------	--------
DRAW		CPU -> GL
READ		GL -> CPU
COPY		GL -> GL
*/
GLBufferRef CreateRGBAPBO(const GLBuffer::Target & inTarget, const int32_t & inUsage, const Size & inSize, const void * inData, const bool & inCreateInCurrentContext, const GLBufferPoolRef & inPoolRef)	{
	if (inPoolRef == nullptr)
		return nullptr;
	
	GLBuffer::Descriptor	desc;
	
	desc.type = GLBuffer::Type_PBO;
	desc.target = inTarget;
#if defined(VVGL_SDK_MAC)
	desc.internalFormat = GLBuffer::IF_RGBA8;
	desc.pixelType = GLBuffer::PT_UInt_8888_Rev;
#else
	desc.internalFormat = GLBuffer::IF_RGBA;
	desc.pixelType = GLBuffer::PT_UByte;
#endif
	desc.pixelFormat = GLBuffer::PF_RGBA;
	desc.cpuBackingType = GLBuffer::Backing_None;
	desc.gpuBackingType = GLBuffer::Backing_Internal;
	desc.texRangeFlag = false;
	desc.texClientStorageFlag = false;
	desc.msAmount = 0;
	desc.localSurfaceID = 0;
	
	size_t			pboSizeInBytes = desc.backingLengthForSize(inSize);
	//	first try to find a PBO that matches the provided data
	GLBufferRef		returnMe = inPoolRef->fetchMatchingFreeBuffer(desc, inSize);
	//	if we found a PBO, we're recycling it: we need to discard-initialize it, and then fill it with a non-reserving call
	if (returnMe != nullptr)	{
		returnMe->parentBufferPool = inPoolRef;
		returnMe->backingSize = inSize;
		//	...if we're recycling then we may not have a current context on this thread right now
		if (!inCreateInCurrentContext)	{
			GLContextRef		tmpCtx = inPoolRef->getContext();
			if (tmpCtx == nullptr)
				return nullptr;
			//context->makeCurrentIfNull();
			//context->makeCurrent();
			tmpCtx->makeCurrentIfNotCurrent();
		}
		//	bind the PBO
		glBindBuffer(returnMe->desc.target, returnMe->name);
		GLERRLOG
		//	if the pbo is currently mapped, unmap it
		if (returnMe->pboMapped)	{
			glUnmapBuffer(inTarget);
			GLERRLOG
			returnMe->pboMapped = false;
			returnMe->cpuBackingPtr = nullptr;
		}
		glBufferData(inTarget, pboSizeInBytes, nullptr, inUsage);
		GLERRLOG
		if (inData != nullptr)	{
			glBufferSubData(inTarget, 0, pboSizeInBytes, inData);
			GLERRLOG
		}
		//	un-bind the PBO
		glBindBuffer(returnMe->desc.target, 0);
		GLERRLOG
	}
	//	else we didn't find a matching PBO
	else	{
		//	ask the pool to create a new PBO
		returnMe = inPoolRef->createBufferRef(desc, inSize, nullptr, inSize, inCreateInCurrentContext);
		if (returnMe == nullptr)
			return nullptr;
		returnMe->parentBufferPool = inPoolRef;
		returnMe->backingSize = inSize;
		//	...logically, we shouldn't have to check GL contexts- we just created a PBO, so either the buffer pool context is current or the originally-current context is still current.
		//	bind the PBO
		glBindBuffer(returnMe->desc.target, returnMe->name);
		GLERRLOG
		//	reserve-initialize the PBO the pool just created with the data we were passed
		glBufferData(inTarget, pboSizeInBytes, nullptr, inUsage);
		GLERRLOG
		if (inData != nullptr)	{
			glBufferSubData(inTarget, 0, pboSizeInBytes, inData);
			GLERRLOG
		}
		//glBufferData(inTarget, pboSizeInBytes, inData, inUsage);
		//GLERRLOG
		//	un-bind the PBO
		glBindBuffer(returnMe->desc.target, 0);
		GLERRLOG
	}
	
	return returnMe;
}
GLBufferRef CreateBGRAPBO(const int32_t & inTarget, const int32_t & inUsage, const Size & inSize, const void * inData, const bool & inCreateInCurrentContext, const GLBufferPoolRef & inPoolRef)	{
	if (inPoolRef == nullptr)
		return nullptr;
	
	GLBuffer::Descriptor	desc;
	
	desc.type = GLBuffer::Type_PBO;
	desc.target = (VVGL::GLBuffer::Target)inTarget;
#if defined(VVGL_SDK_MAC)
	desc.internalFormat = GLBuffer::IF_RGBA8;
	desc.pixelType = GLBuffer::PT_UInt_8888_Rev;
#else
	desc.internalFormat = GLBuffer::IF_RGBA;
	desc.pixelType = GLBuffer::PT_UByte;
#endif
	desc.pixelFormat = GLBuffer::PF_BGRA;
	desc.cpuBackingType = GLBuffer::Backing_None;
	desc.gpuBackingType = GLBuffer::Backing_Internal;
	desc.texRangeFlag = false;
	desc.texClientStorageFlag = false;
	desc.msAmount = 0;
	desc.localSurfaceID = 0;
	
	size_t			pboSizeInBytes = desc.backingLengthForSize(inSize);
	//	first try to find a PBO that matches the provided data
	GLBufferRef		returnMe = inPoolRef->fetchMatchingFreeBuffer(desc, inSize);
	//	if we found a PBO, we're recycling it: we need to discard-initialize it, and then fill it with a non-reserving call
	if (returnMe != nullptr)	{
		returnMe->parentBufferPool = inPoolRef;
		returnMe->backingSize = inSize;
		//	...if we're recycling then we may not have a current context on this thread right now
		if (!inCreateInCurrentContext)	{
			GLContextRef		tmpCtx = inPoolRef->getContext();
			if (tmpCtx == nullptr)
				return nullptr;
			//context->makeCurrentIfNull();
			//context->makeCurrent();
			tmpCtx->makeCurrentIfNotCurrent();
		}
		//	bind the PBO
		glBindBuffer(returnMe->desc.target, returnMe->name);
		GLERRLOG
		//	if the pbo is currently mapped, unmap it
		if (returnMe->pboMapped)	{
			glUnmapBuffer(inTarget);
			GLERRLOG
			returnMe->pboMapped = false;
			returnMe->cpuBackingPtr = nullptr;
		}
		glBufferData(inTarget, pboSizeInBytes, nullptr, inUsage);
		GLERRLOG
		if (inData != nullptr)	{
			glBufferSubData(inTarget, 0, pboSizeInBytes, inData);
			GLERRLOG
		}
		//	un-bind the PBO
		glBindBuffer(returnMe->desc.target, 0);
		GLERRLOG
	}
	//	else we didn't find a matching PBO
	else	{
		//	ask the pool to create a new PBO
		returnMe = inPoolRef->createBufferRef(desc, inSize, nullptr, inSize, inCreateInCurrentContext);
		if (returnMe == nullptr)
			return nullptr;
		returnMe->parentBufferPool = inPoolRef;
		returnMe->backingSize = inSize;
		//	...logically, we shouldn't have to check GL contexts- we just created a PBO, so either the buffer pool context is current or the originally-current context is still current.
		//	bind the PBO
		glBindBuffer(returnMe->desc.target, returnMe->name);
		GLERRLOG
		//	reserve-initialize the PBO the pool just created with the data we were passed
		glBufferData(inTarget, pboSizeInBytes, nullptr, inUsage);
		GLERRLOG
		if (inData != nullptr)	{
			glBufferSubData(inTarget, 0, pboSizeInBytes, inData);
			GLERRLOG
		}
		//glBufferData(inTarget, pboSizeInBytes, inData, inUsage);
		//GLERRLOG
		//	un-bind the PBO
		glBindBuffer(returnMe->desc.target, 0);
		GLERRLOG
	}
	
	return returnMe;
}
GLBufferRef CreateRGBAFloatPBO(const int32_t & inTarget, const int32_t & inUsage, const Size & inSize, const void * inData, const bool & inCreateInCurrentContext, const GLBufferPoolRef & inPoolRef)	{
	if (inPoolRef == nullptr)
		return nullptr;
	
	GLBuffer::Descriptor	desc;
	
	desc.type = GLBuffer::Type_PBO;
	desc.target = (VVGL::GLBuffer::Target)inTarget;
	desc.internalFormat = GLBuffer::IF_RGBA32F;
	desc.pixelType = GLBuffer::PT_Float;
	desc.pixelFormat = GLBuffer::PF_RGBA;
	desc.cpuBackingType = GLBuffer::Backing_None;
	desc.gpuBackingType = GLBuffer::Backing_Internal;
	desc.texRangeFlag = false;
	desc.texClientStorageFlag = false;
	desc.msAmount = 0;
	desc.localSurfaceID = 0;
	
	size_t			pboSizeInBytes = desc.backingLengthForSize(inSize);
	//	first try to find a PBO that matches the provided data
	GLBufferRef		returnMe = inPoolRef->fetchMatchingFreeBuffer(desc, inSize);
	//	if we found a PBO, we're recycling it: we need to discard-initialize it, and then fill it with a non-reserving call
	if (returnMe != nullptr)	{
		returnMe->parentBufferPool = inPoolRef;
		returnMe->backingSize = inSize;
		//	...if we're recycling then we may not have a current context on this thread right now
		if (!inCreateInCurrentContext)	{
			GLContextRef		tmpCtx = inPoolRef->getContext();
			if (tmpCtx == nullptr)
				return nullptr;
			//context->makeCurrentIfNull();
			//context->makeCurrent();
			tmpCtx->makeCurrentIfNotCurrent();
		}
		//	bind the PBO
		glBindBuffer(returnMe->desc.target, returnMe->name);
		GLERRLOG
		//	if the pbo is currently mapped, unmap it
		if (returnMe->pboMapped)	{
			glUnmapBuffer(inTarget);
			GLERRLOG
			returnMe->pboMapped = false;
			returnMe->cpuBackingPtr = nullptr;
		}
		glBufferData(inTarget, pboSizeInBytes, nullptr, inUsage);
		GLERRLOG
		if (inData != nullptr)	{
			glBufferSubData(inTarget, 0, pboSizeInBytes, inData);
			GLERRLOG
		}
		//	un-bind the PBO
		glBindBuffer(returnMe->desc.target, 0);
		GLERRLOG
	}
	//	else we didn't find a matching PBO
	else	{
		//	ask the pool to create a new PBO
		returnMe = inPoolRef->createBufferRef(desc, inSize, nullptr, inSize, inCreateInCurrentContext);
		if (returnMe == nullptr)
			return nullptr;
		returnMe->parentBufferPool = inPoolRef;
		returnMe->backingSize = inSize;
		//	...logically, we shouldn't have to check GL contexts- we just created a PBO, so either the buffer pool context is current or the originally-current context is still current.
		//	bind the PBO
		glBindBuffer(returnMe->desc.target, returnMe->name);
		GLERRLOG
		//	reserve-initialize the PBO the pool just created with the data we were passed
		glBufferData(inTarget, pboSizeInBytes, nullptr, inUsage);
		GLERRLOG
		if (inData != nullptr)	{
			glBufferSubData(inTarget, 0, pboSizeInBytes, inData);
			GLERRLOG
		}
		//glBufferData(inTarget, pboSizeInBytes, inData, inUsage);
		//GLERRLOG
		//	un-bind the PBO
		glBindBuffer(returnMe->desc.target, 0);
		GLERRLOG
	}
	
	return returnMe;
}
GLBufferRef CreateBGRAFloatPBO(const int32_t & inTarget, const int32_t & inUsage, const Size & inSize, const void * inData, const bool & inCreateInCurrentContext, const GLBufferPoolRef & inPoolRef)	{
	if (inPoolRef == nullptr)
		return nullptr;
	
	GLBuffer::Descriptor	desc;
	
	desc.type = GLBuffer::Type_PBO;
	desc.target = (VVGL::GLBuffer::Target)inTarget;
	desc.internalFormat = GLBuffer::IF_RGBA32F;
	desc.pixelType = GLBuffer::PT_Float;
	desc.pixelFormat = GLBuffer::PF_RGBA;
	desc.cpuBackingType = GLBuffer::Backing_None;
	desc.gpuBackingType = GLBuffer::Backing_Internal;
	desc.texRangeFlag = false;
	desc.texClientStorageFlag = false;
	desc.msAmount = 0;
	desc.localSurfaceID = 0;
	
	size_t			pboSizeInBytes = desc.backingLengthForSize(inSize);
	//	first try to find a PBO that matches the provided data
	GLBufferRef		returnMe = inPoolRef->fetchMatchingFreeBuffer(desc, inSize);
	//	if we found a PBO, we're recycling it: we need to discard-initialize it, and then fill it with a non-reserving call
	if (returnMe != nullptr)	{
		returnMe->parentBufferPool = inPoolRef;
		returnMe->backingSize = inSize;
		//	...if we're recycling then we may not have a current context on this thread right now
		if (!inCreateInCurrentContext)	{
			GLContextRef		tmpCtx = inPoolRef->getContext();
			if (tmpCtx == nullptr)
				return nullptr;
			//context->makeCurrentIfNull();
			//context->makeCurrent();
			tmpCtx->makeCurrentIfNotCurrent();
		}
		//	bind the PBO
		glBindBuffer(returnMe->desc.target, returnMe->name);
		GLERRLOG
		//	if the pbo is currently mapped, unmap it
		if (returnMe->pboMapped)	{
			glUnmapBuffer(inTarget);
			GLERRLOG
			returnMe->pboMapped = false;
			returnMe->cpuBackingPtr = nullptr;
		}
		glBufferData(inTarget, pboSizeInBytes, nullptr, inUsage);
		GLERRLOG
		if (inData != nullptr)	{
			glBufferSubData(inTarget, 0, pboSizeInBytes, inData);
			GLERRLOG
		}
		//	un-bind the PBO
		glBindBuffer(returnMe->desc.target, 0);
		GLERRLOG
	}
	//	else we didn't find a matching PBO
	else	{
		//	ask the pool to create a new PBO
		returnMe = inPoolRef->createBufferRef(desc, inSize, nullptr, inSize, inCreateInCurrentContext);
		if (returnMe == nullptr)
			return nullptr;
		returnMe->parentBufferPool = inPoolRef;
		returnMe->backingSize = inSize;
		//	...logically, we shouldn't have to check GL contexts- we just created a PBO, so either the buffer pool context is current or the originally-current context is still current.
		//	bind the PBO
		glBindBuffer(returnMe->desc.target, returnMe->name);
		GLERRLOG
		//	reserve-initialize the PBO the pool just created with the data we were passed
		glBufferData(inTarget, pboSizeInBytes, nullptr, inUsage);
		GLERRLOG
		if (inData != nullptr)	{
			glBufferSubData(inTarget, 0, pboSizeInBytes, inData);
			GLERRLOG
		}
		//glBufferData(inTarget, pboSizeInBytes, inData, inUsage);
		//GLERRLOG
		//	un-bind the PBO
		glBindBuffer(returnMe->desc.target, 0);
		GLERRLOG
	}
	
	return returnMe;
}
GLBufferRef CreateYCbCrPBO(const int32_t & inTarget, const int32_t & inUsage, const Size & inSize, const void * inData, const bool & inCreateInCurrentContext, const GLBufferPoolRef & inPoolRef)	{
	if (inPoolRef == nullptr)
		return nullptr;
	
	GLBuffer::Descriptor	desc;
	
	desc.type = GLBuffer::Type_PBO;
	desc.target = (VVGL::GLBuffer::Target)inTarget;
	desc.internalFormat = GLBuffer::IF_RGB;
	desc.pixelType = GLBuffer::PT_UShort88;
	desc.pixelFormat = GLBuffer::PF_YCbCr_422;
	desc.cpuBackingType = GLBuffer::Backing_None;
	desc.gpuBackingType = GLBuffer::Backing_Internal;
	desc.texRangeFlag = false;
	desc.texClientStorageFlag = false;
	desc.msAmount = 0;
	desc.localSurfaceID = 0;
	
	size_t			pboSizeInBytes = desc.backingLengthForSize(inSize);
	//	first try to find a PBO that matches the provided data
	GLBufferRef		returnMe = inPoolRef->fetchMatchingFreeBuffer(desc, inSize);
	//	if we found a PBO, we're recycling it: we need to discard-initialize it, and then fill it with a non-reserving call
	if (returnMe != nullptr)	{
		returnMe->parentBufferPool = inPoolRef;
		returnMe->backingSize = inSize;
		//	...if we're recycling then we may not have a current context on this thread right now
		if (!inCreateInCurrentContext)	{
			GLContextRef		tmpCtx = inPoolRef->getContext();
			if (tmpCtx == nullptr)
				return nullptr;
			//context->makeCurrentIfNull();
			//context->makeCurrent();
			tmpCtx->makeCurrentIfNotCurrent();
		}
		//	bind the PBO
		glBindBuffer(returnMe->desc.target, returnMe->name);
		GLERRLOG
		//	if the pbo is currently mapped, unmap it
		if (returnMe->pboMapped)	{
			glUnmapBuffer(inTarget);
			GLERRLOG
			returnMe->pboMapped = false;
			returnMe->cpuBackingPtr = nullptr;
		}
		glBufferData(inTarget, pboSizeInBytes, nullptr, inUsage);
		GLERRLOG
		if (inData != nullptr)	{
			glBufferSubData(inTarget, 0, pboSizeInBytes, inData);
			GLERRLOG
		}
		//	un-bind the PBO
		glBindBuffer(returnMe->desc.target, 0);
		GLERRLOG
	}
	//	else we didn't find a matching PBO
	else	{
		//	ask the pool to create a new PBO
		returnMe = inPoolRef->createBufferRef(desc, inSize, nullptr, inSize, inCreateInCurrentContext);
		if (returnMe == nullptr)
			return nullptr;
		returnMe->parentBufferPool = inPoolRef;
		returnMe->backingSize = inSize;
		//	...logically, we shouldn't have to check GL contexts- we just created a PBO, so either the buffer pool context is current or the originally-current context is still current.
		//	bind the PBO
		glBindBuffer(returnMe->desc.target, returnMe->name);
		GLERRLOG
		//	reserve-initialize the PBO the pool just created with the data we were passed
		glBufferData(inTarget, pboSizeInBytes, nullptr, inUsage);
		GLERRLOG
		if (inData != nullptr)	{
			glBufferSubData(inTarget, 0, pboSizeInBytes, inData);
			GLERRLOG
		}
		//glBufferData(inTarget, pboSizeInBytes, inData, inUsage);
		//GLERRLOG
		//	un-bind the PBO
		glBindBuffer(returnMe->desc.target, 0);
		GLERRLOG
	}
	
	return returnMe;
}

#endif	//	!defined(VVGL_TARGETENV_GLES) && !defined(VVGL_TARGETENV_GLES3)




/*	========================================	*/
#pragma mark --------------------- Qt funcs




#if defined(VVGL_SDK_QT)
GLBufferRef CreateBufferForQImage(QImage * inImg, const bool & createInCurrentContext, const GLBufferPoolRef & inPoolRef)	{
	if (inImg==nullptr || inPoolRef==nullptr)
		return nullptr;
	void			*pixelData = inImg->bits();
	if (pixelData == nullptr)
		return nullptr;
	QSize			imgSize = inImg->size();
	VVGL::Size		repSize(imgSize.width(), imgSize.height());
	VVGL::Size		gpuSize = repSize;
	
	GLBuffer::Descriptor		desc;
	desc.type = GLBuffer::Type_Tex;
	desc.target = GLBuffer::Target_2D;
	desc.internalFormat = GLBuffer::IF_RGBA8;
	desc.pixelFormat = GLBuffer::PF_BGRA;
	desc.pixelType = GLBuffer::PT_UInt_8888_Rev;
	desc.cpuBackingType = GLBuffer::Backing_None;
	desc.gpuBackingType = GLBuffer::Backing_Internal;
	desc.texRangeFlag = false;
	desc.texClientStorageFlag = true;
	desc.msAmount = 0;
	desc.localSurfaceID = 0;
	
	GLBufferRef		returnMe = inPoolRef->createBufferRef(desc, gpuSize, pixelData, repSize, createInCurrentContext);
	returnMe->parentBufferPool = inPoolRef;
	returnMe->srcRect = VVGL::Rect(0,0,repSize.width,repSize.height);
	returnMe->backingID = GLBuffer::BackingID_None;
	returnMe->backingSize = repSize;
	returnMe->flipped = true;
	
	returnMe->preferDeletion = true;
	
	return returnMe;
}
#endif




/*	========================================	*/
#pragma mark --------------------- mac-only funcs




#if defined(VVGL_SDK_MAC)
void PushTexRangeBufferRAMtoVRAM(const GLBufferRef & inBufferRef, const GLContextRef & inContextRef)	{
	if (inBufferRef == nullptr)
		return;
	if (inContextRef == nullptr)	{
		cout << "\tERR: context nil in " << __PRETTY_FUNCTION__ << endl;
		return;
	}
	GLBuffer::Descriptor		&desc = inBufferRef->desc;
	if (desc.type != GLBuffer::Type_Tex)
		return;
	if (desc.cpuBackingType == GLBuffer::Backing_None)
		return;
	if (desc.gpuBackingType != GLBuffer::Backing_Internal)
		return;
	if (!desc.texRangeFlag)
		return;
	
	VVGL::Size		bSize = inBufferRef->srcRect.size;
	void			*pixels = inBufferRef->cpuBackingPtr;
	bool			doCompressedUpload = false;
	
	inContextRef->makeCurrentIfNotCurrent();
	
//#if defined(VVGL_TARGETENV_GL2)
	glActiveTexture(GL_TEXTURE0);
	GLERRLOG
	if (inContextRef->version == GLVersion_2)	{
		glEnable(desc.target);
		GLERRLOG
	}
//#endif
	glBindTexture(desc.target, inBufferRef->name);
	GLERRLOG
	
	if (desc.texClientStorageFlag)	{
		glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_TRUE);
		GLERRLOG
	}
	
	switch (desc.internalFormat)	{
	case GLBuffer::IF_None:
	case GLBuffer::IF_R:
	case GLBuffer::IF_RGB:
	case GLBuffer::IF_RGBA:
	case GLBuffer::IF_RGBA8:
	case GLBuffer::IF_RGBA32F:
	case GLBuffer::IF_Depth24:
		doCompressedUpload = false;
		bSize = inBufferRef->size;
		break;
	case GLBuffer::IF_RGB_DXT1:
	case GLBuffer::IF_RGBA_DXT5:
	//case GLBuffer::IF_YCoCg_DXT5:	//	(flagged as duplicate case if un-commented, because both RGBA_DXT5 and YCoCg_DXT5 evaluate to the same internal format)
	case GLBuffer::IF_A_RGTC:
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
		GLERRLOG
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
		GLERRLOG
		//NSLog(@"\t\tfinished compressed upload");
	}
	/*
	if (desc->backingType == VVBufferBack_GWorld)	{
		//if (pmHandle != nil)
		//	UnlockPixels(pmHandle);
	}
	*/
	if (desc.texClientStorageFlag)	{
		glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_FALSE);
		GLERRLOG
	}
	
	glBindTexture(desc.target, 0);
	GLERRLOG
	if (inContextRef->version == GLVersion_2)	{
		glDisable(desc.target);
		GLERRLOG
	}
	glFlush();
	GLERRLOG
	
	//	timestamp the buffer, so we know a new frame has been pushed to it!
	//[VVBufferPool timestampThisBuffer:b];
	const GLBufferPoolRef	&bp = GetGlobalBufferPool();
	if (bp != nullptr)
		bp->timestampThisBuffer(inBufferRef);
}
GLBufferRef CreateRGBATexIOSurface(const Size & inSize, const bool & inCreateInCurrentContext, const GLBufferPoolRef & inPoolRef)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	if (inPoolRef == nullptr)
		return nullptr;
	
	GLBuffer::Descriptor	desc;
	
	desc.type = GLBuffer::Type_Tex;
	desc.target = GLBuffer::Target_Rect;
	desc.internalFormat = GLBuffer::IF_RGBA8;
	desc.pixelFormat = GLBuffer::PF_BGRA;
	desc.pixelType = GLBuffer::PT_UInt_8888_Rev;
	desc.cpuBackingType = GLBuffer::Backing_None;
	desc.gpuBackingType = GLBuffer::Backing_Internal;
	desc.texRangeFlag = false;
	desc.texClientStorageFlag = false;
	desc.msAmount = 0;
	desc.localSurfaceID = 1;
	
	GLBufferRef	returnMe = inPoolRef->createBufferRef(desc, inSize, nullptr, Size(), inCreateInCurrentContext);
	returnMe->parentBufferPool = inPoolRef;
	
	return returnMe;
}
GLBufferRef CreateRGBAFloatTexIOSurface(const Size & inSize, const bool & inCreateInCurrentContext, const GLBufferPoolRef & inPoolRef)	{
	if (inPoolRef == nullptr)
		return nullptr;
	
	GLBuffer::Descriptor	desc;
	
	desc.type = GLBuffer::Type_Tex;
	desc.target = GLBuffer::Target_Rect;
	desc.internalFormat = GLBuffer::IF_RGBA32F;
	desc.pixelFormat = GLBuffer::PF_RGBA;
	desc.pixelType = GLBuffer::PT_Float;
	desc.cpuBackingType = GLBuffer::Backing_None;
	desc.gpuBackingType = GLBuffer::Backing_Internal;
	desc.texRangeFlag = false;
	desc.texClientStorageFlag = false;
	desc.msAmount = 0;
	desc.localSurfaceID = 1;
	
	GLBufferRef	returnMe = inPoolRef->createBufferRef(desc, inSize, nullptr, Size(), inCreateInCurrentContext);
	returnMe->parentBufferPool = inPoolRef;
	
	return returnMe;
}
GLBufferRef CreateRGBATexFromIOSurfaceID(const IOSurfaceID & inID, const bool & inCreateInCurrentContext, const GLBufferPoolRef & inPoolRef)	{
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
	GLBufferRef		returnMe = make_shared<GLBuffer>(inPoolRef);
	inPoolRef->timestampThisBuffer(returnMe);
	returnMe->desc.type = GLBuffer::Type_Tex;
	returnMe->desc.target = GLBuffer::Target_Rect;
	
	switch (pixelFormat)	{
	case kCVPixelFormatType_32BGRA:	//	'BGRA'
	case GLBuffer::PF_BGRA:
		returnMe->desc.pixelFormat = GLBuffer::PF_BGRA;
		if (isRGBAFloatTex)	{
			returnMe->desc.internalFormat = GLBuffer::IF_RGBA32F;
			returnMe->desc.pixelType = GLBuffer::PT_Float;
		}
		else	{
			returnMe->desc.internalFormat = GLBuffer::IF_RGBA8;
			returnMe->desc.pixelType = GLBuffer::PT_UInt_8888_Rev;
		}
		break;
	case kCVPixelFormatType_32RGBA:	//	'RGBA'
	case GLBuffer::PF_RGBA:
		returnMe->desc.pixelFormat = GLBuffer::PF_RGBA;
		if (isRGBAFloatTex)	{
			returnMe->desc.internalFormat = GLBuffer::IF_RGBA32F;
			returnMe->desc.pixelType = GLBuffer::PT_Float;
		}
		else	{
			returnMe->desc.internalFormat = GLBuffer::IF_RGBA8;
			returnMe->desc.pixelType = GLBuffer::PT_UInt_8888_Rev;
		}
		break;
	case kCVPixelFormatType_422YpCbCr8:	//	'2vuy'
	case GLBuffer::PF_YCbCr_422:
		returnMe->desc.internalFormat = GLBuffer::IF_RGB;
		returnMe->desc.pixelFormat = GLBuffer::PF_YCbCr_422;
		returnMe->desc.pixelType = GLBuffer::PT_UShort88;
		break;
	default:
		cout << "\tERR: unknown pixel format (" << pixelFormat << ") in " << __PRETTY_FUNCTION__ << endl;
		returnMe->desc.internalFormat = GLBuffer::IF_RGBA8;
		returnMe->desc.pixelFormat = GLBuffer::PF_BGRA;
		returnMe->desc.pixelType = GLBuffer::PT_UInt_8888_Rev;
		break;
	}
	
	returnMe->desc.cpuBackingType = GLBuffer::Backing_None;
	returnMe->desc.gpuBackingType = GLBuffer::Backing_Internal;
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
	returnMe->backingID = GLBuffer::BackingID_RemoteIOSrf;
	
	returnMe->setRemoteSurfaceRef(newSurface);
	
	//	we can free the surface now that the buffer we'll be returning has retained it
	CFRelease(newSurface);
	
	//	...now that i've created and set up the GLBuffer, take care of the GL resource setup...
	
	//	grab a context lock so we can do stuff with the GL context
	lock_guard<recursive_mutex>		lock(inPoolRef->getContextLock());
	if (!inCreateInCurrentContext)	{
		GLContextRef					context = (inPoolRef==nullptr) ? nullptr : inPoolRef->getContext();
		context->makeCurrentIfNotCurrent();
	}
	CGLContextObj		cglCtx = CGLGetCurrentContext();
	if (cglCtx != NULL)	{
		glActiveTexture(GL_TEXTURE0);
		GLERRLOG
		if (inPoolRef->getContext()->version <= GLVersion_2)	{
#if defined(VVGL_TARGETENV_GL2)
			glEnable(returnMe->desc.target);
			GLERRLOG
#endif
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
#if defined(VVGL_TARGETENV_GL2)
			glDisable(returnMe->desc.target);
			GLERRLOG
#endif
		}
	}
	
	return returnMe;
}
GLBufferRef CreateBufferForCVPixelBuffer(CVPixelBufferRef & inCVPB, const bool & inTexRange, const bool & inIOSurface, const bool & createInCurrentContext, const GLBufferPoolRef & inPoolRef)	{
	if (inPoolRef==nullptr || inCVPB==NULL || inPoolRef->getContext()==nullptr)
		return nullptr;
	
	GLBuffer::Descriptor	desc;
	
	desc.type = GLBuffer::Type_Tex;
	desc.target = GLBuffer::Target_Rect;
	desc.internalFormat = GLBuffer::IF_RGBA8;
	desc.pixelFormat = GLBuffer::PF_BGRA;
	desc.pixelType = GLBuffer::PT_UInt_8888_Rev;
	desc.cpuBackingType = GLBuffer::Backing_External;
	desc.gpuBackingType = GLBuffer::Backing_Internal;
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
			/*
			bitsPerPixel = 16;
			desc.internalFormat = GLBuffer::IF_RGB;
			desc.pixelFormat = GLBuffer::PF_YCbCr_422;
			desc.pixelType = GLBuffer::PT_UShort88;
			*/
			
			if (inPoolRef->getContext()->version == GLVersion_4)	{
				//	GL4 doesn't have YCbCr textures on os x, so we create it as a half-width RGBA image
				bitsPerPixel = 32;
			}
			else	{
				bitsPerPixel = 16;
				desc.internalFormat = GLBuffer::IF_RGB;
				desc.pixelFormat = GLBuffer::PF_YCbCr_422;
				desc.pixelType = GLBuffer::PT_UShort88;
			}
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
	
	GLBufferRef	returnMe = inPoolRef->createBufferRef(desc, cvpb_backingSize, cvpb_baseAddr, cvpb_backingSize, createInCurrentContext);
	returnMe->parentBufferPool = inPoolRef;
	
	//CVPixelBufferUnlockBaseAddress(inCVPB, 0);	//	don't unlock- we'll unlock when we release
	CVPixelBufferRetain(inCVPB);	//	the CVPixelBuffer is retained by the VVBuffer for the backing release callback context, here's where the retain count is incremented
	
	returnMe->backingID = GLBuffer::BackingID_CVPixBuf;
	returnMe->backingContext = inCVPB;
	returnMe->backingReleaseCallback = [](GLBuffer& inBuffer, void* inReleaseContext) {
		CVPixelBufferRef	tmpRef = (CVPixelBufferRef)inReleaseContext;
		if (tmpRef != nil)	{
			//NSLog(@"\t\tunlocking %p",tmpRef);
			CVPixelBufferUnlockBaseAddress(tmpRef, 0);
			CVPixelBufferRelease(tmpRef);
		}
	};
	returnMe->preferDeletion = true;
	
	if (inPoolRef->getContext()->version == GLVersion_4)	{
		returnMe->srcRect = VVGL::Rect(cvpb_srcRect.origin.x, cvpb_srcRect.origin.y, cvpb_srcRect.size.width/2., cvpb_srcRect.size.height);
	}
	else	{
		returnMe->srcRect = VVGL::Rect(cvpb_srcRect.origin.x, cvpb_srcRect.origin.y, cvpb_srcRect.size.width, cvpb_srcRect.size.height);
	}
	
	returnMe->flipped = (CVImageBufferIsFlipped(inCVPB)) ? true : false;
	
	return returnMe;
}
GLBufferRef CreateTexRangeFromCMSampleBuffer(CMSampleBufferRef & n, const bool & createInCurrentContext, const GLBufferPoolRef & inPoolRef)	{
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
	GLBufferRef			returnMe = nullptr;
	if (cvImg!=NULL && (CFGetTypeID(cvImg) == CVPixelBufferGetTypeID()))	{
		//	make the actual buffer i'll be returning
		//returnMe = [self allocBufferForCVPixelBuffer:cvImg texRange:YES ioSurface:NO];
		returnMe = CreateBufferForCVPixelBuffer(cvImg, true, false, createInCurrentContext, inPoolRef);
		if (returnMe != nullptr)	{
			//	get the CMSampleBuffer timing info, apply it to the buffer
			CMTime				bufferTime = CMSampleBufferGetPresentationTimeStamp(n);
			returnMe->contentTimestamp = VVGL::Timestamp((uint64_t)(bufferTime.value), bufferTime.timescale);
			//double				timeInSec = (!CMTIME_IS_VALID(bufferTime)) ? 0. : CMTimeGetSeconds(bufferTime);
			//uint64_t			tmpTimestamp = SWatchAbsTimeUnitForTime(timeInSec);
			//[returnMe setContentTimestampFromPtr:&tmpTimestamp];
		}
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
GLBufferRef CreateRGBARectTex(const Size & size, const bool & inCreateInCurrentContext, const GLBufferPoolRef & inPoolRef)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	if (inPoolRef == nullptr)
		return nullptr;
	
	GLBuffer::Descriptor	desc;
	
	desc.type = GLBuffer::Type_Tex;
	desc.target = GLBuffer::Target_Rect;
//#if defined(VVGL_SDK_MAC)
	desc.internalFormat = GLBuffer::IF_RGBA8;
//#else
//	desc.internalFormat = GLBuffer::IF_RGBA;
//#endif
	desc.pixelFormat = GLBuffer::PF_RGBA;
	desc.pixelType = GLBuffer::PT_UByte;
	desc.cpuBackingType = GLBuffer::Backing_None;
	desc.gpuBackingType = GLBuffer::Backing_Internal;
	desc.texRangeFlag = false;
	desc.texClientStorageFlag = false;
	desc.msAmount = 0;
	desc.localSurfaceID = 0;
	
	GLBufferRef	returnMe = inPoolRef->createBufferRef(desc, size, nullptr, Size(), inCreateInCurrentContext);
	returnMe->parentBufferPool = inPoolRef;
	
	return returnMe;
}
#endif	//	VVGL_SDK_MAC




/*	========================================	*/
#pragma mark --------------------- mac & ios funcs




#if defined(VVGL_SDK_MAC) || defined(VVGL_SDK_IOS)
GLBufferRef CreateTexFromCGImageRef(const CGImageRef & n, const bool & inCreateInCurrentContext, const GLBufferPoolRef & inPoolRef)	{
	if (inPoolRef == nullptr)
		return nullptr;
	
	lock_guard<recursive_mutex>		lock(inPoolRef->getContextLock());
	if (!inCreateInCurrentContext)	{
		GLContextRef				context = inPoolRef->getContext();
		if (context == nullptr)
			return nullptr;
		//context->makeCurrentIfNull();
		//context->makeCurrent();
		context->makeCurrentIfNotCurrent();
	}
	
	bool				directUploadOK = true;
	CGBitmapInfo		newImgInfo = CGImageGetBitmapInfo(n);
	CGImageAlphaInfo	calculatedAlphaInfo = static_cast<CGImageAlphaInfo>(newImgInfo & kCGBitmapAlphaInfoMask);
	
	//((newImgInfo & kCGBitmapByteOrderDefault) == kCGBitmapByteOrderDefault)	||
	if (((newImgInfo & kCGBitmapFloatComponents) == kCGBitmapFloatComponents)	||
	((newImgInfo & kCGBitmapByteOrder16Little) == kCGBitmapByteOrder16Little)	||
	((newImgInfo & kCGBitmapByteOrder16Big) == kCGBitmapByteOrder16Big)	||
	//((newImgInfo & kCGBitmapByteOrder32Little) == kCGBitmapByteOrder32Little)	||
	((newImgInfo & kCGBitmapByteOrder32Big) == kCGBitmapByteOrder32Big))	{
		directUploadOK = false;
	}
	
	switch (calculatedAlphaInfo)	{
	case kCGImageAlphaPremultipliedLast:
	case kCGImageAlphaPremultipliedFirst:
	case kCGImageAlphaOnly:
	case kCGImageAlphaNone:
		directUploadOK = false;
		break;
	case kCGImageAlphaLast:
	case kCGImageAlphaFirst:
	case kCGImageAlphaNoneSkipLast:
	case kCGImageAlphaNoneSkipFirst:
		break;
	}
	
	GLBufferRef		returnMe = nullptr;
	
	Size				imgSize = Size(CGImageGetWidth(n), CGImageGetHeight(n));
	//	if i can upload the pixel data from the CGImageRef directly to a texture...
	if (directUploadOK)	{
		//	this just copies the data right out of the image provider, let's give it a shot...
		CFDataRef	frameData = CGDataProviderCopyData(CGImageGetDataProvider(n));
		if (frameData != NULL)	{
			
			GLBuffer::Descriptor	desc;
			desc.type = GLBuffer::Type_Tex;
			desc.target = GLBuffer::Target_2D;
#if defined(VVGL_SDK_MAC)
			desc.internalFormat = GLBuffer::IF_RGBA8;
#else
			desc.internalFormat = GLBuffer::IF_RGBA;
#endif
			desc.pixelFormat = GLBuffer::PF_RGBA;
#if defined(VVGL_SDK_MAC)
			desc.pixelType = GLBuffer::PT_UInt_8888_Rev;
#else
			desc.pixelType = GLBuffer::PT_UByte;
#endif
			desc.cpuBackingType = GLBuffer::Backing_External;
			desc.gpuBackingType = GLBuffer::Backing_Internal;
			desc.texRangeFlag = true;
			desc.texClientStorageFlag = true;
			desc.msAmount = 0;
			desc.localSurfaceID = 0;
			
			returnMe = inPoolRef->createBufferRef(desc, imgSize, (void*)CFDataGetBytePtr(frameData), imgSize, inCreateInCurrentContext);
			if (returnMe != nullptr)	{
				returnMe->parentBufferPool = inPoolRef;
			
				CFRetain(frameData);
				returnMe->backingContext = (void*)(frameData);
				returnMe->backingReleaseCallback = [](GLBuffer& inBuffer, void* inReleaseContext) {
					CFRelease((CFDataRef)(inReleaseContext));
				};
				returnMe->preferDeletion = true;
				returnMe->flipped = true;
			}
			
			CFRelease(frameData);
			frameData = NULL;
		}
	}
	//	else the direct upload isn't okay...
	else	{
		//	alloc some memory, make a CGBitmapContext that uses the memory to store pixel data
		GLubyte			*imgData = (GLubyte*)calloc((long)imgSize.width * (long)imgSize.height, sizeof(GLubyte)*4);
		CGContextRef	ctx = CGBitmapContextCreate(imgData,
			(long)imgSize.width,
			(long)imgSize.height,
			8,
			((long)(imgSize.width))*4,
			inPoolRef->getColorSpace(),
			kCGImageAlphaPremultipliedLast);
		if (ctx == NULL)	{
			cout << "ERR: ctx null in " << __PRETTY_FUNCTION__ << endl;
			free(imgData);
			return nullptr;
		}
		//	draw the image in the bitmap context, flush it
		CGContextDrawImage(ctx, CGRectMake(0,0,imgSize.width,imgSize.height), n);
		CGContextFlush(ctx);
		//	the bitmap context we just drew into has premultiplied alpha, so we need to un-premultiply before uploading it
		CGBitmapContextUnpremultiply(ctx);
		CGContextRelease(ctx);
		
		GLBuffer::Descriptor	desc;
		desc.type = GLBuffer::Type_Tex;
		desc.target = GLBuffer::Target_2D;
#if defined(VVGL_SDK_MAC)
		desc.internalFormat = GLBuffer::IF_RGBA8;
#else
		desc.internalFormat = GLBuffer::IF_RGBA;
#endif
		desc.pixelFormat = GLBuffer::PF_RGBA;
		desc.pixelType = GLBuffer::PT_UByte;
		desc.cpuBackingType = GLBuffer::Backing_External;
		desc.gpuBackingType = GLBuffer::Backing_Internal;
		desc.texRangeFlag = false;
		desc.texClientStorageFlag = true;
		desc.msAmount = 0;
		desc.localSurfaceID = 0;
		
		returnMe = inPoolRef->createBufferRef(desc, imgSize, (void*)imgData, imgSize, inCreateInCurrentContext);
		if (returnMe != nullptr)	{
			returnMe->parentBufferPool = inPoolRef;
			
			returnMe->backingContext = (void*)imgData;
			returnMe->backingReleaseCallback = [](GLBuffer& inBuffer, void *inReleaseContext)	{
				if (inReleaseContext != nullptr)	{
					free(inReleaseContext);
				}
			};
			
			returnMe->backingID = GLBuffer::BackingID_Pixels;
			returnMe->preferDeletion = true;
			returnMe->flipped = true;
		}
		else	{
			free(imgData);
			imgData = NULL;
		}
	}
	
	if (returnMe != nullptr)
		inPoolRef->timestampThisBuffer(returnMe);
	
	return returnMe;
	
	//return nullptr;
}
GLBufferRef CreateCubeTexFromImages(const vector<CGImageRef> & inImgs, const bool & inCreateInCurrentContext, const GLBufferPoolRef & inPoolRef)	{
	//cout << __FUNCTION__ << endl;
	if (inPoolRef == nullptr)
		return nullptr;
	//	make sure that i was passed six images, and that all six images have the same dimensions
	if (inImgs.size() != 6)	{
		cout << "ERR: bailing, not passed 6 images, " << __FUNCTION__ << endl;
		return nullptr;
	}
	//	make sure that the dimensions of all the passed images are identical
	VVGL::Size		baseSize(CGImageGetWidth(inImgs[0]), CGImageGetHeight(inImgs[0]));
	for (const auto & imgIt : inImgs)	{
		VVGL::Size		tmpSize(CGImageGetWidth(imgIt), CGImageGetHeight(imgIt));
		if (baseSize != tmpSize)
			return nullptr;
	}
	
//#if defined(VVGL_SDK_RPI)
//	return nullptr;
//#else
	//	assemble a descriptor for the resource we want to create
	GLBuffer::Descriptor	desc;
	desc.type = GLBuffer::Type_Tex;
	desc.target = GLBuffer::Target_Cube;
#if defined(VVGL_SDK_MAC)
	desc.internalFormat = GLBuffer::IF_RGBA8;
	desc.pixelFormat = GLBuffer::PF_RGBA;
	desc.pixelType = GLBuffer::PT_UInt_8888_Rev;
#else
	desc.internalFormat = GLBuffer::IF_RGBA;
	desc.pixelFormat = GLBuffer::PF_RGBA;
	desc.pixelType = GLBuffer::PT_UByte;
#endif
	desc.cpuBackingType = GLBuffer::Backing_None;
	desc.gpuBackingType = GLBuffer::Backing_Internal;
	desc.texRangeFlag = false;
	desc.texClientStorageFlag = false;
	desc.msAmount = 0;
	desc.localSurfaceID = 0;
	//	create the GLBuffer we'll be returning, set it up with the basic info
	GLBufferRef		returnMe = make_shared<GLBuffer>();
	returnMe->parentBufferPool = inPoolRef;
	returnMe->desc = desc;
	returnMe->preferDeletion = true;
	returnMe->size = baseSize;
	returnMe->srcRect = Rect(0,0,baseSize.width,baseSize.height);
	inPoolRef->timestampThisBuffer(returnMe);
	
	
	//	lock, set the current context
	lock_guard<recursive_mutex>		lock(inPoolRef->getContextLock());
	if (!inCreateInCurrentContext)	{
		GLContextRef				context = inPoolRef->getContext();
		if (context == nullptr)
			return nullptr;
		//context->makeCurrentIfNull();
		//context->makeCurrent();
		context->makeCurrentIfNotCurrent();
	}
	
	//	create the GL resource, do some basic setup before we upload data to it
	glActiveTexture(GL_TEXTURE0);
	GLERRLOG
	if (inPoolRef->getContext()->version <= GLVersion_2)	{
#if defined(VVGL_TARGETENV_GL2)
		glEnable(desc.target);
		GLERRLOG
#endif
	}
	glGenTextures(1, &(returnMe->name));
	GLERRLOG
	glBindTexture(desc.target, returnMe->name);
	GLERRLOG
	
	glPixelStorei(GL_UNPACK_SKIP_ROWS, GL_FALSE);
	GLERRLOG
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, GL_FALSE);
	GLERRLOG
#if !defined(VVGL_SDK_IOS)
	glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
	GLERRLOG
#endif
	
	glTexParameteri(desc.target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	GLERRLOG
	glTexParameteri(desc.target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	GLERRLOG
	glTexParameteri(desc.target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	GLERRLOG
	glTexParameteri(desc.target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	GLERRLOG
	glFlush();
	GLERRLOG
	
	
	//	allocate some "clipboard data"- we're going to draw each of the images into this data (it's bitmap data) and then upload this
	int				faceCount = 0;
	uint32_t		bytesPerRow = 32 * baseSize.width / 8;
	void			*clipboardData = malloc(bytesPerRow * baseSize.height);
	CGContextRef	ctx = CGBitmapContextCreate(clipboardData,
		(long)baseSize.width,
		(long)baseSize.height,
		8,
		((long)(baseSize.width))*4,
		inPoolRef->getColorSpace(),
		kCGImageAlphaPremultipliedLast);
	if (ctx != NULL)	{
		//	run through the images, drawing them into the clipboard and then uploading the clipboard
		for (const auto & imgIt : inImgs)	{
			//	draw the image in the bitmap context, flush it
			CGContextDrawImage(ctx, CGRectMake(0,0,baseSize.width,baseSize.height), imgIt);
			CGContextFlush(ctx);
			//	the context we just drew into has premultiplied alpha, so we need to un-premultiply it
			CGBitmapContextUnpremultiply(ctx);
			//	upload the bitmap data to the texture
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + faceCount,
				0,
				desc.internalFormat,
				baseSize.width,
				baseSize.height,
				0,
				desc.pixelFormat,
				desc.pixelType,
				clipboardData);
			GLERRLOG
			glFlush();
			GLERRLOG
			
			++faceCount;
		}
		
		CGContextRelease(ctx);
	}
	else	{
		cout << "\tERR: bailing, couldn't create CGContext, " << __FUNCTION__ << endl;
		returnMe = nullptr;
	}
	
	glBindTexture(desc.target, 0);
	GLERRLOG
//#if !defined(VVGL_SDK_IOS) && !defined(VVGL_SDK_RPI)
	if (inPoolRef->getContext()->version <= GLVersion_2)	{
		glDisable(desc.target);
		GLERRLOG
	}
//#endif
	
	free(clipboardData);
	
	return returnMe;
//#endif	/*	VVGL_SDK_RPI	*/
}
void CGBitmapContextUnpremultiply(CGContextRef ctx)	{
	Size				actualSize = Size(CGBitmapContextGetWidth(ctx), CGBitmapContextGetHeight(ctx));
	unsigned long		bytesPerRow = CGBitmapContextGetBytesPerRow(ctx);
	unsigned char		*bitmapData = (unsigned char *)CGBitmapContextGetData(ctx);
	unsigned char		*pixelPtr = nil;
	double				colors[4];
	if (bitmapData==nil || bytesPerRow<=0 || actualSize.width<1 || actualSize.height<1)
		return;
	for (int y=0; y<actualSize.height; ++y)	{
		pixelPtr = bitmapData + (y * bytesPerRow);
		for (int x=0; x<actualSize.width; ++x)	{
			//	convert unsigned chars to normalized doubles
			for (int i=0; i<4; ++i)
				colors[i] = ((double)*(pixelPtr+i))/255.;
			//	unpremultiply if there's an alpha and it won't cause a divide-by-zero
			if (colors[3]>0. && colors[3]<1.)	{
				for (int i=0; i<3; ++i)
					colors[i] = colors[i] / colors[3];
			}
			//	convert the normalized components back into unsigned chars
			for (int i=0; i<4; ++i)
				*(pixelPtr+i) = (unsigned char)(colors[i]*255.);
			
			//	don't forget to increment the pixel ptr!
			pixelPtr += 4;
		}
	}
}
#endif	//	VVGL_SDK_MAC || VVGL_SDK_IOS


#if defined(VVGL_SDK_MAC)
GLBufferRef CreateBufferForCVGLTex(CVOpenGLTextureRef & inTexRef, const bool & createInCurrentContext, const GLBufferPoolRef & inPoolRef)
#elif defined(VVGL_SDK_IOS)
GLBufferRef CreateBufferForCVGLTex(CVOpenGLESTextureRef & inTexRef, const bool & createInCurrentContext, const GLBufferPoolRef & inPoolRef)
#endif
#if defined(VVGL_SDK_MAC) || defined(VVGL_SDK_IOS)
{
	//cout << __PRETTY_FUNCTION__ << " ... " << CVOpenGLTextureGetName(inTexRef) << endl;
	if (inTexRef == NULL)	{
		//NSLog(@"\t\terr: passed nil tex %s",__func__);
		cout << "\terr: passed nil tex in " << __PRETTY_FUNCTION__ << endl;
		return nullptr;
	}
#if !defined(VVGL_SDK_IOS)
	GLuint			texName = CVOpenGLTextureGetName(inTexRef);
#else	//	NOT !VVGL_SDK_IOS
	GLuint			texName = CVOpenGLESTextureGetName(inTexRef);
#endif	//	!VVGL_SDK_IOS
	if (texName <= 0)	{
		//NSLog(@"\t\terr: passed invalid tex num %s",__func__);
		cout << "\terr: passed invalid tex num " << __PRETTY_FUNCTION__ << endl;
		return nil;
	}
#if !defined(VVGL_SDK_IOS)
	if (CFGetTypeID(inTexRef) != CVOpenGLTextureGetTypeID())
#else	//	NOT !VVGL_SDK_IOS
	if (CFGetTypeID(inTexRef) != CVOpenGLESTextureGetTypeID())
#endif	//	!VVGL_SDK_IOS
	{
		//NSLog(@"\t\terr: CFTypeID of passed tex doesn't match expected %s",__func__);
		cout << "\terr: CFTypeID of passed tex doesnt match expected " << __PRETTY_FUNCTION__ << endl;
		return nil;
	}
	
	//VVBuffer			*returnMe = [[VVBuffer alloc] initWithPool:self];
	GLBufferRef		returnMe = make_shared<GLBuffer>(inPoolRef);
	//[VVBufferPool timestampThisBuffer:returnMe];
	GetGlobalBufferPool()->timestampThisBuffer(returnMe);
	//GLBuffer::Descriptor	&desc = returnMe->desc;
	//if (desc == nil)	{
	//	VVRELEASE(returnMe);
	//	return nil;
	//}
	GLBuffer::Descriptor	&desc = returnMe->desc;
	//desc->type = VVBufferType_Tex;
	desc.type = GLBuffer::Type_Tex;
#if !defined(VVGL_SDK_IOS)
	//desc->target = CVOpenGLTextureGetTarget(inTexRef);
	desc.target = (VVGL::GLBuffer::Target)CVOpenGLTextureGetTarget(inTexRef);
	//desc->internalFormat = VVBufferIF_RGBA8;
	desc.internalFormat = GLBuffer::IF_RGBA8;
	//desc->pixelFormat = VVBufferPF_BGRA;
	desc.pixelFormat = GLBuffer::PF_BGRA;
	//desc->pixelType = VVBufferPT_U_Int_8888_Rev;
	desc.pixelType = GLBuffer::PT_UInt_8888_Rev;
#else	//	NOT !VVGL_SDK_IOS
	//desc->target = CVOpenGLESTextureGetTarget(inTexRef);
	desc.target = (VVGL::GLBuffer::Target)CVOpenGLESTextureGetTarget(inTexRef);
	//desc->internalFormat = VVBufferIF_RGBA;
	desc.internalFormat = GLBuffer::IF_RGBA;
	//desc->pixelFormat = VVBufferPF_BGRA;
	desc.pixelFormat = GLBuffer::PF_BGRA;
	//desc->pixelType = VVBufferPT_U_Byte;
	desc.pixelType = GLBuffer::PT_UByte;
#endif	//	!VVGL_SDK_IOS
	//desc->cpuBackingType = VVBufferCPUBack_None;
	desc.cpuBackingType = GLBuffer::Backing_None;
	//desc->gpuBackingType = VVBufferGPUBack_External;
	desc.gpuBackingType = GLBuffer::Backing_External;
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
#if !defined(VVGL_SDK_IOS)
	//[returnMe setFlipped:CVOpenGLTextureIsFlipped(inTexRef)];
	returnMe->flipped = (CVOpenGLTextureIsFlipped(inTexRef)) ? true : false;
#else	//	NOT !VVGL_SDK_IOS
	//[returnMe setFlipped:CVOpenGLESTextureIsFlipped(inTexRef)];
	returnMe->flipped = (CVOpenGLESTextureIsFlipped(inTexRef)) ? true : false;
#endif	//	!VVGL_SDK_IOS
	//[returnMe setBackingSize:[returnMe size]];
	returnMe->backingSize = returnMe->size;
	
	//[returnMe setBackingID:VVBufferBackID_CVTex];
	returnMe->backingID = GLBuffer::BackingID_CVTex;
#if !defined(VVGL_SDK_IOS)
	CVOpenGLTextureRetain(inTexRef);
#else	//	NOT !VVGL_SDK_IOS
	//CVOpenGLESTextureRetain(inTexRef);
	CVBufferRetain(inTexRef);
#endif	//	!VVGL_SDK_IOS
	//[returnMe setBackingReleaseCallback:VVBuffer_ReleaseCVGLT];
#if !defined(VVGL_SDK_IOS)
	returnMe->backingReleaseCallback = [](GLBuffer& inBuffer, void* inReleaseContext)	{
		CVOpenGLTextureRef	tmpRef = (CVOpenGLTextureRef)inReleaseContext;
		if (tmpRef != NULL)
			CVOpenGLTextureRelease(tmpRef);
	};
#else
	returnMe->backingReleaseCallback = [](GLBuffer& inBuffer, void* inReleaseContext)	{
		CVOpenGLESTextureRef	tmpRef = (CVOpenGLESTextureRef)inReleaseContext;
		if (tmpRef != NULL)
			CFRelease(tmpRef);
	};
#endif
	
	//[returnMe setBackingReleaseCallbackContext:inTexRef];
	returnMe->backingContext = inTexRef;
	return returnMe;
}
#endif	//	VVGL_SDK_MAC || VVGL_SDK_IOS




}
