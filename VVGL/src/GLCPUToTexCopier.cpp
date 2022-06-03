#include "GLCPUToTexCopier.hpp"
#include <cstring>



//	none of this stuff should be available if we're running ES
#if !defined(VVGL_TARGETENV_GLES) && !defined(VVGL_TARGETENV_GLES3)




//	we're using this PATHTYPE define to crudely establish two different paths that have 
//	approximately the same performance characteristics.  path 0 delete-initializes a PBO and then 
//	reserve-initializes it with the passed data directly.  path 1 maps the PBO and then copies the 
//	CPU data to it manually.  path 1 is slightly faster on the mac i'm writing this on.
#define PATHTYPE 1




namespace VVGL
{


using namespace std;




GLCPUToTexCopier::GLCPUToTexCopier()	{
	GLBufferPoolRef		bp = (_privatePool==nullptr) ? GetGlobalBufferPool() : _privatePool;
	if (bp != nullptr)
		_queueCtx = bp->context()->newContextSharingMe();
}
GLCPUToTexCopier::~GLCPUToTexCopier()	{
	clearStream();
}
void GLCPUToTexCopier::clearStream()	{
	lock_guard<recursive_mutex>		lock(_queueLock);
	while (_cpuQueue.size() > 0)
		_cpuQueue.pop();
	while (_pboQueue.size() > 0)
		_pboQueue.pop();
	while (_texQueue.size() > 0)
		_texQueue.pop();
}
void GLCPUToTexCopier::setQueueSize(const int & inNewQueueSize)	{
	lock_guard<recursive_mutex>		lock(_queueLock);
	
	_queueSize = inNewQueueSize;
	if (_queueSize < 0)
		_queueSize = 0;
	
	while (static_cast<int>(_cpuQueue.size()) > _queueSize)
		_cpuQueue.pop();
	while (static_cast<int>(_pboQueue.size()) > _queueSize)
		_pboQueue.pop();
	while (static_cast<int>(_texQueue.size()) > _queueSize)
		_texQueue.pop();
}


void GLCPUToTexCopier::_beginProcessing(const GLBufferRef & inCPUBuffer, const GLBufferRef & inPBOBuffer, const GLBufferRef & inTexBuffer)	{

#if PATHTYPE==0
	//	intentionally blank
	if (inCPUBuffer==nullptr || inPBOBuffer==nullptr || inTexBuffer==nullptr)
		return;
#elif PATHTYPE==1
	if (inCPUBuffer==nullptr || inPBOBuffer==nullptr || inTexBuffer==nullptr)
		return;
	//	bind the PBO
	glBindBuffer(inPBOBuffer->desc.target, inPBOBuffer->name);
	GLERRLOG
	//	map the PBO- this should return immediately, provided that we discard-initialized the PBO just before this
	inPBOBuffer->cpuBackingPtr = glMapBuffer(inPBOBuffer->desc.target, GL_WRITE_ONLY);
	GLERRLOG
	inPBOBuffer->pboMapped = (inPBOBuffer->cpuBackingPtr != NULL) ? true : false;
	if (!inPBOBuffer->pboMapped)
		cout << "\tERR: couldnt map PBO in " << __PRETTY_FUNCTION__ << endl;
	else	{
		//	copy the data from the cpu buffer to the PBO
		size_t		cpuBPR = inCPUBuffer->desc.bytesPerRowForWidth(static_cast<uint32_t>(inCPUBuffer->size.width));
		size_t		pboBPR = inPBOBuffer->desc.bytesPerRowForWidth(static_cast<uint32_t>(inPBOBuffer->size.width));
		uint8_t		*rPtr = (uint8_t*)inCPUBuffer->cpuBackingPtr;
		uint8_t		*wPtr = (uint8_t*)inPBOBuffer->cpuBackingPtr;

		//	if the cpu buffer's bytes per row differs from the PBO's bytes per row, we have to copy one row at a time
		if (cpuBPR != pboBPR)	{
			size_t		copyBytesPerRow = (cpuBPR<pboBPR) ? cpuBPR : pboBPR;
			for (int i=0; i<inPBOBuffer->size.height; ++i)	{
				memcpy(wPtr, rPtr, copyBytesPerRow);
				wPtr += cpuBPR;
				rPtr += pboBPR;
			}
		}
		//	else the CPU buffer and the PBO have the same exact number of bytes per row- we can copy their contents in a single call
		else	{
			memcpy(wPtr, rPtr, pboBPR * static_cast<size_t>(round(inPBOBuffer->size.height)));
		}
		//	unmap the PBO
		glUnmapBuffer(inPBOBuffer->desc.target);
		GLERRLOG
		inPBOBuffer->pboMapped = false;
		inPBOBuffer->cpuBackingPtr = nullptr;
	}
	
	glBindBuffer(inPBOBuffer->desc.target, 0);
	GLERRLOG
	
	glFlush();
	GLERRLOG
#endif	//	PATHTYPE==1

}
void GLCPUToTexCopier::_finishProcessing(const GLBufferRef & inCPUBuffer, const GLBufferRef & inPBOBuffer, const GLBufferRef & inTexBuffer)	{
	/*
	if (inCPUBuffer==nullptr || inPBOBuffer==nullptr || inTexBuffer==nullptr)
		return;
	GLBufferPoolRef		bp = (_privatePool==nullptr) ? GetGlobalBufferPool() : _privatePool;
	if (bp == nullptr)
		return;
	bp->timestampThisBuffer(inTexBuffer);
	*/
	
	
	
	
	
	if (inCPUBuffer==nullptr || inPBOBuffer==nullptr || inTexBuffer==nullptr)
		return;
	
	GLVersion		myVers = _queueCtx->version;
	//	bind the PBO and texture
	glBindBuffer(inPBOBuffer->desc.target, inPBOBuffer->name);
	GLERRLOG
	
	if (myVers==GLVersion_2)	{
		glEnable(inTexBuffer->desc.target);
		GLERRLOG
	}
	glBindTexture(inTexBuffer->desc.target, inTexBuffer->name);
	GLERRLOG
	
	//	set up some pixel transfer modes
	glPixelStorei(GL_UNPACK_ROW_LENGTH, static_cast<GLint>(inCPUBuffer->size.width));
	GLERRLOG
#if !defined(Q_OS_WIN)
	glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_TRUE);
	GLERRLOG
#endif
	glPixelStorei(GL_UNPACK_SWAP_BYTES, (_swapBytes) ? GL_TRUE : GL_FALSE);
	GLERRLOG
	
