#import <Cocoa/Cocoa.h>
#import "VVGL.hpp"

using namespace VVGL;
using namespace std;

@interface JustTransFeedbackAppDelegate : NSObject <NSApplicationDelegate>	{
	GLContextRef		sharedContext;
	
	GLSceneRef		scene;
	GLBufferRef		srcGeoVBO;
	GLBufferRef		srcColorVBO;
	GLBufferRef		dstGeoVBO;
	GLBufferRef		dstColorVBO;
}

- (void) initForGL2;
- (void) initForModernGL;

- (IBAction) renderClicked:(id)sender;

@property (assign,readwrite) GLBufferRef srcGeoVBO;
@property (assign,readwrite) GLBufferRef srcColorVBO;
@property (assign,readwrite) GLBufferRef dstGeoVBO;
@property (assign,readwrite) GLBufferRef dstColorVBO;

@end

