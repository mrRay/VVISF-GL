#import "VVGLBufferGLView.h"




#define VVRELEASE(item) {if (item != nil)	{			\
	[item release];										\
	item = nil;											\
}}

@interface VVGLBufferGLView()	{
	
}
- (void) setSharedGLContext:(const VVGLContext *)n;
@end




@implementation VVGLBufferGLView


- (id) initWithFrame:(NSRect)f	{
	if (self = [super initWithFrame:f])	{
		pthread_mutexattr_t		attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
		pthread_mutex_init(&renderLock, &attr);
		pthread_mutexattr_destroy(&attr);
		initialized = NO;
		sizingMode = VVISF::SizingMode_Fit;
		retainDraw = NO;
		retainDrawLock = OS_SPINLOCK_INIT;
		retainDrawBuffer = nullptr;
		onlyDrawNewStuff = NO;
		onlyDrawNewStuffLock = OS_SPINLOCK_INIT;
		onlyDrawNewStuffTimestamp = VVISF::Timestamp();
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
		sizingMode = VVISF::SizingMode_Fit;
		retainDraw = NO;
		retainDrawLock = OS_SPINLOCK_INIT;
		retainDrawBuffer = nullptr;
		onlyDrawNewStuff = NO;
		onlyDrawNewStuffLock = OS_SPINLOCK_INIT;
		onlyDrawNewStuffTimestamp = VVISF::Timestamp();
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
	[super dealloc];
}
- (void) drawRect:(NSRect)r	{
	
	pthread_mutex_lock(&renderLock);
		if (!initialized)	{
			using namespace VVISF;
			
			VVGLBufferPoolRef		bp = GetGlobalBufferPool();
			if (bp != nullptr)	{
				[self setSharedGLContext:bp->getContext()];
				initialized = YES;
			}
		}
	pthread_mutex_unlock(&renderLock);
	[self redraw];
	
}
- (void) redraw	{
	using namespace VVISF;
	
	VVGLBufferRef	lastBuffer = nullptr;
	OSSpinLockLock(&retainDrawLock);
	lastBuffer = (!retainDraw || retainDrawBuffer==nullptr) ? nullptr : retainDrawBuffer;
	OSSpinLockUnlock(&retainDrawLock);
	
	[self drawBuffer:lastBuffer];
}
- (void) drawBuffer:(VVISF::VVGLBufferRef)b	{
	//NSLog(@"%s",__func__);
	
	BOOL			bail = NO;
	
	OSSpinLockLock(&retainDrawLock);
	if (retainDraw)	{
		retainDrawBuffer = b;
	}
	OSSpinLockUnlock(&retainDrawLock);
	
	OSSpinLockLock(&onlyDrawNewStuffLock);
	if (onlyDrawNewStuff)	{
		if (onlyDrawNewStuffTimestamp == b->contentTimestamp)
			bail = YES;
	}
	OSSpinLockUnlock(&onlyDrawNewStuffLock);
	if (bail)
		return;
	
	GLuint			target = (b==nullptr) ? GL_TEXTURE_2D : b->desc.target;
	pthread_mutex_lock(&renderLock);
		if (initialized)	{
			[[self openGLContext] makeCurrentContext];
			
			//CGLContextObj		cgl_ctx = [[self openGLContext] CGLContextObj];
			glEnableClientState(GL_VERTEX_ARRAY);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			
			//glBlendFunc(GL_ONE, GL_ZERO);
			//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glHint(GL_CLIP_VOLUME_CLIPPING_HINT_EXT, GL_FASTEST);
			glDisable(GL_DEPTH_TEST);
			glClearColor(0.0, 0.0, 0.0, 1.0);
			
			glEnable(target);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			
			//	bilinear filtering stuff
			//glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			//glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			//glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			//glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			
			//	set up the view to draw
			NSRect				bounds = [self bounds];
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glViewport(0, 0, (GLsizei) bounds.size.width, (GLsizei) bounds.size.height);
			//if (flipped)
				glOrtho(bounds.origin.x, bounds.origin.x+bounds.size.width, bounds.origin.y, bounds.origin.y+bounds.size.height, 1.0, -1.0);
			//else
			//	glOrtho(bounds.origin.x, bounds.origin.x+bounds.size.width, bounds.origin.y+bounds.size.height, bounds.origin.y, 1.0, -1.0);
			//glEnable(GL_BLEND);
			//glBlendFunc(GL_SRC_ALPHA,GL_DST_ALPHA);
			//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glDisable(GL_BLEND);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
			//	clear the view
			glClearColor(0.0,0.0,0.0,0.0);
			glClear(GL_COLOR_BUFFER_BIT);
			
			
			if (b != nil)	{
				NSRect			bounds = [self bounds];
				VVISF::Rect	boundsRect = { bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height };
				VVISF::Rect	destRect = VVISF::ResizeRect(b->srcRect, boundsRect, VVISF::SizingMode_Fit);
				
				/*
				b->draw(destRect);
				*/
				
				
				
				//inCtx.makeCurrentIfNotCurrent();
				float			verts[] = {
					(float)MinX(destRect), (float)MinY(destRect), 0.0,
					(float)MaxX(destRect), (float)MinY(destRect), 0.0,
					(float)MaxX(destRect), (float)MaxY(destRect), 0.0,
					(float)MinX(destRect), (float)MaxY(destRect), 0.0
				};
				bool			flipped = b->flipped;
				VVISF::Rect			src = b->glReadySrcRect();
				float			texs[] = {
					(float)MinX(src), (flipped) ? (float)MaxY(src) : (float)MinY(src),
					(float)MaxX(src), (flipped) ? (float)MaxY(src) : (float)MinY(src),
					(float)MaxX(src), (flipped) ? (float)MinY(src) : (float)MaxY(src),
					(float)MinX(src), (flipped) ? (float)MinY(src) : (float)MaxY(src)
				};
				glEnableClientState(GL_VERTEX_ARRAY);
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				glDisableClientState(GL_COLOR_ARRAY);
	
				glVertexPointer(3, GL_FLOAT, 0, verts);
				glTexCoordPointer(2, GL_FLOAT, 0, texs);
				glBindTexture(b->desc.target, b->name);
				glDrawArrays(GL_QUADS, 0, 4);
				glBindTexture(b->desc.target, 0);
				
			}
			//	flush!
			glFlush();
			
			glDisable(target);
		}
	pthread_mutex_unlock(&renderLock);
	
}
/*
- (void) setSharedGLContext:(CGLContextObj)c	{
	if (c == nullptr)
		return;
	pthread_mutex_lock(&renderLock);
		CGLPixelFormatObj	rawPxlFmt = VVISF::CreateDefaultPixelFormat();
		NSOpenGLPixelFormat	*pxlFmt = [[[NSOpenGLPixelFormat alloc] initWithCGLPixelFormatObj:rawPxlFmt] autorelease];
		NSOpenGLContext		*sharedCtx = [[NSOpenGLContext alloc] initWithCGLContextObj:c];
		
		NSOpenGLContext		*newContext = [[NSOpenGLContext alloc] initWithFormat:pxlFmt shareContext:sharedCtx];
		[self setOpenGLContext:newContext];
		[newContext setView:self];
		[newContext release];
		
		long				swap = 1;
		[[self openGLContext] setValues:(GLint *)&swap forParameter:NSOpenGLCPSwapInterval];
		initialized = YES;
		
		CGLReleasePixelFormat(rawPxlFmt);
	pthread_mutex_unlock(&renderLock);
}
*/
- (void) setSharedGLContext:(const VVGLContext *)n	{
	pthread_mutex_lock(&renderLock);
		NSOpenGLContext		*tmpSharedCtx = (n->sharedCtx==nullptr) ? nil : [[[NSOpenGLContext alloc] initWithCGLContextObj:n->sharedCtx] autorelease];
		NSOpenGLContext		*newContext = [[NSOpenGLContext alloc]
			initWithFormat:[[[NSOpenGLPixelFormat alloc] initWithCGLPixelFormatObj:n->pxlFmt] autorelease]
			shareContext:tmpSharedCtx];
		if (newContext != nil)	{
			[self setOpenGLContext:newContext];
			[newContext setView:self];
			[newContext release];
			long				swap = 1;
			[[self openGLContext] setValues:(GLint *)&swap forParameter:NSOpenGLCPSwapInterval];
			initialized = YES;
		}
	pthread_mutex_unlock(&renderLock);
}


@synthesize initialized;
@synthesize sizingMode;
- (void) setOnlyDrawNewStuff:(BOOL)n	{
	using namespace VVISF;
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
- (void) setRetainDrawBuffer:(VVISF::VVGLBufferRef)n	{
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
