#import <Cocoa/Cocoa.h>
#import <OpenGL/OpenGL.h>
#import <CoreVideo/CoreVideo.h>
#include "VVGL.hpp"
#import "VVGLBufferGLView.h"




using namespace VVGL;




@interface DifferingGLVersionsAppDelegate : NSObject <NSApplicationDelegate>	{
	CVDisplayLinkRef			displayLink;
	
	GLContextRef				legacyGLCtx;
	GLContextRef				modernGLCtx;
	
	GLBufferPoolRef			legacyBufferPool;
	GLBufferPoolRef			modernBufferPool;
	
	IBOutlet VVGLBufferGLView	*legacyBufferView;
	IBOutlet VVGLBufferGLView	*modernBufferView;
	
	GLSceneRef				legacyGLScene;
	GLSceneRef				modernGLScene;
	
	NSDate						*date;	//	used to fade the texture in repeatedly
	GLBufferRef				vao;
	Quad<VertXYST>				lastVBOCoords;	//	the last coords used in the VBO associated with 'vao' (the VAO implicitly retains the VBO, so we only need to update it when the coords change- which we track with this)
}

- (void) initLegacyGL;
- (void) initModernGL;

- (void) renderCallback;

@property (retain,readwrite) NSDate *date;
@property (assign,readwrite,setter=setVAO:) GLBufferRef vao;

@end




CVReturn displayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp *inNow, const CVTimeStamp *inOutputTime, CVOptionFlags flagsIn, CVOptionFlags *flagsOut, void *displayLinkContext);
