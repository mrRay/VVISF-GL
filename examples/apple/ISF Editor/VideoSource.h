#import <Foundation/Foundation.h>
//#import <VVBufferPool/VVBufferPool.h>
//#import <VVISFKit/VVISFKit.h>
#include "ISFKit.h"



@protocol VideoSourceDelegate
- (void) listOfStaticSourcesUpdated:(id)ds;
@end




@interface VideoSource : NSObject	{
	BOOL			deleted;
	
	//OSSpinLock		lastBufferLock;
	//VVBuffer		*lastBuffer;
	
	OSSpinLock		propLock;
	BOOL			propRunning;
	id <VideoSourceDelegate>	propDelegate;
}

- (void) prepareToBeDeleted;
- (VVGLBufferRef) allocBuffer;
- (NSArray *) arrayOfSourceMenuItems;

- (void) start;
- (void) _start;
- (void) stop;
- (void) _stop;

//- (void) render;
//- (void) _render;

- (BOOL) propRunning;
- (void) setPropDelegate:(id<VideoSourceDelegate>)n;

@end
