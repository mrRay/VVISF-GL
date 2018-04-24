#import "AVCaptureVideoSource.h"




#define VVRELEASE(item) {if (item != nil)	{			\
	[item release];										\
	item = nil;											\
}}

#define MUTDICT [NSMutableDictionary dictionaryWithCapacity:0]
#define MUTARRAY [NSMutableArray arrayWithCapacity:0]


using namespace VVGL;
using namespace VVISF;




@implementation AVCaptureVideoSource


/*===================================================================================*/
#pragma mark --------------------- init/dealloc
/*------------------------------------*/


- (id) init	{
	//NSLog(@"%s",__func__);
	if (self = [super init])	{
		propDeviceInput = nil;
		propSession = nil;
		propOutput = nil;
		propQueue = NULL;
		CVReturn			err = kCVReturnSuccess;
		propGLCtx = nullptr;
		propTextureCache = NULL;
		propLastBuffer = nullptr;
		//NSLog(@"\t\tshared context used for tex cache is %@",[_globalVVBufferPool sharedContext]);
		/*
		err = CVOpenGLTextureCacheCreate(
			NULL,
			NULL,
			[[_globalVVBufferPool sharedContext] CGLContextObj],
			[[GLScene defaultPixelFormat] CGLPixelFormatObj],
			NULL,
			&propTextureCache);
		if (err != kCVReturnSuccess)	{
			NSLog(@"\t\terr %d at CVOpenGLTextureCacheCreate, %s",err,__func__);
		}
		*/
		const GLBufferPoolRef		&bp = GetGlobalBufferPool();
		if (bp == nullptr)	{
			NSLog(@"\t\terr: no global buffer pool, bailing, %s",__func__);
			[self release];
			return nil;
		}
		GLContextRef			poolCtx = (bp==nullptr) ? nullptr : bp->getContext();
		if (poolCtx!=nullptr)	{
			propGLCtx = poolCtx->newContextSharingMe();
			err = CVOpenGLTextureCacheCreate(
				NULL,
				NULL,
				propGLCtx->ctx,
				propGLCtx->pxlFmt,
				NULL,
				&propTextureCache);
			if (err != kCVReturnSuccess)	{
				NSLog(@"\t\terr %d at CVOpenGLTextureCacheCreate, %s",err,__func__);
			}
			
			swizzleScene = CreateISFScene(poolCtx->newContextSharingMe());
			NSString		*pathToSwizzleShader = [[NSBundle mainBundle] pathForResource:@"SwizzleISF-CbY0CrY1toRGB" ofType:@"fs"];
			swizzleScene->useFile(string([pathToSwizzleShader UTF8String]));
		}
		return self;
	}
	[self release];
	return nil;
}
- (void) prepareToBeDeleted	{
	[super prepareToBeDeleted];
}
- (void) dealloc	{
	//NSLog(@"%s",__func__);
	if (!deleted)
		[self prepareToBeDeleted];
	
	OSSpinLockLock(&propLock);
	CVOpenGLTextureCacheRelease(propTextureCache);
	propGLCtx = nullptr;
	//VVRELEASE(propLastBuffer);
	propLastBuffer = nullptr;
	OSSpinLockUnlock(&propLock);
	
	[super dealloc];
}


/*===================================================================================*/
#pragma mark --------------------- superclass overrides
/*------------------------------------*/


