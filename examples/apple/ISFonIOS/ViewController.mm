#import "ViewController.h"




@implementation ViewController


- (id) initWithCoder:(NSCoder *)c	{
	NSLog(@"%s",__func__);
	self = [super initWithCoder:c];
	if (self != nil)	{
		//	make the base VVGLContextRef (we could also just call make_shared<VVGLcontext>();)
		EAGLContext		*tmpCtx = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
		baseCtx = make_shared<VVGLContext>((void*)tmpCtx);
		[tmpCtx release];
		
		//	make the global buffer pool, have it share the base context
		CreateGlobalBufferPool(baseCtx);
		
		//	make the ISFScene
		isfScene = make_shared<ISFScene>(baseCtx->newContextSharingMe());
		//	tell the ISFScene to load the included file
		NSString		*tmpString = [[NSBundle mainBundle] pathForResource:@"blueelectricspiral" ofType:@"fs"];
		if (tmpString != nil)	{
			isfScene->useFile(string([tmpString UTF8String]));
		}
		
	}
	return self;
}
- (void)viewDidLoad {
	[super viewDidLoad];
	// Do any additional setup after loading the view, typically from a nib.
	CADisplayLink		*dl = [CADisplayLink displayLinkWithTarget:self selector:@selector(displayLinkCallback:)];
	[dl addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
}
- (void) displayLinkCallback:(CADisplayLink *)dl	{
	//NSLog(@"%s",__func__);
	VVGLBufferRef		newBuffer = isfScene->createAndRenderABuffer();
	if (newBuffer != nullptr)	{
		[bufferView drawBuffer:newBuffer];
	}
	
	VVGLBufferPoolRef		bp = GetGlobalBufferPool();
	if (bp != nullptr)
		bp->housekeeping();
}


@end
