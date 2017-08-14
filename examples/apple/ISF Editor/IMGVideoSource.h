#import <Foundation/Foundation.h>
#import "VideoSource.h"




@interface IMGVideoSource : VideoSource	{
	//VVBuffer					*propLastBuffer;
	VVGLBufferRef			propLastBuffer;
}

- (void) loadFileAtPath:(NSString *)p;

@end
