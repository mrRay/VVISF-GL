#import "IMGVideoSource.h"
#import <AppKit/AppKit.h>
#import "VVGLBufferPool_CocoaAdditions.h"




@implementation IMGVideoSource


/*===================================================================================*/
#pragma mark --------------------- init/dealloc
/*------------------------------------*/


- (id) init	{
	if (self = [super init])	{
		propLastBuffer = nullptr;
		//lastBufferLock = OS_SPINLOCK_INIT;
		//lastBuffer = nil;
		return self;
	}
	[self release];
	return nil;
}
- (void) prepareToBeDeleted	{
	[super prepareToBeDeleted];
}
- (void) dealloc	{
	if (!deleted)
		[self prepareToBeDeleted];
	OSSpinLockLock(&propLock);
	//VVRELEASE(propLastBuffer);
	propLastBuffer = nullptr;
	OSSpinLockUnlock(&propLock);
	[super dealloc];
}


/*===================================================================================*/
#pragma mark --------------------- superclass overrides
/*------------------------------------*/


- (void) loadFileAtPath:(NSString *)p	{
	NSLog(@"%s ... %@",__func__,p);
	if (p==nil)
		return;
	NSImage		*img = [[NSImage alloc] initWithContentsOfFile:p];
	if (img==nil)	{
		NSLog(@"\t\terr: couldn't make NSImage from path \"%@\"",p);
		return;
	}
	
	//VVBuffer		*newBuffer = [_globalVVBufferPool allocBufferForNSImage:img];
	VVGLBufferRef	newBuffer = CreateBufferForNSImage(img);
	if (newBuffer==nullptr)	{
		NSLog(@"\t\terr: couldn't make VVGLBuffer from NSImage in %s",__func__);
	}
	else	{
		//[newBuffer setFlipped:YES];
		newBuffer->flipped = true;
		
		OSSpinLockLock(&propLock);
		//VVRELEASE(propLastBuffer);
		propLastBuffer = newBuffer;
		OSSpinLockUnlock(&propLock);
	}
	
	[img release];
	img = nil;
}
- (void) _stop	{
	//VVRELEASE(propLastBuffer);
	propLastBuffer = nullptr;
}
- (VVGLBufferRef) allocBuffer	{
	//VVBuffer		*returnMe = nil;
	VVGLBufferRef	returnMe = nullptr;
	OSSpinLockLock(&propLock);
	//returnMe = (propLastBuffer==nil) ? nil : [propLastBuffer retain];
	returnMe = propLastBuffer;
	OSSpinLockUnlock(&propLock);
	return returnMe;
}


@end
