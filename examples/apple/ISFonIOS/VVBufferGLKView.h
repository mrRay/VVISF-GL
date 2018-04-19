#import <Foundation/Foundation.h>
#import <pthread.h>
#import <libkern/OSAtomic.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES3/glext.h>

#import <GLKit/GLKit.h>

#include "VVGL.hpp"




using namespace VVGL;




@interface VVBufferGLKView : GLKView	{
	BOOL				initialized;
	pthread_mutex_t		renderLock;
	VVGL::GLSceneRef	scene;	//	this scene draws in the view
	VVGL::GLBufferRef	vao;
	
	VVGL::SizingMode	sizingMode;
	
	BOOL				retainDraw;
	OSSpinLock			retainDrawLock;
	VVGL::GLBufferRef	retainDrawBuffer;
	
	BOOL				onlyDrawNewStuff;	//	NO by default. if YES, only draws buffers with content timestamps different from the timestamp of the last buffer displayed
	OSSpinLock			onlyDrawNewStuffLock;
	VVGL::Timestamp		onlyDrawNewStuffTimestamp;
}

- (void) redraw;
///	Draws the passed buffer
- (void) drawBuffer:(VVGL::GLBufferRef)b;
///	Sets the GL context to share- this is generally done automatically (using the global buffer pool's shared context), but if you want to override it and use a different context...this is how.
- (void) setSharedGLContext:(const GLContextRef)n;


@property (assign,readwrite) VVGL::SizingMode sizingMode;
- (void) setRetainDraw:(BOOL)n;
- (void) setRetainDrawBuffer:(VVGL::GLBufferRef)n;
@property (assign,readwrite) BOOL onlyDrawNewStuff;

@end
