#import "ISFQLRendererAgent.h"
#import "ISFQL_RendererAppDelegate.h"

#include "VVISF.hpp"
#include "VVGLContextCacheItem.h"




#define LOCK OSSpinLockLock
#define UNLOCK OSSpinLockUnlock




@interface ISFQLRendererAgent ()
- (void) resetTTLTimer;
@end




@implementation ISFQLRendererAgent


- (id) init	{
	//NSLog(@"%s ... %p",__func__,self);
	if (self = [super init])	{
		deleted = NO;
		connLock = OS_SPINLOCK_INIT;
		conn = nil;
		timerLock = OS_SPINLOCK_INIT;
		ttlTimer = nil;
		delegateLock = OS_SPINLOCK_INIT;
		delegate = nil;
		return self;
	}
	[self release];
	return nil;
}
- (void) prepareToBeDeleted	{
	if (deleted)
		return;
	//NSLog(@"%s ... %p",__func__,self);
	
	deleted = YES;
	
	//	kill the timer
	LOCK(&timerLock);
	if (ttlTimer != nil)	{
		[ttlTimer invalidate];
		ttlTimer = nil;
	}
	UNLOCK(&timerLock);
	
	//	kill the connection
	LOCK(&connLock);
	[conn invalidate];
	conn = nil;
	UNLOCK(&connLock);
	
	//	tell the delegate that i'm dying
	id			myDelegate = [self delegate];
	if (myDelegate != nil)
		[myDelegate agentKilled:self];
	
	//	release myself (not a bug!)
	[self autorelease];
}
- (void) dealloc	{
	//NSLog(@"%s ... %p",__func__,self);
	if (!deleted)
		[self prepareToBeDeleted];
	
	[super dealloc];
}


#pragma mark XPC backend


- (void) setConn:(NSXPCConnection *)n	{
	//NSLog(@"%s ... %p",__func__,self);
	OSSpinLockLock(&connLock);
	conn = n;
	OSSpinLockUnlock(&connLock);
	
	[n setInvalidationHandler:^()	{
		NSLog(@"err: %@ invalidation handler",[self className]);
	}];
	[n setInterruptionHandler:^()	{
		NSLog(@"err: %@ interruption handler",[self className]);
	}];
}


#pragma mark TTL timer stuff


//	this is a timeout method- if it gets called, something went wrong and i need to kill this agent.
- (void) ttlTimer:(NSTimer *)t	{
	//NSLog(@"%s ... %p",__func__,self);
	if (deleted)
		return;
	
	LOCK(&timerLock);
	ttlTimer = nil;
	UNLOCK(&timerLock);
	
	//	get the ROP, and pass empty bitmap data back to it
	LOCK(&connLock);
	id<ISFQLService>	rop = (conn==nil) ? nil : [conn remoteObjectProxy];
	UNLOCK(&connLock);
	
	if (rop != nil)	{
		NSLog(@"\t\terr: timed out, passing back empty frame, %s",__func__);
		[rop renderedBitmapData:[NSData data] sized:NSMakeSize(0,0)];
	}
	else	{
		NSLog(@"\t\terr: timed out, but ROP was nil, %s",__func__);
	}
	
	//	prepping for deletion kills the connection & timer and releases myself
	[self prepareToBeDeleted];
}
- (void) resetTTLTimer	{
	//NSLog(@"%s ... %p",__func__,self);
	LOCK(&timerLock);
	if (ttlTimer != nil)	{
		[ttlTimer invalidate];
		ttlTimer = nil;
	}
	UNLOCK(&timerLock);
	
	dispatch_async(dispatch_get_main_queue(), ^{
		if (!deleted)	{
			LOCK(&timerLock);
			ttlTimer = [NSTimer
				scheduledTimerWithTimeInterval:3.
				target:self
				selector:@selector(ttlTimer:)
				userInfo:nil
				repeats:NO];
			UNLOCK(&timerLock);
		}
	});
}


#pragma mark ISFQLAgentService


