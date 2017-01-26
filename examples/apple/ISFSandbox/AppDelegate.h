#import <Cocoa/Cocoa.h>
#include "ISFKit.h"
#import "VVGLBufferGLView.h"




@interface AppDelegate : NSObject <NSApplicationDelegate>	{
	VVGLContext		*sharedCtx;
	
	VVISF::VVGLScene			*scene;
	VVISF::ISFScene			*isfScene;
	
	GLuint						texture;	//	we're going to create this GL texture, upload an image to it, and then create a VVGLBuffer from it
	NSSize						textureSize;	//	populated when we upload the image to the texture- this is the size of the texture
	
	IBOutlet VVGLBufferGLView		*glView;
}

- (IBAction) buttonClicked:(id)sender;
- (IBAction) flushClicked:(id)sender;

- (IBAction) renderToTexture:(id)sender;
- (IBAction) drawInOutput:(id)sender;

@end






