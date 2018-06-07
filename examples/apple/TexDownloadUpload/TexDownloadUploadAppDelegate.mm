#import "TexDownloadUploadAppDelegate.h"




@interface TexDownloadUploadAppDelegate ()
@property (weak) IBOutlet NSWindow *window;
@end




@implementation TexDownloadUploadAppDelegate


- (id) init	{
	self = [super init];
	if (self != nil)	{
		
		renderStep = 10;
		imageCPUBuffer = nullptr;
		srcBuffer = nullptr;
		pboBuffer = nullptr;
		cpuBuffer = nullptr;
		targetBuffer = nullptr;
		
		//	make the shared context using the vsn of GL you need to target.  all GL contexts are going to share this so they can share textures/etc with one another
		sharedContext = CreateNewGLContextRef(NULL, CreateCompatibilityGLPixelFormat());
		//sharedContext = CreateNewGLContextRef(NULL, CreateGL4PixelFormat());
		
		//	make the global buffer pool.  if there's a global buffer pool, calls to create textures/etc will be shorter.  the global buffer pool will use the shared context to create any GL resources
		CreateGlobalBufferPool(sharedContext);
		
		//	make the GL scene we're going to use to render to texture (the texture will then be drawn in a view)
		glScene = CreateGLSceneRefUsing(sharedContext->newContextSharingMe());
		
		//	make the downloader and uploader
		downloader = CreateGLTexToCPUCopierRefUsing(sharedContext->newContextSharingMe());
		uploader = CreateGLCPUToTexCopierRefUsing(sharedContext->newContextSharingMe());
		
		//	load the included image
		//NSImage			*tmpImg = [[NSImage alloc] initWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"SampleImg" ofType:@"png"]];
		NSImage			*tmpImg = [[NSImage alloc] initWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"BigImg" ofType:@"png"]];
		if (tmpImg == nil)
			NSLog(@"\t\tERR: couldn't load image from file, %s",__func__);
		else	{
			
			
			
			
			NSSize				origImageSize = [tmpImg size];
			NSRect				origImageRect = NSMakeRect(0, 0, origImageSize.width, origImageSize.height);
			NSImageRep			*bestRep = [tmpImg bestRepresentationForRect:origImageRect context:nil hints:nil];
			NSSize				bitmapSize = NSMakeSize([bestRep pixelsWide], [bestRep pixelsHigh]);
			if (bitmapSize.width==0 || bitmapSize.height==0)
				bitmapSize = [tmpImg size];
			NSRect				bitmapRect = NSMakeRect(0,0,bitmapSize.width,bitmapSize.height);
	
			//	make a bitmap image rep
			NSBitmapImageRep		*rep = [[NSBitmapImageRep alloc]
				initWithBitmapDataPlanes:nil
				pixelsWide:bitmapSize.width
				pixelsHigh:bitmapSize.height
				bitsPerSample:8
				samplesPerPixel:4
				hasAlpha:YES
				isPlanar:NO
				colorSpaceName:NSCalibratedRGBColorSpace
				bitmapFormat:0
				bytesPerRow:32 * bitmapSize.width / 8
				bitsPerPixel:32];
			if (rep != nil)	{
				//	save the current NSGraphicsContext, make a new one based on the bitmap image rep i just created
				NSGraphicsContext		*origContext = [NSGraphicsContext currentContext];
				if (origContext != nil)
					[origContext retain];
				NSGraphicsContext		*newContext = [NSGraphicsContext graphicsContextWithBitmapImageRep:rep];
				if (newContext != nil)	{
					//	set up & start drawing in the new graphics context (draws into the bitmap image rep)
					[NSGraphicsContext setCurrentContext:newContext];
					[newContext setShouldAntialias:NO];
		
					//[[NSColor colorWithDeviceRed:0.0 green:0.0 blue:0.0 alpha:1.0] set];
					//VVRECTFill(bitmapRect);
					[tmpImg
						drawInRect:bitmapRect
						fromRect:origImageRect
						operation:NSCompositeCopy
						fraction:1.0];
		
					//	flush the graphics
					[newContext flushGraphics];
				}
				[NSGraphicsContext setCurrentContext:origContext];
				if (origContext != nil)	{
					[origContext release];
					origContext = nil;
				}
	
				//	the bitmap rep we just drew into was premultiplied, and we have to fix that before uploading it
				[rep unpremultiply];
			
				//	make a CPU-based GLBufferRef from the rep's pixel data
				imageCPUBuffer = CreateRGBACPUBufferUsing(
					VVGL::Size(bitmapSize.width, bitmapSize.height),
					(void*)[rep bitmapData],
					VVGL::Size(bitmapSize.width, bitmapSize.height),
					(void*)rep,
					[](GLBuffer & inBuffer, void * inBackingReleaseContext)	{
						if (inBackingReleaseContext == nullptr)
							return;
						NSBitmapImageRep		*tmpRep = (NSBitmapImageRep *)inBackingReleaseContext;
						[tmpRep release];
					});
				imageCPUBuffer->flipped = true;
				
				cout << "\timageCPUBuffer is ";
				if (imageCPUBuffer == nullptr)
					cout << "null\n";
				else
					cout << *imageCPUBuffer << endl;
				
				//	...do NOT release the bitmap rep- it's retained implicitly by the CPU-based buffer, which will release the rep in its backing release callback.
			}
			
			[tmpImg release];
			tmpImg = nil;
		}
	}
	return self;
}


