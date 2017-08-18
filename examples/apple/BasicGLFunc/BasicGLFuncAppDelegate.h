#import <Cocoa/Cocoa.h>
#import <OpenGL/OpenGL.h>
#import <CoreVideo/CoreVideo.h>
#include "VVGL.hpp"
#import "VVGLBufferGLView.h"




using namespace VVGL;




@interface BasicGLFuncAppDelegate : NSObject <NSApplicationDelegate>	{
	CVDisplayLinkRef			displayLink;
	VVGLContextRef				sharedContext;
	IBOutlet VVGLBufferGLView	*bufferView;
	
	VVGLSceneRef				glScene;	//	this renders-to-texture
	NSDate						*date;	//	used to fade the texture in repeatedly
	VVGLBufferRef				vao;
	GLBufferQuadXYST	lastVBOCoords;	//	the last coords used in the VBO associated with 'vao' (the VAO implicitly retains the VBO, so we only need to update it when the coords change- which we track with this)
}

- (void) initForGL2;
- (void) initForModernGL;

- (void) renderCallback;

@property (retain,readwrite) NSDate *date;
@property (assign,readwrite,setter=setVAO:) VVGLBufferRef vao;

@end




CVReturn displayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp *inNow, const CVTimeStamp *inOutputTime, CVOptionFlags flagsIn, CVOptionFlags *flagsOut, void *displayLinkContext);
