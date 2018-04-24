#import "ISFVVGLBufferView.h"
//#include "GLContext.hpp"




#define VVRELEASE(item) {if (item != nil)	{			\
	[item release];										\
	item = nil;											\
}}

@interface ISFVVGLBufferView()	{
	
}
@property (assign,readwrite,setter=setVAO:,getter=vao) VVGL::GLBufferRef vao;
@property (assign,readwrite) BOOL initialized;
@property (readonly) VVGL::GLBufferRef retainDrawBuffer;
@end




@implementation ISFVVGLBufferView


- (id) initWithFrame:(NSRect)f	{
	if (self = [super initWithFrame:f])	{
		pthread_mutexattr_t		attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
		pthread_mutex_init(&renderLock, &attr);
		pthread_mutexattr_destroy(&attr);
		initialized = NO;
		sceneFilePath = nil;
		scene = nullptr;
		vao = nullptr;
		sizingMode = VVGL::SizingMode_Fit;
		retainDraw = NO;
		retainDrawLock = OS_SPINLOCK_INIT;
		retainDrawBuffer = nullptr;
		onlyDrawNewStuff = NO;
		onlyDrawNewStuffLock = OS_SPINLOCK_INIT;
		onlyDrawNewStuffTimestamp = VVGL::Timestamp();
		return self;
	}
	if (self != nil)
		[self release];
	return nil;
}
- (id) initWithCoder:(NSCoder *)c	{
	if (self = [super initWithCoder:c])	{
		pthread_mutexattr_t		attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
		pthread_mutex_init(&renderLock, &attr);
		pthread_mutexattr_destroy(&attr);
		initialized = NO;
		sceneFilePath = nil;
		scene = nullptr;
		vao = nullptr;
		sizingMode = VVGL::SizingMode_Fit;
		retainDraw = NO;
		retainDrawLock = OS_SPINLOCK_INIT;
		retainDrawBuffer = nullptr;
		onlyDrawNewStuff = NO;
		onlyDrawNewStuffLock = OS_SPINLOCK_INIT;
		onlyDrawNewStuffTimestamp = VVGL::Timestamp();
		return self;
	}
	if (self != nil)
		[self release];
	return nil;
}
- (void) awakeFromNib	{
	initialized = NO;
}
- (void) dealloc	{
	pthread_mutex_destroy(&renderLock);
	OSSpinLockLock(&retainDrawLock);
	retainDrawBuffer = nullptr;
	OSSpinLockUnlock(&retainDrawLock);
	VVRELEASE(sceneFilePath);
	[super dealloc];
}
- (void) drawRect:(NSRect)r	{
	
	pthread_mutex_lock(&renderLock);
		if (!initialized)	{
			//using namespace VVISF;
			
			GLBufferPoolRef		bp = GetGlobalBufferPool();
			if (bp != nullptr)	{
				[self setSharedGLContext:bp->getContext()];
				initialized = YES;
			}
		}
	pthread_mutex_unlock(&renderLock);
	
	if (initialized)
		[self redraw];
	
}
- (void) redraw	{
	//using namespace VVISF;
	
	GLBufferRef	lastBuffer = nullptr;
	OSSpinLockLock(&retainDrawLock);
	lastBuffer = (!retainDraw || retainDrawBuffer==nullptr) ? nullptr : retainDrawBuffer;
	OSSpinLockUnlock(&retainDrawLock);
	
	[self drawBuffer:lastBuffer];
}
- (void) drawBuffer:(VVGL::GLBufferRef)b	{
	//NSLog(@"%s ... %s",__func__,b->getDescriptionString().c_str());
	
	BOOL			bail = NO;
	
	OSSpinLockLock(&retainDrawLock);
	//if (retainDraw)	{
		retainDrawBuffer = b;
	//}
	OSSpinLockUnlock(&retainDrawLock);
	
	OSSpinLockLock(&onlyDrawNewStuffLock);
	if (onlyDrawNewStuff)	{
		if (onlyDrawNewStuffTimestamp == b->contentTimestamp)
			bail = YES;
	}
	OSSpinLockUnlock(&onlyDrawNewStuffLock);
	if (bail)
		return;
	
	//	we probably shouldn't do this every time we draw a buffer.
	NSRect		tmpBounds = [self backingBounds];
	scene->setOrthoSize(VVGL::Size(tmpBounds.size.width, tmpBounds.size.height));
	
	OSSpinLockLock(&retainDrawLock);
	if (retainDrawBuffer != nullptr)	{
		scene->setFilterInputBuffer(retainDrawBuffer);
	}
	OSSpinLockUnlock(&retainDrawLock);
	
	pthread_mutex_lock(&renderLock);
		if (initialized)	{
			[[self openGLContext] makeCurrentContext];
			scene->render();
		}
	pthread_mutex_unlock(&renderLock);
	
	
	OSSpinLockLock(&retainDrawLock);
	if (!retainDraw)	{
		retainDrawBuffer = nullptr;
	}
	OSSpinLockUnlock(&retainDrawLock);
}
- (void) setSharedGLContext:(const GLContextRef)n	{
	using namespace VVISF;
	
	pthread_mutex_lock(&renderLock);
	
	void			*selfPtr = (void*)self;
	//	make a new scene- this also makes a new GL context that shares the passed context
	scene = CreateISFScene(n->newContextSharingMe());
	if (sceneFilePath != nil)
		scene->useFile(string([sceneFilePath UTF8String]));
	//	make an NSOpenGLContext wrapping the CGLContext inside the GLContextRef, make it draw into the view
	NSOpenGLContext		*sceneCtxWrapper = [[NSOpenGLContext alloc] initWithCGLContextObj:scene->getContext()->ctx];
	if (sceneCtxWrapper != nil)	{
		[self setOpenGLContext:sceneCtxWrapper];
		[sceneCtxWrapper setView:self];
		[sceneCtxWrapper release];
		long				swap = 1;
		[[self openGLContext] setValues:(GLint *)&swap forParameter:NSOpenGLCPSwapInterval];
		initialized = YES;
	}
	
	pthread_mutex_unlock(&renderLock);
}