	//	start copying the buffer data from the PBO to the texture
	glTexSubImage2D(inTexBuffer->desc.target,
		0,
		0,
		0,
		static_cast<GLsizei>(inCPUBuffer->srcRect.size.width),
		static_cast<GLsizei>(inCPUBuffer->srcRect.size.height),
		inTexBuffer->desc.pixelFormat, 
		inTexBuffer->desc.pixelType,
		0);
	GLERRLOG
	
	//	tear down pixel transfer modes
	glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
	GLERRLOG
#if !defined(Q_OS_WIN)
	glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_FALSE);
	GLERRLOG
#endif
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	GLERRLOG
	
	//	unbind the PBO and texture
	glBindTexture(inTexBuffer->desc.target, 0);
	GLERRLOG
	if (myVers==GLVersion_2)	{
		glDisable(inTexBuffer->desc.target);
		GLERRLOG
	}
	
	glBindBuffer(inPBOBuffer->desc.target, 0);
	GLERRLOG
	//	flush- start the DMA transfer.  the CPU doesn't wait for this to complete, and returns immediately.
	glFlush();
	GLERRLOG
	
	//	timestamp the buffer...
	GLBufferPoolRef		bp = (_privatePool==nullptr) ? GetGlobalBufferPool() : _privatePool;
	if (bp == nullptr)
		return;
	bp->timestampThisBuffer(inTexBuffer);
	
	//	make sure the buffers inherit the source's flippedness and timestamp
	inPBOBuffer->flipped = inCPUBuffer->flipped;
	inPBOBuffer->contentTimestamp = inCPUBuffer->contentTimestamp;
	inTexBuffer->flipped = inCPUBuffer->flipped;
	inTexBuffer->contentTimestamp = inCPUBuffer->contentTimestamp;
}


