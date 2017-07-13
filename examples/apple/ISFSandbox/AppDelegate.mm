#import "AppDelegate.h"

#include <iostream>




VVGL::VVGLBufferRef		newTex = nullptr;




@implementation AppDelegate


- (id) init	{
	using namespace VVISF;
	using namespace std;
	//cout << __PRETTY_FUNCTION__ << endl;
	
	self = [super init];
	if (self != nil)	{
		//CGLPixelFormatObj	pxlFmt = CreateDefaultPixelFormat();
		//sharedContext = [[NSOpenGLContext alloc]
		//	initWithFormat:[[[NSOpenGLPixelFormat alloc] initWithCGLPixelFormatObj:pxlFmt] autorelease]
		//	shareContext:nil];
		
		
		/*	make a shared context- this is required.  note that resources created by this context 
		are not shared with anything (when this ctx was created, its sharegroup was null), so stuff 
		you create in this context will not be shared.  which is okay- we're never going to tell this 
		context to render anything (it exists merely to establish a sharegroup for other contexts)		*/
		sharedCtx = make_shared<VVGLContext>();
		/*	now make the global buffer pool with the shared context.  the buffer pool will create 
		its own context, and that context will be a member of the sharegroup so its resources can be 
		shared with other contexts.  create further GL contexts either using the original 
		sharedContext, or using the buffer pool's context.		*/
		CreateGlobalBufferPool(sharedCtx);
		
		
		//	from here on out, we can proceed with making other contexts/scenes/etc.
		
		
		//	make a new scene.  we aren't giving the scene a context to use, so it will create its own context in the global buffer pool's sharegroup.
		scene = new VVGLScene(GetGlobalBufferPool()->getContext());
		scene->setClearColor(1., 0., 0., 1.);
		//scene->setRenderCallback([](const VVGLScene & n){
		//	cout << __PRETTY_FUNCTION__ << endl;
		//});
		
		//	make a new ISFScene
		isfScene = new ISFScene(GetGlobalBufferPool()->getContext());
	}
	return self;
}
- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
	using namespace std;
	using namespace VVISF;
	
	//cout << __PRETTY_FUNCTION__ << endl;
	
	//NSString		*path = @"/Users/testAdmin/Library/Graphics/ISF/CellMod.fs";
	/*
	NSString		*path = @"/Users/testAdmin/Library/Graphics/ISF/Image_Based_PBR_Material_ld3SRr.fs";
	*/
	NSString		*path = [[NSBundle mainBundle] pathForResource:@"CellMod" ofType:@"fs"];
	/*
	NSString		*path = [[NSBundle mainBundle] pathForResource:@"Test-Audio" ofType:@"fs"];
	NSString		*path = [[NSBundle mainBundle] pathForResource:@"Test-AudioFFT" ofType:@"fs"];
	NSString		*path = [[NSBundle mainBundle] pathForResource:@"Test-Bool" ofType:@"fs"];
	NSString		*path = [[NSBundle mainBundle] pathForResource:@"Test-Color" ofType:@"fs"];
	NSString		*path = [[NSBundle mainBundle] pathForResource:@"Test-Event" ofType:@"fs"];
	NSString		*path = [[NSBundle mainBundle] pathForResource:@"Test-Float" ofType:@"fs"];
	NSString		*path = [[NSBundle mainBundle] pathForResource:@"Test-Functionality" ofType:@"fs"];
	NSString		*path = [[NSBundle mainBundle] pathForResource:@"Test-IMG_NORM_PIXEL" ofType:@"fs"];
	NSString		*path = [[NSBundle mainBundle] pathForResource:@"Test-IMG_PIXEL" ofType:@"fs"];
	NSString		*path = [[NSBundle mainBundle] pathForResource:@"Test-IMG_THIS_NORM_PIXEL" ofType:@"fs"];
	NSString		*path = [[NSBundle mainBundle] pathForResource:@"Test-IMG_THIS_PIXEL" ofType:@"fs"];
	NSString		*path = [[NSBundle mainBundle] pathForResource:@"Test-ImportedImage" ofType:@"fs"];
	NSString		*path = [[NSBundle mainBundle] pathForResource:@"Test-Long" ofType:@"fs"];
	NSString		*path = [[NSBundle mainBundle] pathForResource:@"Test-MultiPassRendering" ofType:@"fs"];
	NSString		*path = [[NSBundle mainBundle] pathForResource:@"Test-PersistentBuffer" ofType:@"fs"];
	NSString		*path = [[NSBundle mainBundle] pathForResource:@"Test-PersistentBufferDifferingSizes" ofType:@"fs"];
	NSString		*path = [[NSBundle mainBundle] pathForResource:@"Test-Point" ofType:@"fs"];
	NSString		*path = [[NSBundle mainBundle] pathForResource:@"Test-Sampler" ofType:@"fs"];
	NSString		*path = [[NSBundle mainBundle] pathForResource:@"Test-TempBufferDifferingSizes" ofType:@"fs"];
	*/
	std::string		tmpString = std::string([path UTF8String]);
	isfScene->useFile(tmpString);
	//cout << "doc is \n" << *(isfScene->getDoc()) << endl;
	
	
	[NSTimer
		scheduledTimerWithTimeInterval:1./60.
		target:self
		selector:@selector(timerCallback:)
		userInfo:nil
		repeats:YES];
	
}
- (void)applicationWillTerminate:(NSNotification *)aNotification {
	// Insert code here to tear down your application
}


