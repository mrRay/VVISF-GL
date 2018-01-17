#import <Cocoa/Cocoa.h>
#import "VVGL.hpp"
#import "VVGLBufferGLView.h"


using namespace VVGL;

@interface TransFeedbackAppDelegate : NSObject <NSApplicationDelegate>	{
	CVDisplayLinkRef			displayLink;
	VVGLContextRef				sharedContext;
	IBOutlet VVGLBufferGLView	*bufferView;
	
	VVGLSceneRef				feedbackScene;
	VVGLSceneRef				rasterScene;	//	this renders-to-texture
	NSDate						*date;	//	used to fade the texture in repeatedly
	//VVGLBufferRef				vao;
	Quad<VertXYZRGBA>			lastVBOCoords;	//	the last coords used in the VBO associated with 'vao' (the VAO implicitly retains the VBO, so we only need to update it when the coords change- which we track with this)
	VVGLBufferRef				vertVBO;
	VVGLBufferRef				feedbackVBO;
}

- (void) initForGL2;
- (void) initForModernGL;

- (void) renderCallback;

@property (readonly) NSDate * date;
//@property (assign,readwrite,setter=setVAO:) VVGLBufferRef vao;
@property (assign,readwrite) VVGLBufferRef vertVBO;
@property (assign,readwrite) VVGLBufferRef feedbackVBO;

@end




CVReturn displayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp *inNow, const CVTimeStamp *inOutputTime, CVOptionFlags flagsIn, CVOptionFlags *flagsOut, void *displayLinkContext);
