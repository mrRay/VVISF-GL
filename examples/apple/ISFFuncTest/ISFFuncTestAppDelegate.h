#import <Cocoa/Cocoa.h>
#import <OpenGL/OpenGL.h>
#import <CoreVideo/CoreVideo.h>
#include "VVISF.h"
#import "VVGLBufferGLView.h"




using namespace VVISF;




@interface ISFFuncTestAppDelegate : NSObject <NSApplicationDelegate>	{
	CVDisplayLinkRef			displayLink;
	GLContextRef				sharedContext;
	IBOutlet VVGLBufferGLView	*bufferView;
	
	ISFSceneRef					scene;
	
	IBOutlet NSPopUpButton		*glVersPUB;
}

- (void) loadBackendFromDefaults;

- (IBAction) glVersPUBUsed:(id)sender;

- (void) renderCallback;

@end




CVReturn displayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp *inNow, const CVTimeStamp *inOutputTime, CVOptionFlags flagsIn, CVOptionFlags *flagsOut, void *displayLinkContext);