- (void)applicationDidFinishLaunching:(NSNotification *)aNotification	{
	//	pass the shared context to the buffer view (view has its own ctx, so it needs to be in the same sharegroup to be able to draw the passed texture)
	[bufferView setSharedGLContext:sharedContext];
	[bufferView setRetainDraw:YES];
	
	//	make the displaylink, which will drive rendering
	CVReturn				err = kCVReturnSuccess;
	CGOpenGLDisplayMask		totalDisplayMask = 0;
	GLint					virtualScreen = 0;
	GLint					displayMask = 0;
	NSOpenGLPixelFormat		*format = [[[NSOpenGLPixelFormat alloc] initWithCGLPixelFormatObj:sharedContext->pxlFmt] autorelease];
	
	for (virtualScreen=0; virtualScreen<[format numberOfVirtualScreens]; ++virtualScreen)	{
		[format getValues:&displayMask forAttribute:NSOpenGLPFAScreenMask forVirtualScreen:virtualScreen];
		totalDisplayMask |= displayMask;
	}
	err = CVDisplayLinkCreateWithOpenGLDisplayMask(totalDisplayMask, &displayLink);
	if (err)	{
		NSLog(@"\t\terr %d creating display link in %s",err,__func__);
		displayLink = NULL;
	}
	else	{
		CVDisplayLinkSetOutputCallback(displayLink, displayLinkCallback, self);
		CVDisplayLinkStart(displayLink);
	}
}
//	this method is called from the displaylink callback
- (void) renderCallback	{
	//NSLog(@"%s",__func__);
	
	using namespace VVGL;
	
	//	this bit of code here streams a CPU-based image buffer created on app launch to textures
	
	GLBufferRef		tmpTex = uploader->streamCPUToTex(imageCPUBuffer);
	//GLBufferRef		tmpTex = uploader->uploadCPUToTex(imageCPUBuffer);
	//if (tmpTex == nullptr)
	//	cout << "\ttmpTex was null\n";
	//else
	//	cout << "\ttmpTex was " << *tmpTex << endl;
	
	[bufferView drawBuffer:tmpTex];
	
	//	tell the buffer pool to do its housekeeping (releases any "old" resources in the pool that have been sticking around for a while)
	GetGlobalBufferPool()->housekeeping();
	
	
	
	
	
	//	this bit of code here fills a texture with solid red, downloads it (streams it) to a CPU-based 
	//	buffer, which it then streams back to another GL texture.  This new texture is displayed below.
	/*
	//	make some resources to render into
	GLBufferRef		fbo = CreateFBO();
	GLBufferRef		tmpTex = CreateBGRATex(VVGL::Size(1920,1080));
	GLBufferRef		tmpCPU = CreateBGRACPUBuffer(tmpTex->size);
	//cout << "\ttmpTex is " << *tmpTex << endl;
	GLScene::RenderTarget	target = GLScene::RenderTarget(fbo, tmpTex, nullptr);
	glScene->renderRedFrame(target);
	//	stream the texture we just rendered into to the downloader
	GLBufferRef		tmpDownloaded = downloader->streamTexToCPU(tmpTex, tmpCPU, true);
	if (tmpDownloaded == nullptr)
		cout << "\terr: no buffer downloaded!\n";
	else
		cout << "\ttmpDownloaded is " << *tmpDownloaded << endl;
	
	//	stream the cpu back up
	GLBufferRef		tmpUploaded = uploader->streamCPUToTex(tmpDownloaded);
	if (tmpUploaded == nullptr)
		cout << "\terr: no tex re-uploaded\n";
	else
		cout << "\tre-uploaded text " << *tmpUploaded << endl;
	targetBuffer = tmpUploaded;
	
	[bufferView drawBuffer:targetBuffer];
	
	//	tell the buffer pool to do its housekeeping (releases any "old" resources in the pool that have been sticking around for a while)
	GetGlobalBufferPool()->housekeeping();
	*/
	
	
	
	
	//	this bit of code here displays 'targetBuffer', which is used with the render steps!
	/*
	[bufferView drawBuffer:targetBuffer];
	//	tell the buffer pool to do its housekeeping (releases any "old" resources in the pool that have been sticking around for a while)
	GetGlobalBufferPool()->housekeeping();
	*/
	
	
	
	
	//	this bit of code here just fills a texture with solid red and displays it.  no texture upload or download.
	/*
	using namespace VVGL;
	
	//	make some resources to render into
	GLBufferRef		fbo = CreateFBO();
	GLBufferRef		origTex = CreateRGBATex(VVGL::Size(64,64));
	GLScene::RenderTarget	target = GLScene::RenderTarget(fbo, origTex, nullptr);
	glScene->renderRedFrame(target);
	
	//	draw the GL texture i just rendered in the buffer view
	[bufferView drawBuffer:origTex];
	//	tell the buffer pool to do its housekeeping (releases any "old" resources in the pool that have been sticking around for a while)
	GetGlobalBufferPool()->housekeeping();
	*/
}

