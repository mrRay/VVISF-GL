#import <Cocoa/Cocoa.h>
#include "ISFKit.h"
#import "VVGLBufferGLView.h"
#import "ISFVVGLBufferView.h"




using namespace VVISF;




@interface AppDelegate : NSObject <NSApplicationDelegate>	{
	CVDisplayLinkRef			displayLink;
	VVGLContextRef				sharedContext;
	VVGLSceneRef				scene;
	VVGLBufferRef				vao;
	
	VVGLBufferRef				lastRenderedBuffer;
	VVGLBufferRef				lastCopiedBuffer;
	
	IBOutlet ISFVVGLBufferView		*glView;
}

- (void) loadBackendFromDefaults;

- (void) initGL2;
- (void) initGL4;

@property (assign,readwrite,setter=setVAO:) VVGLBufferRef vao;

- (void) renderCallback;

- (IBAction) flushClicked:(id)sender;
- (IBAction) renderClicked:(id)sender;

@end




CVReturn displayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp *inNow, const CVTimeStamp *inOutputTime, CVOptionFlags flagsIn, CVOptionFlags *flagsOut, void *displayLinkContext);







