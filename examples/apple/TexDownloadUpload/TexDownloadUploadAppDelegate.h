#import <Cocoa/Cocoa.h>
#import <OpenGL/OpenGL.h>
#import <CoreVideo/CoreVideo.h>
#include "VVGL.hpp"
#import "VVGLBufferGLView.h"




@interface TexDownloadUploadAppDelegate : NSObject <NSApplicationDelegate>	{
	CVDisplayLinkRef			displayLink;
	GLContextRef				sharedContext;
	IBOutlet VVGLBufferGLView	*bufferView;
	
	GLSceneRef				glScene;
	GLTexToCPUCopierRef		downloader;
	GLCPUToTexCopierRef			uploader;
	
	GLBufferRef		imageCPUBuffer;
	
	int				renderStep;
	GLBufferRef		srcBuffer;
	GLBufferRef		pboBuffer;
	GLBufferRef		cpuBuffer;
	GLBufferRef		targetBuffer;
}

- (void) renderCallback;
- (IBAction) renderStepClicked:(id)sender;

@end




CVReturn displayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp *inNow, const CVTimeStamp *inOutputTime, CVOptionFlags flagsIn, CVOptionFlags *flagsOut, void *displayLinkContext);
