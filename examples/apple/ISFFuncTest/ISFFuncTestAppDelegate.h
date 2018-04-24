#import <Cocoa/Cocoa.h>
#import <OpenGL/OpenGL.h>
#import <CoreVideo/CoreVideo.h>
#include "VVGL.hpp"
#include "VVISF.h"
#import "VVGLBufferGLView.h"




using namespace VVGL;
using namespace VVISF;




@interface ISFFuncTestAppDelegate : NSObject <NSApplicationDelegate>	{
	CVDisplayLinkRef			displayLink;
	GLContextRef				sharedContext;
	GLBufferRef					srcBuffer;
	
	IBOutlet VVGLBufferGLView	*bufferView;
	
	ISFSceneRef					scene;
	
	IBOutlet NSPopUpButton		*glVersPUB;
}

- (void) loadBackendFromDefaults;

- (IBAction) glVersPUBUsed:(id)sender;
- (IBAction) sliderUsed:(id)sender;

- (void) renderCallback;

@end




CVReturn displayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp *inNow, const CVTimeStamp *inOutputTime, CVOptionFlags flagsIn, CVOptionFlags *flagsOut, void *displayLinkContext);
