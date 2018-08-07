#include "GLCPUToTexCopier.hpp"




//	none of this stuff should be available if we're running ES
#if !defined(VVGL_TARGETENV_GLES) && !defined(VVGL_TARGETENV_GLES3)




//	we're using this PATHTYPE define to crudely establish two different paths that have 
//	approximately the same performance characteristics.  path 0 delete-initializes a PBO and then 
//	reserve-initializes it with the passed data directly.  path 1 maps the PBO and then copies the 
//	CPU data to it manually.  path 1 is slightly less efficient.
#define PATHTYPE 0




namespace VVGL
{


using namespace std;




GLCPUToTexCopier::GLCPUToTexCopier()	{
	GLBufferPoolRef		bp = GetGlobalBufferPool();
	if (bp != nullptr)
		queueCtx = bp->getContext();
}
GLCPUToTexCopier::~GLCPUToTexCopier()	{
	clearStream();
}
void GLCPUToTexCopier::clearStream()	{
	lock_guard<recursive_mutex>		lock(queueLock);
	while (cpuQueue.size() > 0)
		cpuQueue.pop();
	while (pboQueue.size() > 0)
		pboQueue.pop();
	while (texQueue.size() > 0)
		texQueue.pop();
}
void GLCPUToTexCopier::setQueueSize(const int & inNewQueueSize)	{
	lock_guard<recursive_mutex>		lock(queueLock);
	
	queueSize = inNewQueueSize;
	if (queueSize < 0)
		queueSize = 0;
	
	while (static_cast<int>(cpuQueue.size()) > queueSize)
		cpuQueue.pop();
	while (static_cast<int>(pboQueue.size()) > queueSize)
		pboQueue.pop();
	while (static_cast<int>(texQueue.size()) > queueSize)
		texQueue.pop();
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
		size_t		cpuBPR = inCPUBuffer->desc.bytesPerRowForWidth(inCPUBuffer->size.width);
		size_t		pboBPR = inPBOBuffer->desc.bytesPerRowForWidth(inPBOBuffer->size.width);
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
			memcpy(wPtr, rPtr, pboBPR * inPBOBuffer->size.height);
		}
		//	unmap the PBO
		glUnmapBuffer(inPBOBuffer->desc.target);
		GLERRLOG
		inPBOBuffer->pboMapped = false;
		inPBOBuffer->cpuBackingPtr = nullptr;
	}
	
	glBindBuffer(inPBOBuffer->desc.target, 0);
	GLERRLOG
