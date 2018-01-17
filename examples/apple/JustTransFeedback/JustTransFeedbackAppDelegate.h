#import <Cocoa/Cocoa.h>
#import "VVGL.hpp"

using namespace VVGL;
using namespace std;

@interface JustTransFeedbackAppDelegate : NSObject <NSApplicationDelegate>	{
	VVGLContextRef		sharedContext;
	
	VVGLSceneRef		scene;
	VVGLBufferRef		srcGeoVBO;
	VVGLBufferRef		srcColorVBO;
	VVGLBufferRef		dstGeoVBO;
	VVGLBufferRef		dstColorVBO;
}

- (void) initForGL2;
- (void) initForModernGL;

- (IBAction) renderClicked:(id)sender;

@property (assign,readwrite) VVGLBufferRef srcGeoVBO;
@property (assign,readwrite) VVGLBufferRef srcColorVBO;
@property (assign,readwrite) VVGLBufferRef dstGeoVBO;
@property (assign,readwrite) VVGLBufferRef dstColorVBO;

@end

