#import <Foundation/Foundation.h>
//#import <VVBufferPool/VVBufferPool.h>
#include "VVGL.hpp"
#import <Accelerate/Accelerate.h>
#import "ISFAVFAudioSource.h"
#import "ISFAudioFFT.h"
#import "MutLockArray.h"




extern id				_globalAudioController;
extern NSString			*kAudioControllerInputNameChangedNotification;




using namespace VVGL;




@interface AudioController : NSObject <ISFAVFAudioSourceDelegate>	{
	BOOL					deleted;
	OSSpinLock				audioLock;
	ISFAVFAudioSource		*audioSource;
	MutLockArray			*audioBufferArray;
	ISFAudioBufferList		*rawABL;
	ISFAudioFFT				*audioFFT;
	NSArray					*fftResults;
	
	OSSpinLock				bufferLock;
	GLBufferRef			audioBuffer;
	GLBufferRef			fftBuffer;
}

- (void) prepareToBeDeleted;

- (void) updateAudioResults;

- (NSArray *) arrayOfAudioMenuItems;
- (void) loadDeviceWithUniqueID:(NSString *)n;

- (NSString *) inputName;

- (void) audioInputsChangedNotification:(NSNotification *)note;

- (GLBufferRef) allocAudioImageBuffer;
- (GLBufferRef) allocAudioImageBufferWithWidth:(long)w;
- (GLBufferRef) allocAudioFFTImageBuffer;
- (GLBufferRef) allocAudioFFTImageBufferWithWidth:(long)w;


@end
