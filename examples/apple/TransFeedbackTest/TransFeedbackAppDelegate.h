#import <Cocoa/Cocoa.h>
#import "VVGL.hpp"
#import "VVGLBufferGLView.h"


using namespace VVGL;

@interface TransFeedbackAppDelegate : NSObject <NSApplicationDelegate>	{
	CVDisplayLinkRef			displayLink;
	GLContextRef				sharedContext;
	IBOutlet VVGLBufferGLView	*bufferView;
	
	GLSceneRef				feedbackScene;
	GLSceneRef				rasterScene;	//	this renders-to-texture
	NSDate						*date;	//	used to fade the texture in repeatedly
	//GLBufferRef				vao;
	Quad<VertXYZRGBA>			lastVBOCoords;	//	the last coords used in the VBO associated with 'vao' (the VAO implicitly retains the VBO, so we only need to update it when the coords change- which we track with this)
	GLBufferRef				vertVBO;
	GLBufferRef				feedbackVBO;
}

- (void) initForGL2;
- (void) initForModernGL;

- (void) renderCallback;

@property (readonly) NSDate * date;
//@property (assign,readwrite,setter=setVAO:) GLBufferRef vao;
@property (assign,readwrite) GLBufferRef vertVBO;
@property (assign,readwrite) GLBufferRef feedbackVBO;

@end




CVReturn displayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp *inNow, const CVTimeStamp *inOutputTime, CVOptionFlags flagsIn, CVOptionFlags *flagsOut, void *displayLinkContext);
