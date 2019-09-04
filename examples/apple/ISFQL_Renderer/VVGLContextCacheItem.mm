#import "VVGLContextCacheItem.h"




static recursive_mutex						_VVGLContextCacheArrayLock;
static vector<VVGLContextCacheItemRef>		_VVGLContextCacheArray;




VVGLContextCacheItem::VVGLContextCacheItem()	{
	using namespace std;
	//cout << __PRETTY_FUNCTION__ << endl;
	
	gl2Context = CreateNewGLContextRef(NULL, CreateDefaultPixelFormat());
	gl2Pool = make_shared<GLBufferPool>(gl2Context);
	
	gl4Context = CreateNewGLContextRef(NULL, CreateGL4PixelFormat());
	gl4Pool = make_shared<GLBufferPool>(gl4Context);
	
	NSImage				*colorBarsImg = [[NSImage alloc] initWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"ColorBars" ofType:@"png"]];
	NSImage				*invColorBarsImg = [[NSImage alloc] initWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"InvColorBars" ofType:@"png"]];
	
	if (gl2Context != nullptr && gl2Pool != nullptr)	{
		gl2Copier = make_shared<GLTexToTexCopier>(gl2Context->newContextSharingMe());
		gl2Copier->setPrivatePool(gl2Pool);
		
		gl2Downloader = make_shared<GLTexToCPUCopier>(gl2Context->newContextSharingMe());
		gl2Downloader->setPrivatePool(gl2Pool);
		
		gl2ColorBars = CreateBufferForNSImage(colorBarsImg, false, gl2Pool);
		gl2InvColorBars = CreateBufferForNSImage(invColorBarsImg, false, gl2Pool);
	}
	if (gl4Context != nullptr && gl4Pool != nullptr)	{
		gl4Copier = make_shared<GLTexToTexCopier>(gl4Context->newContextSharingMe());
		gl4Copier->setPrivatePool(gl4Pool);
		
		gl4Downloader = make_shared<GLTexToCPUCopier>(gl4Context->newContextSharingMe());
		gl4Downloader->setPrivatePool(gl4Pool);
		
		gl4ColorBars = CreateBufferForNSImage(colorBarsImg, false, gl4Pool);
		gl4InvColorBars = CreateBufferForNSImage(invColorBarsImg, false, gl4Pool);
	}
	
}
VVGLContextCacheItem::~VVGLContextCacheItem()	{
	using namespace std;
	//cout << __PRETTY_FUNCTION__ << endl;
	
	if (gl2Pool != nullptr)	{
		gl2Pool->purge();
		gl2Pool = nullptr;
	}
	if (gl4Pool != nullptr)	{
		gl4Pool->purge();
		gl4Pool = nullptr;
	}
	gl2Copier = nullptr;
	gl4Copier = nullptr;
	gl2Downloader = nullptr;
	gl4Downloader = nullptr;
	gl2Context = nullptr;
	gl4Context = nullptr;
}




VVGLContextCacheItem & VVGLContextCacheItem::operator=(const VVGLContextCacheItem & n)	{
	gl2Context = n.getGL2Context();
	gl2Pool = n.getGL2Pool();
	gl4Context = n.getGL4Context();
	gl4Pool = n.getGL4Pool();
	gl2Copier = n.getGL2Copier();
	gl4Copier = n.getGL4Copier();
	gl2Downloader = n.getGL2Downloader();
	gl4Downloader = n.getGL4Downloader();
	return *this;
}
VVGLContextCacheItem & VVGLContextCacheItem::operator=(const VVGLContextCacheItemRef & n)	{
	gl2Context = n->getGL2Context();
	gl2Pool = n->getGL2Pool();
	gl4Context = n->getGL4Context();
	gl4Pool = n->getGL4Pool();
	gl2Copier = n->getGL2Copier();
	gl4Copier = n->getGL4Copier();
	gl2Downloader = n->getGL2Downloader();
	gl4Downloader = n->getGL4Downloader();
	return *this;
}




VVGLContextCacheItemRef GetCacheItem()	{
	using namespace std;
	//cout << __PRETTY_FUNCTION__ << endl;
	
	lock_guard<recursive_mutex>		lock(_VVGLContextCacheArrayLock);
	VVGLContextCacheItemRef			returnMe = nullptr;
	
	if (_VVGLContextCacheArray.capacity() <= 4)
		_VVGLContextCacheArray.reserve(4);
	
	//	run through the scenes in the gl2Pool
	for (auto cacheIt=_VVGLContextCacheArray.begin(); cacheIt!=_VVGLContextCacheArray.end(); ++cacheIt)	{
		returnMe = *cacheIt;
		_VVGLContextCacheArray.erase(cacheIt);
		break;
	}
	
	//	if we haven't found an item to return, we have to create one!
	if (returnMe == nullptr)	{
		//cout << "\tcouldn't find a cached item, making a new GL context and cache item\n";
		returnMe = make_shared<VVGLContextCacheItem>();
	}
	
	return returnMe;
}
void ReturnCacheItemToPool(const VVGLContextCacheItemRef & inItem)	{
	using namespace std;
	//cout << __PRETTY_FUNCTION__ << endl;
	
	if (inItem == nullptr)
		return;
	lock_guard<recursive_mutex>		lock(_VVGLContextCacheArrayLock);
	if (_VVGLContextCacheArray.size() >= _VVGLContextCacheArray.max_size())	{
		_VVGLContextCacheArray.erase(_VVGLContextCacheArray.begin());
	}
	_VVGLContextCacheArray.push_back(inItem);
}

