#include "GLTexToCPUCopier.hpp"
#include <cstring>



//	none of this stuff should be available if we're running ES
#if !defined(VVGL_TARGETENV_GLES) && !defined(VVGL_TARGETENV_GLES3)




namespace VVGL
{


using namespace std;




GLTexToCPUCopier::GLTexToCPUCopier()	{
	GLBufferPoolRef		bp = GetGlobalBufferPool();
	if (bp != nullptr)
		_queueCtx = bp->context();
}
GLTexToCPUCopier::~GLTexToCPUCopier()	{
	clearStream();
}
void GLTexToCPUCopier::clearStream()	{
	lock_guard<recursive_mutex>		lock(_queueLock);
	while (_cpuQueue.size() > 0)
		_cpuQueue.pop();
	while (_pboQueue.size() > 0)
		_pboQueue.pop();
	while (_texQueue.size() > 0)
		_texQueue.pop();
	while (_fboQueue.size() > 0)
		_fboQueue.pop();
}
void GLTexToCPUCopier::setQueueSize(const int & inNewQueueSize)	{
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


void GLTexToCPUCopier::_beginProcessing(const GLBufferRef & /*inCPUBuffer*/, const GLBufferRef & inPBOBuffer, const GLBufferRef & inTexBuffer, const GLBufferRef & inFBOBuffer)	{
	/*
	cout << __FUNCTION__ << endl;
	if (inCPUBuffer == nullptr)
		cout << "\tcpu is null\n";
	else
		cout << "\tcpu is " << *inCPUBuffer << endl;
	
	if (inPBOBuffer == nullptr)
		cout << "\tpbo is null\n";
	else
		cout << "\tpbo is " << *inPBOBuffer << endl;
	
	if (inTexBuffer == nullptr)
		cout << "\ttex is null\n";
	else
		cout << "\ttex is " << *inTexBuffer << endl;
	*/
	//	the CPU buffer may be null, but not the PBO nor texture
	if (inPBOBuffer==nullptr || inTexBuffer==nullptr)
		return;
	
	
	
	//	bind the framebuffer, attach the texture to it
	glBindFramebuffer(GL_FRAMEBUFFER, inFBOBuffer->name);
	GLERRLOG
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, inTexBuffer->desc.target, inTexBuffer->name, 0);
	GLERRLOG
	
	
	
	
	//	bind the PBO and texture
	glBindBuffer(inPBOBuffer->desc.target, inPBOBuffer->name);
	GLERRLOG
	
	//glEnable(inTexBuffer->desc.target);
	//GLERRLOG
	//glBindTexture(inTexBuffer->desc.target, inTexBuffer->name);
	//GLERRLOG
	
	//	set up some pixel transfer modes
	glPixelStorei(GL_PACK_ROW_LENGTH, static_cast<int>(inPBOBuffer->size.width));
	GLERRLOG
	
	//	start packing the texture data into the pbo
	//glGetTexImage(
	//	inTexBuffer->desc.target,
	//	0,
	//	inTexBuffer->desc.pixelFormat,
	//	inTexBuffer->desc.pixelType,
	//	NULL
	//	);
	//GLERRLOG
	glReadPixels(
		0,
		0,
		static_cast<int>(inPBOBuffer->size.width),
		static_cast<int>(inPBOBuffer->size.height),
		inTexBuffer->desc.pixelFormat,
		inTexBuffer->desc.pixelType,
		nullptr
		);
	GLERRLOG
	
	//	unbind the texture and PBO
	//glBindTexture(inTexBuffer->desc.target, 0);
	//GLERRLOG
	//glDisable(inTexBuffer->desc.target);
	//GLERRLOG
	glBindBuffer(inPBOBuffer->desc.target, 0);
	GLERRLOG
	
	//	flush- this starts the DMA transfer.  the CPU won't wait for this transfer to complete, and will return execution immediately.
	glFlush();
	GLERRLOG
	
	//	unbind the FBO
	/*
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 0, 0, 0);
	GLERRLOG
	*/
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	GLERRLOG
}
void GLTexToCPUCopier::_finishProcessing(const GLBufferRef & inCPUBuffer, const GLBufferRef & inPBOBuffer, const GLBufferRef & inTexBuffer, const GLBufferRef & /*inFBOBuffer*/)	{
	/*
	cout << __FUNCTION__ << endl;
	if (inCPUBuffer == nullptr)
		cout << "\tcpu is null\n";
	else
		cout << "\tcpu is " << *inCPUBuffer << endl;
	
	if (inPBOBuffer == nullptr)
		cout << "\tpbo is null\n";
	else
		cout << "\tpbo is " << *inPBOBuffer << endl;
	
	if (inTexBuffer == nullptr)
		cout << "\ttex is null\n";
	else
		cout << "\ttex is " << *inTexBuffer << endl;
	*/
	if (inPBOBuffer==nullptr || inTexBuffer==nullptr)
		return;
	
	//	try to map the PBO
	inPBOBuffer->mapPBO(GL_READ_ONLY, true);
	//	if we mapped the PBO and got a valid cpu backing...
	if (inPBOBuffer->pboMapped && inPBOBuffer->cpuBackingPtr!=nullptr)	{
		//	if the CPU buffer is non-null, copy the contents of the PBO to the CPU buffer.
		if (inCPUBuffer != nullptr)	{
			size_t		cpuBPR = inCPUBuffer->desc.bytesPerRowForWidth(static_cast<uint32_t>(inCPUBuffer->size.width));
			size_t		pboBPR = inPBOBuffer->desc.bytesPerRowForWidth(static_cast<uint32_t>(inPBOBuffer->size.width));
			uint8_t		*rPtr = static_cast<uint8_t*>(inPBOBuffer->cpuBackingPtr);
			uint8_t		*wPtr = static_cast<uint8_t*>(inCPUBuffer->cpuBackingPtr);
	
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
				memcpy(wPtr, rPtr, pboBPR * static_cast<size_t>(inPBOBuffer->size.height));
			}
	
		}
	}
	
