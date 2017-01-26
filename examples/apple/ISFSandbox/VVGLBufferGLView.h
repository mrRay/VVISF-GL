#import <AppKit/AppKit.h>
#import <pthread.h>
#import <libkern/OSAtomic.h>

#include "ISFKit.h"




@interface VVGLBufferGLView : NSOpenGLView	{
	BOOL				initialized;
	pthread_mutex_t		renderLock;
	
	VVISF::SizingMode	sizingMode;
	
	BOOL				retainDraw;
	OSSpinLock			retainDrawLock;
	VVISF::VVGLBufferRef	retainDrawBuffer;
	
	BOOL				onlyDrawNewStuff;	//	NO by default. if YES, only draws buffers with content timestamps different from the timestamp of the last buffer displayed
	OSSpinLock			onlyDrawNewStuffLock;
	VVISF::Timestamp		onlyDrawNewStuffTimestamp;
}

- (void) redraw;
///	Draws the passd buffer
- (void) drawBuffer:(VVISF::VVGLBufferRef)b;
///	Sets the GL context to share- this is generally done automatically (using the global buffer pool's shared context), but if you want to override it and use a different context...this is how.
//- (void) setSharedGLContext:(CGLContextObj)c;

@property (assign,readwrite) BOOL initialized;
@property (assign,readwrite) VVISF::SizingMode sizingMode;
- (void) setRetainDraw:(BOOL)n;
- (void) setRetainDrawBuffer:(VVISF::VVGLBufferRef)n;
@property (assign,readwrite) BOOL onlyDrawNewStuff;

@end