- (void) useFile:(NSString *)inFilePath	{
	VVRELEASE(sceneFilePath);
	sceneFilePath = (inFilePath==nil) ? nil : [inFilePath retain];
	if (sceneFilePath != nil)	{
		scene->useFile(string([sceneFilePath UTF8String]));
	}
}
- (const VVISF::ISFSceneRef &) scene	{
	return scene;
}


@synthesize initialized;
@synthesize sizingMode;
@synthesize retainDrawBuffer;
@synthesize vao;
- (void) setOnlyDrawNewStuff:(BOOL)n	{
	//using namespace VVISF;
	OSSpinLockLock(&onlyDrawNewStuffLock);
	onlyDrawNewStuff = n;
	onlyDrawNewStuffTimestamp = Timestamp();
	OSSpinLockUnlock(&onlyDrawNewStuffLock);
}
- (void) setRetainDraw:(BOOL)n	{
	OSSpinLockLock(&retainDrawLock);
	retainDraw = n;
	OSSpinLockUnlock(&retainDrawLock);
}
- (void) setRetainDrawBuffer:(VVGL::GLBufferRef)n	{
	OSSpinLockLock(&retainDrawLock);
	retainDrawBuffer = n;
	OSSpinLockUnlock(&retainDrawLock);
}
- (BOOL) onlyDrawNewStuff	{
	BOOL		returnMe = NO;
	OSSpinLockLock(&onlyDrawNewStuffLock);
	returnMe = onlyDrawNewStuff;
	OSSpinLockUnlock(&onlyDrawNewStuffLock);
	return returnMe;
}


@end








@implementation NSOpenGLView (NSOpenGLViewISFVVGLBufferViewAdditions)
- (NSRect) backingBounds	{
	return [(id)self convertRectToBacking:[self bounds]];
}
@end