#endif	//	PATHTYPE==1

}
void GLCPUToTexCopier::_finishProcessing(const GLBufferRef & inCPUBuffer, const GLBufferRef & inPBOBuffer, const GLBufferRef & inTexBuffer)	{
	/*
	if (inCPUBuffer==nullptr || inPBOBuffer==nullptr || inTexBuffer==nullptr)
		return;
	GLBufferPoolRef		bp = GetGlobalBufferPool();
	if (bp == nullptr)
		return;
	bp->timestampThisBuffer(inTexBuffer);
	*/
	
	
	
	
	
	if (inCPUBuffer==nullptr || inPBOBuffer==nullptr || inTexBuffer==nullptr)
		return;
	
	//	bind the PBO and texture
	glBindBuffer(inPBOBuffer->desc.target, inPBOBuffer->name);
	GLERRLOG
	
	glEnable(inTexBuffer->desc.target);
	GLERRLOG
	glBindTexture(inTexBuffer->desc.target, inTexBuffer->name);
	GLERRLOG
	
	//	set up some pixel transfer modes
	glPixelStorei(GL_UNPACK_ROW_LENGTH, inCPUBuffer->size.width);
	GLERRLOG
	glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_TRUE);
	GLERRLOG
	
	//	start copying the buffer data from the PBO to the texture
	glTexSubImage2D(inTexBuffer->desc.target,
		0,
		0,
		0,
		inCPUBuffer->srcRect.size.width,
		inCPUBuffer->srcRect.size.height,
		inTexBuffer->desc.pixelFormat, 
		inTexBuffer->desc.pixelType,
		0);
	GLERRLOG
	
	//	tear down pixel transfer modes
	glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_FALSE);
	GLERRLOG
	
	//	unbind the PBO and texture
	glBindTexture(inTexBuffer->desc.target, 0);
	GLERRLOG
	glDisable(inTexBuffer->desc.target);
	GLERRLOG
	
	glBindBuffer(inPBOBuffer->desc.target, 0);
	GLERRLOG
	
	//	timestamp the buffer...
	GLBufferPoolRef		bp = GetGlobalBufferPool();
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
	
	lock_guard<recursive_mutex>		lock(queueLock);
	
	//	make the queue context current if appropriate- otherwise we are to assume that a GL context is current in this thread
	if (!createInCurrentContext)
		queueCtx->makeCurrentIfNotCurrent();
	
	Size		cpuBufferDims = inCPUBuffer->size;
	
	//	create a PBO and a texture for the CPU buffer
	GLBufferRef		inPBOBuffer = nullptr;
	GLBufferRef		inTexBuffer = nullptr;
	switch (inCPUBuffer->desc.pixelFormat)	{
	case GLBuffer::PF_RGBA:
		inPBOBuffer = CreateRGBAPBO(
			GLBuffer::Target_PBOUnpack,
			GL_STATIC_DRAW,
			cpuBufferDims,
#if PATHTYPE==0
			inCPUBuffer->cpuBackingPtr,	//	this will initialize the buffer with the provided backing
#elif PATHTYPE==1
			NULL,	//	this will delete-initialize the buffer
#endif
			createInCurrentContext);
		inTexBuffer = CreateRGBATex(inCPUBuffer->srcRect.size, createInCurrentContext);
		break;
	case GLBuffer::PF_BGRA:
		inPBOBuffer = CreateBGRAPBO(
			GLBuffer::Target_PBOUnpack,
			GL_STATIC_DRAW,
			cpuBufferDims,
#if PATHTYPE==0
			inCPUBuffer->cpuBackingPtr,	//	this will initialize the buffer with the provided backing
#elif PATHTYPE==1
			NULL,	//	this will delete-initialize the buffer
#endif
			createInCurrentContext);
		inTexBuffer = CreateBGRATex(inCPUBuffer->srcRect.size, createInCurrentContext);
		break;
	case GLBuffer::PF_YCbCr_422:
		inPBOBuffer = CreateYCbCrPBO(
			GLBuffer::Target_PBOUnpack,
			GL_STATIC_DRAW,
			cpuBufferDims,
#if PATHTYPE==0
			inCPUBuffer->cpuBackingPtr,	//	this will initialize the buffer with the provided backing
#elif PATHTYPE==1
			NULL,	//	this will delete-initialize the buffer
#endif
			createInCurrentContext);
		inTexBuffer = CreateRGBATex(inCPUBuffer->srcRect.size, createInCurrentContext);
		break;
	default:
		break;
	}
	
	if (inPBOBuffer==nullptr || inTexBuffer==nullptr)
		return nullptr;
	
	_beginProcessing(inCPUBuffer, inPBOBuffer, inTexBuffer);
	_finishProcessing(inCPUBuffer, inPBOBuffer, inTexBuffer);
	
	return inTexBuffer;
}
GLBufferRef GLCPUToTexCopier::streamCPUToTex(const GLBufferRef & inCPUBuffer, const bool & createInCurrentContext)	{
	//cout << __FUNCTION__ << endl;
	
	lock_guard<recursive_mutex>		lock(queueLock);
	
	//	make the queue context current if appropriate- otherwise we are to assume that a GL context is current in this thread
	if (!createInCurrentContext)
		queueCtx->makeCurrentIfNotCurrent();
	
	//	make sure the queues have the appropriate and expected number of elements
	int			tmpQueueSize = (int)cpuQueue.size();
	if (tmpQueueSize != (int)pboQueue.size() || tmpQueueSize != (int)texQueue.size())	{
		cout << "\tERR: queue size discrepancy, " << __PRETTY_FUNCTION__ << endl;
		return nullptr;
	}
	
	Size		cpuBufferDims = (inCPUBuffer==nullptr) ? Size() : inCPUBuffer->size;
	bool		safeToPush = false;
	bool		safeToPop = false;
	//	we're safe to push if the queue isn't too large AND there's a non-null input buffer
	if (tmpQueueSize<=queueSize && inCPUBuffer!=nullptr)
		safeToPush = true;
	//	we're safe to pop a val if the queue is too large AND if we're safe to push
	if (tmpQueueSize>=queueSize && safeToPush)
		safeToPop = true;
	
	//	if we're safe to push, we need to create a PBO and a texture for the CPU buffer
	GLBufferRef		inPBOBuffer = nullptr;
	GLBufferRef		inTexBuffer = nullptr;
	if (safeToPush)	{
		switch (inCPUBuffer->desc.pixelFormat)	{
		case GLBuffer::PF_RGBA:
			inPBOBuffer = CreateRGBAPBO(
				GLBuffer::Target_PBOUnpack,
				GL_STATIC_DRAW,
				cpuBufferDims,
#if PATHTYPE==0
				inCPUBuffer->cpuBackingPtr,	//	this will initialize the buffer with the provided backing
#elif PATHTYPE==1
				NULL,	//	this will delete-initialize the buffer
#endif
				createInCurrentContext);
			inTexBuffer = CreateRGBATex(inCPUBuffer->srcRect.size, createInCurrentContext);
			break;
		case GLBuffer::PF_BGRA:
			inPBOBuffer = CreateBGRAPBO(
				GLBuffer::Target_PBOUnpack,
				GL_STATIC_DRAW,
				cpuBufferDims,
#if PATHTYPE==0
				inCPUBuffer->cpuBackingPtr,	//	this will initialize the buffer with the provided backing
#elif PATHTYPE==1
				NULL,	//	this will delete-initialize the buffer
#endif
				createInCurrentContext);
			inTexBuffer = CreateBGRATex(inCPUBuffer->srcRect.size, createInCurrentContext);
			break;
		case GLBuffer::PF_YCbCr_422:
			inPBOBuffer = CreateYCbCrPBO(
				GLBuffer::Target_PBOUnpack,
				GL_STATIC_DRAW,
				cpuBufferDims,
#if PATHTYPE==0
				inCPUBuffer->cpuBackingPtr,	//	this will initialize the buffer with the provided backing
#elif PATHTYPE==1
				NULL,	//	this will delete-initialize the buffer
#endif
				createInCurrentContext);
			inTexBuffer = CreateRGBATex(inCPUBuffer->srcRect.size, createInCurrentContext);
			break;
		default:
			break;
		}
		
		//	if we couldn't create the buffers we need then we're not safe to push, and if we're not safe to push then we're not safe to pop.
		if (inPBOBuffer==nullptr || inTexBuffer==nullptr)	{
			cout << "\tERR: couldnt make PBO or tex, " << __PRETTY_FUNCTION__ << endl;
			safeToPush = false;
			safeToPop = false;
		}
	}
	
	GLBufferRef			returnMe = nullptr;
	
	//	pop buffers off the queues if appropriate
	if (safeToPop)	{
		GLBufferRef		outCPUBuffer = cpuQueue.front();
		cpuQueue.pop();
		GLBufferRef		outPBOBuffer = pboQueue.front();
		pboQueue.pop();
		returnMe = texQueue.front();
		texQueue.pop();
		_finishProcessing(outCPUBuffer, outPBOBuffer, returnMe);
	}
	//	push the buffer if appropriate
	if (safeToPush)	{
		cpuQueue.push(inCPUBuffer);
		pboQueue.push(inPBOBuffer);
		texQueue.push(inTexBuffer);
		_beginProcessing(inCPUBuffer, inPBOBuffer, inTexBuffer);
	}
	
	return returnMe;
}



}



#endif	//	!defined(VVGL_TARGETENV_GLES) && !defined(VVGL_TARGETENV_GLES3)
