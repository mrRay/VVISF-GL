#import <Foundation/Foundation.h>
#import "VideoSource.h"
#import <AVFoundation/AVFoundation.h>




@interface AVCaptureVideoSource : VideoSource <AVCaptureVideoDataOutputSampleBufferDelegate>	{
	AVCaptureDeviceInput		*propDeviceInput;
	AVCaptureSession			*propSession;
	AVCaptureVideoDataOutput	*propOutput;
	dispatch_queue_t			propQueue;
	VVGLContextRef				propGLCtx;
	CVOpenGLTextureCacheRef		propTextureCache;
	VVGLBufferRef				propLastBuffer;
	
	//OSSpinLock					lastBufferLock;
	//VVBuffer					*lastBuffer;
}

- (void) loadDeviceWithUniqueID:(NSString *)n;

@end