- (void) renderThumbnailForPath:(NSString *)n sized:(NSSize)s	{
	//NSLog(@"%s ... %@, (%0.2f, %0.2f)",__func__,n,s.width,s.height);
	//NSLog(@"%s ... %p",__func__,self);
	using namespace VVGL;
	using namespace VVISF;
	
	//	start a render timer- if the frame takes longer than a couple seconds, bail and return
	[self resetTTLTimer];
	
	
	
	//	get a cache item, which will contain all the gl resources necessary to create our ISF scene and render it
	VVGLContextCacheItemRef		cache = GetCacheItem();
	if (cache == nullptr)	{
		NSLog(@"\terr: couldn't create GL cache item, returning empty frame");
		LOCK(&connLock);
		id<ISFQLService>	rop = (conn==nil) ? nil : [conn remoteObjectProxy];
		UNLOCK(&connLock);
		if (rop != nil)
			[rop renderedBitmapData:[NSData data] sized:NSMakeSize(0,0)];
	}
	
	//	actually render the frame...
	GLBufferPoolRef		bp = nullptr;
	GLTexToCPUCopierRef		downloader = nullptr;
	GLTexToTexCopierRef		copier = nullptr;
	VVGL::Size			renderSize = VVGL::Size(s.width,s.height);
	ISFSceneRef			scene = nullptr;
	GLBufferRef			colorBars = nullptr;
	GLBufferRef			invColorBars = nullptr;
	GLBufferRef			renderBuffer = nullptr;
	GLBufferRef			postProcBuffer = nullptr;
	GLBufferRef			cpuBuffer = nullptr;
	bool				problemRendering = false;
	
	//	first try rendering using GL2
	try	{
		//NSLog(@"\ttrying to render with GL2...");
		bp = cache->getGL2Pool();
		downloader = cache->getGL2Downloader();
		copier = cache->getGL2Copier();
		colorBars = cache->getGL2ColorBars();
		invColorBars = cache->getGL2InvColorBars();
		
		scene = CreateISFSceneRefUsing(cache->getGL2Context()->newContextSharingMe());
		scene->setThrowExceptions(true);
		scene->setPrivatePool(bp);
		scene->setPrivateCopier(copier);
		scene->useFile(string([n UTF8String]), true, true);
		
		ISFDocRef			tmpDoc = scene->doc();
		if (tmpDoc == nullptr)
			throw ISFErr(ISFErrType_ErrorCompilingGLSL, "Shader Problem", "no doc found!");
		
		switch (tmpDoc->type())	{
		case ISFFileType_None:
		case ISFFileType_Source:
			//	no action necessary
			break;
		case ISFFileType_Filter:
			//	pass the color bars to the image filter input
			scene->setFilterInputBuffer(colorBars);
			break;
		case ISFFileType_Transition:
			//	pass the transition images to the scene, set its progress to 0.5
			scene->setBufferForInputImageKey(colorBars, "startImage");
			scene->setBufferForInputImageKey(invColorBars, "endImage");
			scene->setValueForInputNamed(ISFFloatVal(0.5), "progress");
			break;
		}
		//	render a frame
		renderBuffer = CreateRGBATex(renderSize, false, bp);
		scene->renderToBuffer(renderBuffer);
		renderBuffer->flipped = !renderBuffer->flipped;
		
		//	post-process the frame (flip it upside down, render it on top of opaque black)
		postProcBuffer = CreateRGBATex(renderSize, false, bp);
		scene->useFile(string([[[NSBundle mainBundle] pathForResource:@"PostProc" ofType:@"fs"] UTF8String]));
		scene->setFilterInputBuffer(renderBuffer);
		scene->renderToBuffer(postProcBuffer);
		
		//	download the swizzled buffer to a CPU buffer
		cpuBuffer = CreateRGBACPUBuffer(renderSize, bp);
		downloader->downloadTexToCPU(postProcBuffer, cpuBuffer);
		
	}
	catch (...)	{
		//	if we're here, GL2 rendering failed for some reason, so we need to try GL4
		try	{
			NSLog(@"\terr rendering with GL2, trying to render with GL4...");
			bp = cache->getGL4Pool();
			downloader = cache->getGL4Downloader();
			copier = cache->getGL4Copier();
			colorBars = cache->getGL4ColorBars();
			invColorBars = cache->getGL4InvColorBars();
		
			scene = nullptr;
			scene = CreateISFSceneRefUsing(cache->getGL4Context()->newContextSharingMe());
			scene->setThrowExceptions(true);
			scene->setPrivatePool(bp);
			scene->setPrivateCopier(copier);
			scene->useFile(string([n UTF8String]), true, true);
			
			ISFDocRef			tmpDoc = scene->doc();
			if (tmpDoc == nullptr)
				throw ISFErr(ISFErrType_ErrorCompilingGLSL, "Shader Problem", "no doc found!");
		
			switch (tmpDoc->type())	{
			case ISFFileType_None:
			case ISFFileType_Source:
				//	no action necessary
				break;
			case ISFFileType_Filter:
				//	pass the color bars to the image filter input
				scene->setFilterInputBuffer(colorBars);
				break;
			case ISFFileType_Transition:
				//	pass the transition images to the scene, set its progress to 0.5
				scene->setBufferForInputImageKey(colorBars, "startImage");
				scene->setBufferForInputImageKey(invColorBars, "endImage");
				scene->setValueForInputNamed(ISFFloatVal(0.5), "progress");
				break;
			}
			//	render a frame
			renderBuffer = CreateRGBATex(renderSize, false, bp);
			scene->renderToBuffer(renderBuffer);
			renderBuffer->flipped = !renderBuffer->flipped;
			
			//	post-process the frame (flip it upside down, render it on top of opaque black)
			postProcBuffer = CreateRGBATex(renderSize, false, bp);
			scene->useFile(string([[[NSBundle mainBundle] pathForResource:@"PostProc" ofType:@"fs"] UTF8String]));
			scene->setFilterInputBuffer(renderBuffer);
			scene->renderToBuffer(postProcBuffer);
			
			//	download the swizzled buffer to a CPU buffer
			cpuBuffer = CreateRGBACPUBuffer(renderSize, bp);
			downloader->downloadTexToCPU(postProcBuffer, cpuBuffer);
		}
		catch (...)	{
			problemRendering = true;
		}
	}
	
	
	//	if i had a problem rendering, return an empty image immeidately
	if (problemRendering)	{
		NSLog(@"\t\terr: there was a problem, passing back an empty frame");
		LOCK(&connLock);
		id<ISFQLService>	rop = (conn==nil) ? nil : [conn remoteObjectProxy];
		UNLOCK(&connLock);
		if (rop != nil)
			[rop renderedBitmapData:[NSData data] sized:NSMakeSize(0,0)];
	}
	//	else rendering happened without issue- download the image we rendered, pass it back to the ROP
	else	{
		//NSLog(@"\t\tcpuBuffer is %s",cpuBuffer->getDescriptionString().c_str());
		unsigned long			cpuBufferSize = cpuBuffer->desc.backingLengthForSize(renderSize);
		if (cpuBuffer->cpuBackingPtr==nullptr)
			cpuBufferSize = 0;
		//NSLog(@"\t\tcpuBufferSize is %ld, backing ptr is %p",cpuBufferSize,[cpuBuffer cpuBackingPtr]);
		NSData				*cpuBufferData = [NSData dataWithBytes:cpuBuffer->cpuBackingPtr length:cpuBufferSize];
		
		LOCK(&connLock);
		id<ISFQLService>	rop = (conn==nil) ? nil : [conn remoteObjectProxy];
		UNLOCK(&connLock);
		if (!deleted && rop!=nil)	{
			//NSLog(@"\t\tfinished rendering, passing bitmap data back from agent, %s",__func__);
			[rop renderedBitmapData:cpuBufferData sized:NSMakeSize(renderSize.width,renderSize.height)];
		}
		else	{
			NSLog(@"\t\terr: finished rendering, but ROP is nil- nothing to pass back, %s",__func__);
		}
	}
	
	//	return the cache item to the pool
	ReturnCacheItemToPool(cache);
	cache = nullptr;
	
	//	prepping for deletion kills the connection & timer and releases myself
	[self prepareToBeDeleted];
	
	
}


#pragma mark key/value


- (void) setDelegate:(id<ISFQLRendererAgentDelegate>)n	{
	LOCK(&delegateLock);
	delegate = n;
	UNLOCK(&delegateLock);
}
- (id<ISFQLRendererAgentDelegate>) delegate	{
	id<ISFQLRendererAgentDelegate>		returnMe = nil;
	LOCK(&delegateLock);
	returnMe = delegate;
	UNLOCK(&delegateLock);
	return returnMe;
}


@end