//	this function increments 'renderStep', performing a specific set of GL operations at each step.
//	it is convenient for stepping through a procedure and debugging things.
- (IBAction) renderStepClicked:(id)sender	{
	NSLog(@"%s",__func__);
	NSLog(@"\t\tperforming render step %d",renderStep);
	
	switch (renderStep)	{
	case 0:
		{
			GLBufferRef		fbo = CreateFBO();
			srcBuffer = CreateRGBATex(VVGL::Size(64,64));
			GLScene::RenderTarget	target = GLScene::RenderTarget(fbo, srcBuffer, nullptr);
			glScene->renderRedFrame(target);
			cout << "\tsrcBuffer is " << *srcBuffer << endl;
		}
		break;
	case 1:
		{
			cpuBuffer = CreateRGBACPUBuffer(srcBuffer->size);
			pboBuffer = downloader->downloadTexToCPU(srcBuffer, cpuBuffer);
			cout << "\tpboBuffer is " << *pboBuffer << endl;
			cout << "\tpboBuffer is " << ((pboBuffer->pboMapped)?"mapped":"not mapped") << endl;
			uint8_t		*basePtr = (uint8_t*)pboBuffer->cpuBackingPtr;
			cout << "\tpboBuffer's basePtr is " << (void*)basePtr << endl;
			
			basePtr = (uint8_t *)cpuBuffer->cpuBackingPtr;
			cout << "\tcpuBuffer's basePtr is " << (void*)basePtr << endl;
			
			for (int i=0; i<4; ++i)	{
				if (i==0)
					cout << "\tcolors are " << (int)*basePtr;
				else
					cout << ", " << (int)*basePtr;
				++basePtr;
			}
			
		}
		break;
	case 2:
		{
			targetBuffer = uploader->uploadCPUToTex(cpuBuffer);
			cout << "\ttargetBuffer is " << *targetBuffer << endl;
		}
		break;
	
	
	
	case 10:
		{
			targetBuffer = uploader->uploadCPUToTex(imageCPUBuffer);
			cout << "\ttargetBuffer is " << *targetBuffer << endl;
		}
		break;
	default:
		break;
	}
	
	++renderStep;
}


@end




CVReturn displayLinkCallback(CVDisplayLinkRef displayLink, 
	const CVTimeStamp *inNow, 
	const CVTimeStamp *inOutputTime, 
	CVOptionFlags flagsIn, 
	CVOptionFlags *flagsOut, 
	void *displayLinkContext)
{
	NSAutoreleasePool		*pool =[[NSAutoreleasePool alloc] init];
	[(TexDownloadUploadAppDelegate *)displayLinkContext renderCallback];
	[pool release];
	return kCVReturnSuccess;
}