- (NSArray *) arrayOfSourceMenuItems	{
	NSArray		*devices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
	if (devices==nil || [devices count]<1)
		return nil;
	NSMutableArray		*returnMe = MUTARRAY;
	for (AVCaptureDevice *devicePtr in devices)	{
		NSMenuItem		*newItem = [[NSMenuItem alloc] initWithTitle:[devicePtr localizedName] action:nil keyEquivalent:@""];
		NSString		*uniqueID = [devicePtr uniqueID];
		[newItem setRepresentedObject:uniqueID];
		[returnMe addObject:newItem];
		[newItem release];
	}
	return returnMe;
}
- (void) _stop	{
	//NSLog(@"%s",__func__);
	if (propSession != nil)	{
		[propSession stopRunning];
		if (propDeviceInput != nil)
			[propSession removeInput:propDeviceInput];
		if (propOutput != nil)
			[propSession removeOutput:propOutput];
		
		dispatch_release(propQueue);
		propQueue = NULL;
		
		[propDeviceInput release];
		propDeviceInput = nil;
		[propOutput release];
		propOutput = nil;
		[propSession release];
		propSession = nil;
	}
	//VVRELEASE(propLastBuffer);
	propLastBuffer = nullptr;
}
- (GLBufferRef) allocBuffer	{
	GLBufferRef		returnMe = nil;
	OSSpinLockLock(&propLock);
	returnMe = propLastBuffer;
	OSSpinLockUnlock(&propLock);
	return returnMe;
}


/*===================================================================================*/
#pragma mark --------------------- misc
/*------------------------------------*/


- (void) loadDeviceWithUniqueID:(NSString *)n	{
	if ([self propRunning])
		[self stop];
	if (n==nil)
		return;
	BOOL				bail = NO;
	NSError				*err = nil;
	OSSpinLockLock(&propLock);
	AVCaptureDevice		*propDevice = [AVCaptureDevice deviceWithUniqueID:n];
	propDeviceInput = (propDevice==nil) ? nil : [[AVCaptureDeviceInput alloc] initWithDevice:propDevice error:&err];
	if (propDeviceInput != nil)	{
		propSession = [[AVCaptureSession alloc] init];
		propOutput = [[AVCaptureVideoDataOutput alloc] init];
		
		if (![propSession canAddInput:propDeviceInput])	{
			NSLog(@"\t\terr: problem adding propDeviceInput in %s",__func__);
			bail = YES;
		}
		if (![propSession canAddOutput:propOutput])	{
			NSLog(@"\t\terr: problem adding propOutput in %s",__func__);
			bail = YES;
		}
		
		if (!bail)	{
			propQueue = dispatch_queue_create([[[NSBundle mainBundle] bundleIdentifier] UTF8String], NULL);
			[propOutput setSampleBufferDelegate:self queue:propQueue];
			
			NSMutableDictionary		*tmpDict = MUTDICT;
			//[tmpDict setObject:NUMBOOL(YES) forKey:(NSString *)kCVPixelBufferOpenGLCompatibilityKey];
			[tmpDict setObject:[NSNumber numberWithInteger:kCVPixelFormatType_422YpCbCr8] forKey:(NSString *)kCVPixelBufferPixelFormatTypeKey];
			//[tmpDict setObject:NUMINT(kCVPixelFormatType_32ARGB) forKey:(NSString *)kCVPixelBufferPixelFormatTypeKey];
			//[tmpDict setObject:[NSNumber numberWithInteger:kCVPixelFormatType_32BGRA] forKey:(NSString *)kCVPixelBufferPixelFormatTypeKey];
			//[tmpDict setObject:NUMINT(kCVPixelFormatType_32ABGR) forKey:(NSString *)kCVPixelBufferPixelFormatTypeKey];
			//[tmpDict setObject:NUMINT(kCVPixelFormatType_32RGBA) forKey:(NSString *)kCVPixelBufferPixelFormatTypeKey];
			//[tmpDict setObject:NUMINT(kCVPixelFormatType_420YpCbCr8Planar) forKey:(NSString *)kCVPixelBufferPixelFormatTypeKey];
			[propOutput setVideoSettings:tmpDict];
			[propOutput setAlwaysDiscardsLateVideoFrames:NO];
			
			[propSession addInput:propDeviceInput];
			[propSession addOutput:propOutput];
			[propSession startRunning];
		}
	}
	else
		bail = YES;
	OSSpinLockUnlock(&propLock);
	
	if (bail)
		[self stop];
	else
		[self start];
}


/*===================================================================================*/
#pragma mark --------------------- AVCaptureVideoDataOutputSampleBufferDelegate protocol (and AVCaptureFileOutputDelegate, too- some protocols share these methods)
/*------------------------------------*/


