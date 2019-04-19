#ifndef VVGL_GLCPUToTexCopier_hpp
#define VVGL_GLCPUToTexCopier_hpp

#include "VVGL_Defines.hpp"
#include "GLBufferPool.hpp"

#include <mutex>
#include <queue>




//	none of this stuff should be available if we're running ES
#if !defined(VVGL_TARGETENV_GLES) && !defined(VVGL_TARGETENV_GLES3)




namespace VVGL
{




//! Uploads CPU-based GLBuffers (Type_CPU) to textures.
/*!
\ingroup VVGL_BASIC
Offers both immediate upload and n-buffered texture uploads for double-/triple-/n-buffering/ping-ponging.  Uses PBOs for async DMA.
*/

class VVGL_EXPORT GLCPUToTexCopier	{
	private:
		recursive_mutex			_queueLock;	//	this should be used to serialize access to all member vars
		GLContextRef			_queueCtx = nullptr;	//	this is the context used to perform all GL action
		int						_queueSize = 2;	//	the number of buffers that should be in 'queue' before popping a new buffer off of it (double-buffering is 1)
		queue<GLBufferRef>		_cpuQueue;	//	queue of CPU-based images
		queue<GLBufferRef>		_pboQueue;	//	queue of PBOs
		queue<GLBufferRef>		_texQueue;	//	queue of textures
		bool					_swapBytes = false;
		GLBufferPoolRef			_privatePool = nullptr;	//	by default this is null and the scene will try to use the global buffer pool to create interim resources (temp/persistent buffers).  if non-null, the scene will use this pool to create interim resources.
	
	private:
		//	before calling either of these functions, _queueLock should be locked and a GL context needs to be made current on this thread.
		void _beginProcessing(const GLBufferRef & inCPUBuffer, const GLBufferRef & inPBOBuffer, const GLBufferRef & inTexBuffer);
		void _finishProcessing(const GLBufferRef & inCPUBuffer, const GLBufferRef & inPBOBuffer, const GLBufferRef & inTexBuffer);
	
	public:
		GLCPUToTexCopier();
		GLCPUToTexCopier(const GLContextRef & inCtx) : _queueCtx(inCtx) {};
		~GLCPUToTexCopier();
		
		//!	Gets the context
		GLContextRef context() { return _queueCtx; }
		
		//!	Clears all the queues, called when freed.
		void clearStream();
		
		//!	Sets the size of the queue used for streaming.  Effectively, this is the number of calls it takes for the CPU data to "finish uploading" and get returned as a texture.
		void setQueueSize(const int & inNewQueueSize);
		//!	Returns the size of the queue used for streaming.
		inline int queueSize() { lock_guard<recursive_mutex> lock(_queueLock); return _queueSize; };
		
		void setSwapBytes(const bool & n) { lock_guard<recursive_mutex> lock(_queueLock); _swapBytes=n; }
		bool swapBytes() { lock_guard<recursive_mutex> lock(_queueLock); return _swapBytes; }
		
		//!	Immediately uploads the passed CPU-based buffer to a GL texture- doesn't use the queues.  Less efficient.  Good for quick single-shot texture uploads.
		GLBufferRef uploadCPUToTex(const GLBufferRef & inCPUBuffer, const bool & createInCurrentContext=false);
		//!	Immediately uploads the passed CPU-based buffer to the passed GL texture- doesn't use the queues.  Less efficient.  Good for quick single-shot texture uploads.  Does not check the format or dimensions of the passed texture- make sure it's correct before calling!
		GLBufferRef uploadCPUToTex(const GLBufferRef & inCPUBuffer, const GLBufferRef & inTexBuffer, const bool & createInCurrentContext=false);
		
		//! Begins uploading the passed CPU-based buffer to a GL texture, but stashes it in a queue and will return the texture when this function is called again at a later time (ping-pong/double-/triple-/n-buffering).  Good for streaming texture upload.
		/*!
		\param inCPUBuffer This must be a CPU-based GLBuffer.  This may not be null.
		\param createInCurrentContext Defaults to false- if true, any GL resources will be created by the current GL context in the calling thread.  If false, the local var _queueCtx will be used.
		More efficient than uploadCPUToTex()- CPU use will probably be lower and execution will return to the calling thread more rapidly, though the queue means that there's more latency (it won't start returning buffers until you submit one or two- depending on the size of the queue).
		*/
		GLBufferRef streamCPUToTex(const GLBufferRef & inCPUBuffer, const bool & createInCurrentContext=false);
		//! Begins uploading the passed CPU-based buffer to the passed GL texture, but stashes it in a queue and will return the texture when this function is called again at a later time (ping-pong/double-/triple-/n-buffering).  Good for streaming texture upload.  Does not check the format or dimensions of the passed texture- make sure it's correct before calling!
		/*!
		\param inCPUBuffer This must be a CPU-based GLBuffer.  This may not be null.
		\param createInCurrentContext Defaults to false- if true, any GL resources will be created by the current GL context in the calling thread.  If false, the local var _queueCtx will be used.
		More efficient than uploadCPUToTex()- CPU use will probably be lower and execution will return to the calling thread more rapidly, though the queue means that there's more latency (it won't start returning buffers until you submit one or two- depending on the size of the queue).
		*/
		GLBufferRef streamCPUToTex(const GLBufferRef & inCPUBuffer, const GLBufferRef & inTexBuffer, const bool & createInCurrentContext=false);
		
		//!	Sets the receiver's private buffer pool (which should default to null).  If non-null, this buffer pool will be used to generate any GL resources required by this scene.  Handy if you have a variety of GL contexts that aren't shared and you have to switch between them rapidly on a per-frame basis.
		void setPrivatePool(const GLBufferPoolRef & n) { _privatePool=n; }
		//!	Gets the receiver's private buffer pool- null by default, only non-null if something called setPrivatePool().
		GLBufferPoolRef privatePool() { return _privatePool; }
};




/*!
\relatedalso GLCPUToTexCopier
\brief Creates and returns a GLCPUToTexCopier.  The scene makes a new GL context which shares the context of the global buffer pool.
*/
inline GLCPUToTexCopierRef CreateGLCPUToTexCopierRef() { return make_shared<GLCPUToTexCopier>(); }
/*!
\relatedalso GLCPUToTexCopier
\brief Creates and returns a GLCPUToTexCopier.  The downloader uses the passed GL context to perform its GL operations.
*/
inline GLCPUToTexCopierRef CreateGLCPUToTexCopierRefUsing(const GLContextRef & inCtx) { return make_shared<GLCPUToTexCopier>(inCtx); }




}




#endif	//	!defined(VVGL_TARGETENV_GLES) && !defined(VVGL_TARGETENV_GLES3)




#endif /* VVGL_GLCPUToTexCopier_hpp */