- (void) timerCallback:(NSTimer *)t	{
	//NSLog(@"%s",__func__);
	
	using namespace VVISF;
	using namespace std;
	
	//cout << __PRETTY_FUNCTION__ << std::endl;
	
	//VVGLBufferPoolRef	bp = GetGlobalBufferPool();
	//VVGLBufferCopierRef	copier = GetGlobalBufferCopier();
	/*
	VVGLBufferRef	newFrame = scene->createAndRenderABuffer({1920,1080});
	[glView drawBuffer:newFrame];
	*/
	
	VVGLBufferRef	newFrame = isfScene->createAndRenderABuffer(VVISF::Size(1920.,1080.));
	VVGLBufferRef	copiedFrame = GetGlobalBufferCopier()->copyToNewBuffer(newFrame);
	//cout << "\tnewFrame is " << *newFrame << ", drawing " << *copiedFrame << endl;
	[glView drawBuffer:copiedFrame];
	
	/*
	VVGLBufferRef		renderedBuffer = shaderScene->createAndRenderABuffer();
	[glView drawBuffer:renderedBuffer];
	*/
	/*
	VVGLBufferRef		copiedBuffer = copier->copyToNewBuffer(renderedBuffer);
	*/
	/*
	VVGLBufferRef		copiedBuffer = ISFBPCreateRGBATex(bp, { 1920., 1080. });
	if (!copier->copyFromTo(renderedBuffer, copiedBuffer))
		cout << "\tERR: copy failed in " << __PRETTY_FUNCTION__ << endl;
	*/
	/*
	VVGLBufferRef		copiedBuffer = ISFBPCreateRGBATex(bp, { 40., 30. });
	copier->setCopyAndResize(true);
	copier->setCopySize({40.,30.});
	if (!copier->copyFromTo(renderedBuffer, copiedBuffer))
		cout << "\tERR: copy failed in " << __PRETTY_FUNCTION__ << endl;
	*/
	/*
	VVGLBufferRef		copiedBuffer = ISFBPCreateRGBATex(bp, { 40., 30. });
	copier->sizeVariantCopy(renderedBuffer, copiedBuffer);
	*/
	/*
	VVGLBufferRef		copiedBuffer = ISFBPCreateRGBATex(bp, { 320., 240. });
	copier->ignoreSizeCopy(renderedBuffer, copiedBuffer);
	*/
	
	//cout << "\trendered buffer " << *renderedBuffer << endl;
	//cout << "\trendered buffer is " << *renderedBuffer << ", copied buffer is " << *copiedBuffer << endl;
	//[glView drawBuffer:copiedBuffer];
	
	//bp->housekeeping();
	GetGlobalBufferPool()->housekeeping();
}


- (IBAction) buttonClicked:(id)sender	{
	using namespace std;
	using namespace VVISF;
	cout << __PRETTY_FUNCTION__ << std::endl;
	
	//cout << __PRETTY_FUNCTION__ << std::endl;
	
	//VVGLBufferRef	newFrame = isfScene->createAndRenderABuffer(VVISF::Size(1920.,1080.));
	//cout << "\tnewFrame is " << *newFrame << endl;
	//[glView drawBuffer:newFrame];
	
	//VVGLBufferRef	newFrame = scene->createAndRenderABuffer({1920,1080});
	//cout << "\tnewFrame is " << *newFrame << endl;
	
	VVGLBufferRef		renderedBuffer = isfScene->createAndRenderABuffer();
	//VVGLBufferRef		renderedBuffer = scene->renderRedFrame();
	cout << "\trenderedBuffer is " << *renderedBuffer << endl;
	[glView drawBuffer:renderedBuffer];
	//retainedBuffer = scene->createAndRenderABuffer();
	
	//VVGLBufferRef		renderedBuffer = shaderScene->createAndRenderABuffer();
	//cout << "\trendered buffer " << *renderedBuffer << endl;
	//[glView drawBuffer:renderedBuffer];
	
	//NSString		*path = [[NSBundle mainBundle] pathForResource:@"Test-Functionality" ofType:@"fs"];
	//NSString		*path = [[NSBundle mainBundle] pathForResource:@"Test-Float" ofType:@"fs"];
	//NSString		*path = @"/Users/testAdmin/Library/Graphics/ISF/CellMod.fs";
	//isfScene->useFile(string([path UTF8String]));
	
}
- (IBAction) flushClicked:(id)sender	{
	using namespace std;
	using namespace VVISF;
	cout << __PRETTY_FUNCTION__ << std::endl;
	//GetGlobalBufferPool()->flush();
	/*
	VVGLBufferRef	newFrame = isfScene->createAndRenderABuffer(VVISF::Size(1920.,1080.));
	//cout << "\trendered frame is " << *newFrame << endl;
	[glView drawBuffer:newFrame];
	*/
}
- (IBAction) renderToTexture:(id)sender	{
	isfScene->getContext()->makeCurrent();
	newTex = isfScene->createAndRenderABuffer(VVISF::Size(1920.,1080.));
}
- (IBAction) drawInOutput:(id)sender	{
	[glView drawBuffer:newTex];
}


@end
