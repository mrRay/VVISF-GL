#include "VVISF.hpp"


/*		caches a GL context (provided on init by the host, expected to be a GL2-based context).  also creates a pool and copier that shares the context (the pool and copier create their own contexts)			*/


using namespace VVGL;
using namespace VVISF;
using namespace std;

class VVGLContextCacheItem;
using VVGLContextCacheItemRef = shared_ptr<VVGLContextCacheItem>;


class VVGLContextCacheItem	{
	private:
		
		GLContextRef			gl2Context = nullptr;	//	this is the shared ctx for gl2 contexts
		GLBufferPoolRef			gl2Pool = nullptr;
		
		GLContextRef			gl4Context = nullptr;	//	this is the shared ctx for gl4 contexts
		GLBufferPoolRef			gl4Pool = nullptr;
		
		GLTexToTexCopierRef		gl2Copier = nullptr;
		GLTexToTexCopierRef		gl4Copier = nullptr;
		
		GLTexToCPUCopierRef		gl2Downloader = nullptr;
		GLTexToCPUCopierRef		gl4Downloader = nullptr;
		
		GLBufferRef				gl2ColorBars = nullptr;
		GLBufferRef				gl4ColorBars = nullptr;
		
		GLBufferRef				gl2InvColorBars = nullptr;
		GLBufferRef				gl4InvColorBars = nullptr;
	
	public:
		VVGLContextCacheItem();
		~VVGLContextCacheItem();
		
		VVGLContextCacheItem & operator=(const VVGLContextCacheItem & n);
		VVGLContextCacheItem & operator=(const VVGLContextCacheItemRef & n);
		
		inline GLContextRef getGL2Context() const { return gl2Context; };
		inline GLBufferPoolRef getGL2Pool() const { return gl2Pool; };
		inline GLContextRef getGL4Context() const { return gl4Context; };
		inline GLBufferPoolRef getGL4Pool() const { return gl4Pool; };
		
		inline GLTexToTexCopierRef getGL2Copier() const { return gl2Copier; };
		inline GLTexToTexCopierRef getGL4Copier() const { return gl4Copier; };
		
		inline GLTexToCPUCopierRef getGL2Downloader() const { return gl2Downloader; };
		inline GLTexToCPUCopierRef getGL4Downloader() const { return gl4Downloader; };
		
		inline GLBufferRef getGL2ColorBars() const { return gl2ColorBars; };
		inline GLBufferRef getGL4ColorBars() const { return gl4ColorBars; };
		
		inline GLBufferRef getGL2InvColorBars() const { return gl2InvColorBars; };
		inline GLBufferRef getGL4InvColorBars() const { return gl4InvColorBars; };
};

VVGLContextCacheItemRef GetCacheItem();
//VVGLContextCacheItemRef GetCacheItemForContext(const CGLContextObj & inHostCtx);
//VVGLContextCacheItemRef GetCacheItemForContext(const GLContextRef & inHostCtx);
void ReturnCacheItemToPool(const VVGLContextCacheItemRef & inItem);