	//	...do not un-map the PBO- leave it mapped (the buffer pool will unmap it before releasing it 
	//	or before it pulls it out again for recycling as long as the ivar 'pboMapped' is set 
	//	appropriately)- this allows you to access and work with the mapped memory in other SDKs, 
	//	potentially allowing you to avoid copying the memory.
	
	//	un-bind the texture from the FBO
	//glBindFramebuffer(GL_FRAMEBUFFER, inFBOBuffer->name);
	//GLERRLOG
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 0, 0, 0);
	//GLERRLOG
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//GLERRLOG
	
	//	make sure the created buffers inherit the texture's flippedness...
	if (inCPUBuffer != nullptr)	{
		inCPUBuffer->flipped = inTexBuffer->flipped;
		inCPUBuffer->contentTimestamp = inTexBuffer->contentTimestamp;
	}
	inPBOBuffer->flipped = inTexBuffer->flipped;
	inPBOBuffer->contentTimestamp = inTexBuffer->contentTimestamp;
}


GLBufferRef GLTexToCPUCopier::downloadTexToCPU(const GLBufferRef & inTexBuffer, const GLBufferRef & inCPUBuffer, const bool & createInCurrentContext)	{
	//cout << __PRETTY_FUNCTION__ << "... " << *inTexBuffer << endl;
	//	bail if there's no texture to download
	if (inTexBuffer == nullptr)
		return nullptr;
	
	lock_guard<recursive_mutex>		lock(_queueLock);
	
	GLBufferPoolRef		bp = (_privatePool!=nullptr) ? _privatePool : GetGlobalBufferPool();
	//	make the queue context current if appropriate- otherwise we are to assume that a GL context is current in this thread
	if (!createInCurrentContext)
		_queueCtx->makeCurrentIfNotCurrent();
	
	//Size			gpuBufferDims = inTexBuffer->srcRect.size;
	
	//	make an FBO
	GLBufferRef			tmpFBO = CreateFBO(createInCurrentContext, bp);
	//	create a PBO for the texture
	GLBufferRef			inPBOBuffer = nullptr;
	switch (inTexBuffer->desc.pixelFormat)	{
	case GLBuffer::PF_RGBA:
		inPBOBuffer = CreateRGBAPBO(GLBuffer::Target_PBOPack, GL_DYNAMIC_READ, inTexBuffer->size, nullptr, createInCurrentContext, bp);
		break;
	case GLBuffer::PF_BGRA:
		inPBOBuffer = CreateBGRAPBO(GLBuffer::Target_PBOPack, GL_DYNAMIC_READ, inTexBuffer->size, nullptr, createInCurrentContext, bp);
		break;
	case GLBuffer::PF_YCbCr_422:
		inPBOBuffer = CreateYCbCrPBO(GLBuffer::Target_PBOPack, GL_DYNAMIC_READ, inTexBuffer->size, nullptr, createInCurrentContext, bp);
		break;
	default:
		break;
	}
	
	if (inPBOBuffer==nullptr)
		return nullptr;
	
	_beginProcessing(inCPUBuffer, inPBOBuffer, inTexBuffer, tmpFBO);
	_finishProcessing(inCPUBuffer, inPBOBuffer, inTexBuffer, tmpFBO);
	
	if (inCPUBuffer != nullptr)
		return inCPUBuffer;
	return inPBOBuffer;
}
GLBufferRef GLTexToCPUCopier::streamTexToCPU(const GLBufferRef & inTexBuffer, const GLBufferRef & inCPUBuffer, const bool & createInCurrentContext)	{
	//cout << __PRETTY_FUNCTION__;
	//if (inTexBuffer == nullptr) cout << endl;
	//else cout << "... " << *inTexBuffer << endl;
	
	lock_guard<recursive_mutex>		lock(_queueLock);
	
	GLBufferPoolRef		bp = (_privatePool!=nullptr) ? _privatePool : GetGlobalBufferPool();
	//	make the queue context current if appropriate- otherwise we are to assume that a GL context is current in this thread
	if (!createInCurrentContext)
		_queueCtx->makeCurrentIfNotCurrent();
	
	//	make sure the queues have the appropriate and expected number of elements
	int			tmpQueueSize = static_cast<int>(_pboQueue.size());
	if (tmpQueueSize != static_cast<int>(_cpuQueue.size()) || tmpQueueSize != static_cast<int>(_texQueue.size()))	{
		cout << "\tERR: queue size discrepancy, " << __PRETTY_FUNCTION__ << endl;
		return nullptr;
	}
	
	Size		texBufferDims = (inTexBuffer==nullptr) ? Size() : inTexBuffer->size;
	bool		safeToPush = false;
	bool		safeToPop = false;
	//	we're safe to push if the queue isn't too large AND there's a non-null input buffer
	if (tmpQueueSize<=_queueSize && inTexBuffer!=nullptr)
		safeToPush = true;
	//	we're safe to pop a val if the queue is too large AND if we're safe to push
	if (tmpQueueSize>=_queueSize && safeToPush)
		safeToPop = true;
	
	//	make an FBO- we need to attach the texture we want to download to this.
	GLBufferRef		tmpFBO = nullptr;
	//	if we're safe to push, we need to create a PBO
	GLBufferRef		inPBOBuffer = nullptr;
	if (safeToPush)	{
		tmpFBO = CreateFBO(createInCurrentContext, bp);
		switch (inTexBuffer->desc.pixelFormat)	{
		case GLBuffer::PF_RGBA:
			inPBOBuffer = CreateRGBAPBO(GLBuffer::Target_PBOPack, GL_DYNAMIC_READ, texBufferDims, nullptr, createInCurrentContext, bp);
			break;
		case GLBuffer::PF_BGRA:
			inPBOBuffer = CreateBGRAPBO(GLBuffer::Target_PBOPack, GL_DYNAMIC_READ, texBufferDims, nullptr, createInCurrentContext, bp);
			break;
		case GLBuffer::PF_YCbCr_422:
			inPBOBuffer = CreateYCbCrPBO(GLBuffer::Target_PBOPack, GL_DYNAMIC_READ, texBufferDims, nullptr, createInCurrentContext, bp);
			break;
		default:
			break;
		}
		
		//	if we couldn't create the buffers we need then we're not safe to push, and if we're not safe to push then we're not safe to pop.
		if (inPBOBuffer==nullptr)	{
			cout << "\tERR: couldnt make PBO, " << __PRETTY_FUNCTION__ << endl;
			safeToPush = false;
			safeToPop = false;
		}
	}
	
	//	push the buffer if appropriate
	if (safeToPush)	{
		_cpuQueue.push(inCPUBuffer);
		_pboQueue.push(inPBOBuffer);
		_texQueue.push(inTexBuffer);
		_fboQueue.push(tmpFBO);
		_beginProcessing(inCPUBuffer, inPBOBuffer, inTexBuffer, tmpFBO);
	}
	//	pop buffers off the queues if appropriate
	if (safeToPop)	{
		GLBufferRef		outCPUBuffer = _cpuQueue.front();
		_cpuQueue.pop();
		GLBufferRef		outPBOBuffer = _pboQueue.front();
		_pboQueue.pop();
		GLBufferRef		outTexBuffer = _texQueue.front();
		_texQueue.pop();
		GLBufferRef		outFBO = _fboQueue.front();
		_fboQueue.pop();
		_finishProcessing(outCPUBuffer, outPBOBuffer, outTexBuffer, outFBO);
		if (outCPUBuffer != nullptr)
			return outCPUBuffer;
		else
			return outPBOBuffer;
	}
	
	return nullptr;
}



}




#endif	//	!defined(VVGL_TARGETENV_GLES) && !defined(VVGL_TARGETENV_GLES3)