GLBufferRef GLCPUToTexCopier::uploadCPUToTex(const GLBufferRef & inCPUBuffer, const bool & createInCurrentContext)	{
	if (inCPUBuffer == nullptr)
		return nullptr;
	
	GLBufferRef		texBuffer = nullptr;
	{
		lock_guard<recursive_mutex>		lock(_queueLock);
	
		//	make the queue context current if appropriate- otherwise we are to assume that a GL context is current in this thread
		if (!createInCurrentContext)
			_queueCtx->makeCurrentIfNotCurrent();
	
		//	create a PBO and a texture for the CPU buffer
		GLBufferPoolRef		bp = (_privatePool!=nullptr) ? _privatePool : GetGlobalBufferPool();
		switch (inCPUBuffer->desc.pixelFormat)	{
		case GLBuffer::PF_RGBA:
			if (inCPUBuffer->desc.pixelType == GLBuffer::PT_Float)	{
				texBuffer = CreateRGBAFloatTex(inCPUBuffer->srcRect.size, createInCurrentContext, bp);
			}
			else	{
				texBuffer = CreateRGBATex(inCPUBuffer->srcRect.size, createInCurrentContext, bp);
			}
			break;
		case GLBuffer::PF_BGRA:
			if (inCPUBuffer->desc.pixelType == GLBuffer::PT_Float)	{
				texBuffer = CreateBGRAFloatTex(inCPUBuffer->srcRect.size, createInCurrentContext, bp);
			}
			else	{
				texBuffer = CreateBGRATex(inCPUBuffer->srcRect.size, createInCurrentContext, bp);
			}
			break;
		case GLBuffer::PF_YCbCr_422:
			texBuffer = CreateRGBATex(inCPUBuffer->srcRect.size, createInCurrentContext, bp);
			break;
		default:
			break;
		}
	}
	
	if (texBuffer==nullptr)
		return nullptr;
	
	return uploadCPUToTex(inCPUBuffer, texBuffer, createInCurrentContext);
}
GLBufferRef GLCPUToTexCopier::uploadCPUToTex(const GLBufferRef & inCPUBuffer, const GLBufferRef & inTexBuffer, const bool & createInCurrentContext)	{
	if (inCPUBuffer == nullptr || inTexBuffer == nullptr)
		return nullptr;
	
	lock_guard<recursive_mutex>		lock(_queueLock);
	
	GLBufferPoolRef		bp = (_privatePool!=nullptr) ? _privatePool : GetGlobalBufferPool();
	//	make the queue context current if appropriate- otherwise we are to assume that a GL context is current in this thread
	if (!createInCurrentContext)
		_queueCtx->makeCurrentIfNotCurrent();
	
	Size		cpuBufferDims = inCPUBuffer->size;
	
	//	create a PBO and a texture for the CPU buffer
	GLBufferRef		pboBuffer = nullptr;
	switch (inCPUBuffer->desc.pixelFormat)	{
	case GLBuffer::PF_RGBA:
		if (inCPUBuffer->desc.pixelType == GLBuffer::PT_Float)	{
			pboBuffer = CreateRGBAFloatPBO(
				GLBuffer::Target_PBOUnpack,
				GL_STREAM_DRAW,
				cpuBufferDims,
#if PATHTYPE==0
				inCPUBuffer->cpuBackingPtr,	//	this will initialize the buffer with the provided backing
#elif PATHTYPE==1
				NULL,	//	this will delete-initialize the buffer
#endif
				createInCurrentContext,
				bp);
		}
		else	{
			pboBuffer = CreateRGBAPBO(
				GLBuffer::Target_PBOUnpack,
				GL_STREAM_DRAW,
				cpuBufferDims,
#if PATHTYPE==0
				inCPUBuffer->cpuBackingPtr,	//	this will initialize the buffer with the provided backing
#elif PATHTYPE==1
				NULL,	//	this will delete-initialize the buffer
#endif
				createInCurrentContext,
				bp);
		}
		break;
	case GLBuffer::PF_BGRA:
		if (inCPUBuffer->desc.pixelType == GLBuffer::PT_Float)	{
			pboBuffer = CreateBGRAFloatPBO(
				GLBuffer::Target_PBOUnpack,
				GL_STREAM_DRAW,
				cpuBufferDims,
#if PATHTYPE==0
				inCPUBuffer->cpuBackingPtr,	//	this will initialize the buffer with the provided backing
#elif PATHTYPE==1
				NULL,	//	this will delete-initialize the buffer
#endif
				createInCurrentContext,
				bp);
		}
		else	{
			pboBuffer = CreateBGRAPBO(
				GLBuffer::Target_PBOUnpack,
				GL_STREAM_DRAW,
				cpuBufferDims,
#if PATHTYPE==0
				inCPUBuffer->cpuBackingPtr,	//	this will initialize the buffer with the provided backing
#elif PATHTYPE==1
				NULL,	//	this will delete-initialize the buffer
#endif
				createInCurrentContext,
				bp);
		}
		break;
	case GLBuffer::PF_YCbCr_422:
		pboBuffer = CreateYCbCrPBO(
			GLBuffer::Target_PBOUnpack,
			GL_STREAM_DRAW,
			cpuBufferDims,
#if PATHTYPE==0
			inCPUBuffer->cpuBackingPtr,	//	this will initialize the buffer with the provided backing
#elif PATHTYPE==1
			NULL,	//	this will delete-initialize the buffer
#endif
			createInCurrentContext,
			bp);
		break;
	default:
		break;
	}
	
	if (pboBuffer==nullptr)
		return nullptr;
	
	_beginProcessing(inCPUBuffer, pboBuffer, inTexBuffer);
	_finishProcessing(inCPUBuffer, pboBuffer, inTexBuffer);
	
	return inTexBuffer;
}
GLBufferRef GLCPUToTexCopier::streamCPUToTex(const GLBufferRef & inCPUBuffer, const bool & createInCurrentContext)	{
	//cout << __FUNCTION__ << endl;
	
	lock_guard<recursive_mutex>		lock(_queueLock);
	
	GLBufferPoolRef		bp = (_privatePool!=nullptr) ? _privatePool : GetGlobalBufferPool();
	//	make the queue context current if appropriate- otherwise we are to assume that a GL context is current in this thread
	if (!createInCurrentContext)
		_queueCtx->makeCurrentIfNotCurrent();
	
	//	make sure the queues have the appropriate and expected number of elements
	int			tmpQueueSize = (int)_cpuQueue.size();
	if (tmpQueueSize != (int)_pboQueue.size() || tmpQueueSize != (int)_texQueue.size())	{
		cout << "\tERR: queue size discrepancy, " << __PRETTY_FUNCTION__ << endl;
		return nullptr;
	}
	
	bool		safeToPush = false;
	bool		safeToPop = false;
	//	we're safe to push if the queue isn't too large AND there's a non-null input buffer
	if (tmpQueueSize<=_queueSize && inCPUBuffer!=nullptr)
		safeToPush = true;
	//	we're safe to pop a val if the queue is too large AND if we're safe to push
	if (tmpQueueSize>=_queueSize && safeToPush)
		safeToPop = true;
	
	//	if we're safe to push, we need to create a PBO and a texture for the CPU buffer
	GLBufferRef		inTexBuffer = nullptr;
	if (safeToPush)	{
		switch (inCPUBuffer->desc.pixelFormat)	{
		case GLBuffer::PF_RGBA:
			if (inCPUBuffer->desc.pixelType == GLBuffer::PT_Float)	{
				inTexBuffer = CreateRGBAFloatTex(inCPUBuffer->srcRect.size, createInCurrentContext, bp);
			}
			else	{
				inTexBuffer = CreateRGBATex(inCPUBuffer->srcRect.size, createInCurrentContext, bp);
			}
			break;
		case GLBuffer::PF_BGRA:
			if (inCPUBuffer->desc.pixelType == GLBuffer::PT_Float)	{
				inTexBuffer = CreateBGRAFloatTex(inCPUBuffer->srcRect.size, createInCurrentContext, bp);
			}
			else	{
				inTexBuffer = CreateBGRATex(inCPUBuffer->srcRect.size, createInCurrentContext, bp);
			}
			break;
		case GLBuffer::PF_YCbCr_422:
			inTexBuffer = CreateYCbCrTex(inCPUBuffer->srcRect.size, createInCurrentContext, bp);
			break;
		default:
			break;
		}
		
		//	if we couldn't create the buffers we need then we're not safe to push, and if we're not safe to push then we're not safe to pop.
		if (inTexBuffer==nullptr)	{
			cout << "\tERR: couldnt make tex, " << __PRETTY_FUNCTION__ << endl;
			safeToPush = false;
			safeToPop = false;
			return nullptr;
		}
	}
	
	return streamCPUToTex(inCPUBuffer, inTexBuffer, createInCurrentContext);
}
GLBufferRef GLCPUToTexCopier::streamCPUToTex(const GLBufferRef & inCPUBuffer, const GLBufferRef & inTexBuffer, const bool & createInCurrentContext)	{
	if (inCPUBuffer==nullptr || inTexBuffer==nullptr)
		return nullptr;
	//cout << __FUNCTION__ << "... " << *inCPUBuffer << " -> " << *inTexBuffer << endl;
	
	lock_guard<recursive_mutex>		lock(_queueLock);
	
	GLBufferPoolRef		bp = (_privatePool!=nullptr) ? _privatePool : GetGlobalBufferPool();
	//	make the queue context current if appropriate- otherwise we are to assume that a GL context is current in this thread
	if (!createInCurrentContext)
		_queueCtx->makeCurrentIfNotCurrent();
	
	//	make sure the queues have the appropriate and expected number of elements
	int			tmpQueueSize = (int)_cpuQueue.size();
	if (tmpQueueSize != (int)_pboQueue.size() || tmpQueueSize != (int)_texQueue.size())	{
		cout << "\tERR: queue size discrepancy, " << __PRETTY_FUNCTION__ << endl;
		return nullptr;
	}
	
	Size		cpuBufferDims = (inCPUBuffer==nullptr) ? Size() : inCPUBuffer->size;
	bool		safeToPush = false;
	bool		safeToPop = false;
	//	we're safe to push if the queue isn't too large AND there's a non-null input buffer
	if (tmpQueueSize<=_queueSize && inCPUBuffer!=nullptr)
		safeToPush = true;
	//	we're safe to pop a val if the queue is too large AND if we're safe to push
	if (tmpQueueSize>=_queueSize && safeToPush)
		safeToPop = true;
	
	//	if we're safe to push, we need to create a PBO and a texture for the CPU buffer
	GLBufferRef		inPBOBuffer = nullptr;
	if (safeToPush)	{
		switch (inCPUBuffer->desc.pixelFormat)	{
		case GLBuffer::PF_RGBA:
			if (inCPUBuffer->desc.pixelType == GLBuffer::PT_Float)	{
				inPBOBuffer = CreateRGBAFloatPBO(
					GLBuffer::Target_PBOUnpack,
					GL_STREAM_DRAW,
					cpuBufferDims,
#if PATHTYPE==0
					inCPUBuffer->cpuBackingPtr,	//	this will initialize the buffer with the provided backing
#elif PATHTYPE==1
					NULL,	//	this will delete-initialize the buffer
#endif
					createInCurrentContext,
					bp);
			}
			else	{
				inPBOBuffer = CreateRGBAPBO(
					GLBuffer::Target_PBOUnpack,
					GL_STREAM_DRAW,
					cpuBufferDims,
#if PATHTYPE==0
					inCPUBuffer->cpuBackingPtr,	//	this will initialize the buffer with the provided backing
#elif PATHTYPE==1
					NULL,	//	this will delete-initialize the buffer
#endif
					createInCurrentContext,
					bp);
			}
			break;
		case GLBuffer::PF_BGRA:
			if (inCPUBuffer->desc.pixelType == GLBuffer::PT_Float)	{
				inPBOBuffer = CreateBGRAFloatPBO(
					GLBuffer::Target_PBOUnpack,
					GL_STREAM_DRAW,
					cpuBufferDims,
#if PATHTYPE==0
					inCPUBuffer->cpuBackingPtr,	//	this will initialize the buffer with the provided backing
#elif PATHTYPE==1
					NULL,	//	this will delete-initialize the buffer
#endif
					createInCurrentContext,
					bp);
			}
			else	{
				inPBOBuffer = CreateBGRAPBO(
					GLBuffer::Target_PBOUnpack,
					GL_STREAM_DRAW,
					cpuBufferDims,
#if PATHTYPE==0
					inCPUBuffer->cpuBackingPtr,	//	this will initialize the buffer with the provided backing
#elif PATHTYPE==1
					NULL,	//	this will delete-initialize the buffer
#endif
					createInCurrentContext,
					bp);
			}
			break;
		case GLBuffer::PF_YCbCr_422:
			inPBOBuffer = CreateYCbCrPBO(
				GLBuffer::Target_PBOUnpack,
				GL_STREAM_DRAW,
				cpuBufferDims,
#if PATHTYPE==0
				inCPUBuffer->cpuBackingPtr,	//	this will initialize the buffer with the provided backing
#elif PATHTYPE==1
				NULL,	//	this will delete-initialize the buffer
#endif
				createInCurrentContext,
				bp);
			break;
		default:
			break;
		}
		
		//	if we couldn't create the buffers we need then we're not safe to push, and if we're not safe to push then we're not safe to pop.
		if (inPBOBuffer==nullptr)	{
			cout << "\tERR: couldnt make PBO " << __PRETTY_FUNCTION__ << endl;
			safeToPush = false;
			safeToPop = false;
		}
	}
	
	GLBufferRef			returnMe = nullptr;
	
	//	pop buffers off the queues if appropriate
	if (safeToPop)	{
		GLBufferRef		outCPUBuffer = _cpuQueue.front();
		_cpuQueue.pop();
		GLBufferRef		outPBOBuffer = _pboQueue.front();
		_pboQueue.pop();
		returnMe = _texQueue.front();
		_texQueue.pop();
		_finishProcessing(outCPUBuffer, outPBOBuffer, returnMe);
	}
	//	push the buffer if appropriate
	if (safeToPush)	{
		_cpuQueue.push(inCPUBuffer);
		_pboQueue.push(inPBOBuffer);
		_texQueue.push(inTexBuffer);
		_beginProcessing(inCPUBuffer, inPBOBuffer, inTexBuffer);
	}
	
	return returnMe;
}



}



#endif	//	!defined(VVGL_TARGETENV_GLES) && !defined(VVGL_TARGETENV_GLES3)