- (void)captureOutput:(AVCaptureOutput *)o didDropSampleBuffer:(CMSampleBufferRef)b fromConnection:(AVCaptureConnection *)c	{
	NSLog(@"%s",__func__);
}
- (void)captureOutput:(AVCaptureOutput *)o didOutputSampleBuffer:(CMSampleBufferRef)b fromConnection:(AVCaptureConnection *)c	{
	//NSLog(@"%s",__func__);
	/*
	CMFormatDescriptionRef		portFormatDesc = CMSampleBufferGetFormatDescription(b);
	NSLog(@"\t\t\tCMMediaType is %ld, video is %ld",CMFormatDescriptionGetMediaType(portFormatDesc),kCMMediaType_Video);
	NSLog(@"\t\t\tthe FourCharCode for the media subtype is %ld",CMFormatDescriptionGetMediaSubType(portFormatDesc));
	CMVideoDimensions		vidDims = CMVideoFormatDescriptionGetDimensions(portFormatDesc);
	NSLog(@"\t\t\tport size is %d x %d",vidDims.width,vidDims.height);
	*/
	
	OSSpinLockLock(&propLock);
	if (propGLCtx != nullptr)	{
		propGLCtx->makeCurrentIfNotCurrent();
		//	this buffer is either YCbCr/4:2:2, or an RGB texture with half the width of the output image (which contains YCbCr data, and needs to be converted to an RGB image via a shader)
		GLBufferRef			newBuffer = CreateTexRangeFromCMSampleBuffer(b, true);
		//cout << "\tnewBuffer from isight is " << *newBuffer << endl;
		if (newBuffer != nullptr)	{
			if (propGLCtx->version == GLVersion_2)	{
				propLastBuffer = newBuffer;
			}
			else if (propGLCtx->version == GLVersion_4)	{
				VVGL::Size		outputImageSize = VVGL::Size(newBuffer->srcRect.size.width*2., newBuffer->srcRect.size.height);
				//cout << "\toutputImageSize is " << outputImageSize << endl;
				swizzleScene->setFilterInputBuffer(newBuffer);
				//swizzleScene->setBufferForInputNamed(newBuffer, string("inputImage"));
				GLBufferRef		swizzledBuffer = swizzleScene->createAndRenderABuffer(outputImageSize);
				//cout << "\tswizzledBuffer is " << *swizzledBuffer << endl;
				propLastBuffer = swizzledBuffer;
			}
		}
	}
	
	/*
	//	if this came from a connection belonging to the data output
	GLBufferRef			newBuffer = nullptr;
	//CMBlockBufferRef		blockBufferRef = CMSampleBufferGetDataBuffer(b)
	CVImageBufferRef		imgBufferRef = CMSampleBufferGetImageBuffer(b);
	if (imgBufferRef != NULL)	{
		//CGSize		imgBufferSize = CVImageBufferGetDisplaySize(imgBufferRef);
		//NSSizeLog(@"\t\timg buffer size is",imgBufferSize);
		CVOpenGLTextureRef		cvTexRef = NULL;
		CVReturn				err = kCVReturnSuccess;
		
		
		err = CVOpenGLTextureCacheCreateTextureFromImage(NULL,propTextureCache,imgBufferRef,NULL,&cvTexRef);
		if (err != kCVReturnSuccess)	{
			NSLog(@"\t\terr %d at CVOpenGLTextureCacheCreateTextureFromImage() in %s",err,__func__);
		}
		else	{
			//newBuffer = [_globalVVBufferPool allocBufferForCVGLTex:cvTexRef];
			newBuffer = CreateBufferForCVGLTex(cvTexRef);
			if (newBuffer != nil)	{
				//VVRELEASE(propLastBuffer);
				//propLastBuffer = [newBuffer retain];
				propLastBuffer = newBuffer;
				
				//[newBuffer release];
				//newBuffer = nil;
			}
			CVOpenGLTextureRelease(cvTexRef);
		}
	}
	CVOpenGLTextureCacheFlush(propTextureCache,0);
	*/
	OSSpinLockUnlock(&propLock);
	
}


@end
