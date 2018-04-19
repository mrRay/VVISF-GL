#import <Foundation/Foundation.h>
#import "VideoSource.h"




@interface IMGVideoSource : VideoSource	{
	//VVBuffer					*propLastBuffer;
	GLBufferRef			propLastBuffer;
}

- (void) loadFileAtPath:(NSString *)p;

@end
