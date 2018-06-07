#ifndef VVGL_GLTexToCPUCopier_hpp
#define VVGL_GLTexToCPUCopier_hpp

#include "VVGL_Defines.hpp"
#include "GLBufferPool.hpp"

#include <mutex>
#include <queue>




//	none of this stuff should be available if we're running ES
#if !defined(VVGL_TARGETENV_GLES) && !defined(VVGL_TARGETENV_GLES3)




namespace VVGL
{




using namespace std;




//!	Downloads texture-based GLBuffers (Type_Tex) to CPU memory.
/*!
\ingroup VVGL_BASIC
Offers both immediate download and n-buffered texture downloads for double-/triple-/n-buffering/ping-ponging.  Uses PBOs for async DMA.
*/

class VVGL_EXPORT GLTexToCPUCopier	{
	private:
		recursive_mutex			queueLock;	//	this should be used to serialize access to all member vars
		GLContextRef			queueCtx = nullptr;	//	this is the context used to perform all GL action
		int						queueSize = 1;	//	the number of buffers that should be in 'queue' before popping a new buffer off of it (double-buffering is 1)
		queue<GLBufferRef>		cpuQueue;	//	queue of CPU-based images
		queue<GLBufferRef>		pboQueue;	//	queue of PBOs
		queue<GLBufferRef>		texQueue;	//	queue of textures
		queue<GLBufferRef>		fboQueue;	//	queue of FBOs.  the fastest texture download pipeline involves attaching the texture to an FBO so we can use glReadPixels() instead of glGetTexImage().
		
	private:
		//	before calling either of these functions, queueLock should be locked and a GL context needs to be made current on this thread.
		void _beginProcessing(const GLBufferRef & inCPUBuffer, const GLBufferRef & inPBOBuffer, const GLBufferRef & inTexBuffer, const GLBufferRef & inFBOBuffer);
		void _finishProcessing(const GLBufferRef & inCPUBuffer, const GLBufferRef & inPBOBuffer, const GLBufferRef & inTexBuffer, const GLBufferRef & inFBOBuffer);
	
	public:
		GLTexToCPUCopier();
		GLTexToCPUCopier(const GLContextRef & inCtx) : queueCtx(inCtx) {};
		~GLTexToCPUCopier();
		
		//!	Clears all the queues, called when freed.
		void clearStream();
		
		//!	Sets the size of the queue used for streaming.  Effectively, this is the number of calls it takes for the texture data to "finish downloading" and get returned as a pointer to non-GPU memory.
		void setQueueSize(const int & inNewQueueSize);
		//!	Returns the size of the queue used for streaming.
		inline int getQueueSize() { lock_guard<recursive_mutex> lock(queueLock); return queueSize; };
		
		//! Immediately downloads the passed texture into CPU memory- doesn't use the queues.  Less efficient.  Good for quick single-shot texture downloads.
		/*!
		\param inTexBuffer The texture-type GLBuffer that you wish to download to memory.  Must not be null.
		\param inCPUBuffer May be null (null by default).  If null, this function will return a PBO-type GLBuffer that has been mapped- you can access the pixels at its cpuBackingPtr ivar for analysis, encoding, etc.  If you provide a non-null CPU-type GLBuffer for this param, this function will instead return the CPU-type buffer you provided, after populating it with the contents of the texture.
		This function downloads the texture immediately- it doesn't use the queue/doesn't do any double-/triple-/n-buffering.  This function is generally less efficient than streamTexToCPU(), but it's still appropriate if you just want to download a texture immediately and aren't doing any extensive streaming.
		*/
		GLBufferRef downloadTexToCPU(const GLBufferRef & inTexBuffer, const GLBufferRef & inCPUBuffer=nullptr, const bool & createInCurrentContext=false);
		
		//!	Begins downloading the passed texture-based buffer to CPU memory, but stashes it in a queue and will return the CPU-based GLBuffer when this function is called again at a later time (ping-pong/double-/triple-/n-buffering).  Good for streaming texture download.
		/*!
		\param inTexBuffer This must be a texture-based GLBuffer.  This may not be null.
		\param inCPUBuffer May be null (null by default).  If null, this function will eventually return a PBO-type GLBuffer that has been mapped for 'inTexBuffer'.  You can access the pixels of this mapped PBO at its cpuBackingPtr ivar for analysis, encoding, etc.  If you provide a non-null CPU-type GLBuffer for this param, this function will instead eventually return the CPU-type buffer you provided, after populating it with the contents of the texture.
		This function is more efficient than downloadTexToCPU()- CPU use will probably be lower and execution will return to the calling thread more rapidly, though the queue means that there's more latency (it won't start returning buffers until you submit one or two- depending on the size of the queue).
		*/
		GLBufferRef streamTexToCPU(const GLBufferRef & inTexBuffer, const GLBufferRef & inCPUBuffer=nullptr, const bool & createInCurrentContext=false);
};




/*!
\relatedalso GLTexToCPUCopier
\brief Creates and returns a GLTexToCPUCopier.  The scene makes a new GL context which shares the context of the global buffer pool.
*/
inline GLTexToCPUCopierRef CreateGLTexToCPUCopierRef() { return make_shared<GLTexToCPUCopier>(); }
/*!
\relatedalso GLTexToCPUCopier
\brief Creates and returns a GLTexToCPUCopier.  The downloader uses the passed GL context to perform its GL operations.
*/
inline GLTexToCPUCopierRef CreateGLTexToCPUCopierRefUsing(const GLContextRef & inCtx) { return make_shared<GLTexToCPUCopier>(inCtx); }




}




#endif	//	!defined(VVGL_TARGETENV_GLES) && !defined(VVGL_TARGETENV_GLES3)




#endif /* VVGL_GLTexToCPUCopier_hpp */
