#import "GeoShaderTestAppDelegate.h"
#import "GLBufferPool_CocoaAdditions.h"




@interface GeoShaderTestAppDelegate ()
@property (assign,readwrite) VVGL::Quad<VVGL::VertXYZRGBA> lastVBOCoords;
@end




@implementation GeoShaderTestAppDelegate


- (id) init	{
	self = [super init];
	if (self != nil)	{
		
		//	make the shared context using the vsn of GL you need to target.  all GL contexts are going to share this so they can share textures/etc with one another
		//sharedContext = CreateNewGLContextRef(NULL, CreateCompatibilityGLPixelFormat());
		sharedContext = CreateNewGLContextRef(NULL, CreateGL4PixelFormat());
		
		//	make the global buffer pool.  if there's a global buffer pool, calls to create textures/etc will be shorter.  this global buffer pool will use the shared context to create any GL resources.
		CreateGlobalBufferPool(sharedContext);
		
		//	make the GL scene we're going to use to render to texture (the texture will then be drawn in a view)
		glScene = CreateGLSceneRefUsing(sharedContext->newContextSharingMe());
		//	set up the GL context- this will vary significantly based on the version of GL you chose to use when making the shared context above
		if (glScene->glVersion() == GLVersion_2)
			[self initForGL2];
		else
			[self initForModernGL];
		
		[self setDate:[NSDate date]];
	}
	return self;
}


- (void)applicationDidFinishLaunching:(NSNotification *)aNotification	{
	//	pass the shared context to the buffer view (view has its own ctx, so it needs to be in the same sharegroup to be able to draw the passed texture)
	[bufferView setSharedGLContext:sharedContext];
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
	//	tell the GL scene to allocate and render itself to a buffer
	GLBufferRef		newTex = glScene->createAndRenderABuffer(VVGL::Size(150.,150.));
	//if (newTex == nullptr)
	//	NSLog(@"\t\terr: couldn't render a tex");
	//else
	//	cout << "\trendered texture " << *newTex << endl;
	//	draw the GL texture i just rendered in the buffer view
	[bufferView drawBuffer:newTex];
	//	tell the buffer pool to do its housekeeping (releases any "old" resources in the pool that have been sticking around for a while)
	GetGlobalBufferPool()->housekeeping();
}


- (void) initForGL2	{
	
	void			*selfPtr = (void*)self;
	
	NSBundle		*mb = [NSBundle mainBundle];
	NSString		*nsVSString = [NSString stringWithContentsOfFile:[mb pathForResource:@"GeoShaderTest" ofType:@"vs"] encoding:NSUTF8StringEncoding error:nil];
	NSString		*nsGSString = [NSString stringWithContentsOfFile:[mb pathForResource:@"GeoShaderTest" ofType:@"gs"] encoding:NSUTF8StringEncoding error:nil];
	NSString		*nsFSString = [NSString stringWithContentsOfFile:[mb pathForResource:@"GeoShaderTest" ofType:@"fs"] encoding:NSUTF8StringEncoding error:nil];
	string			vsString([nsVSString UTF8String]);
	string			gsString([nsGSString UTF8String]);
	string			fsString([nsFSString UTF8String]);
	
	glScene->setVertexShaderString(vsString);
	//glScene->setGeometryShaderString(gsString);
	glScene->setFragmentShaderString(fsString);
	
	glScene->setClearColor(0., 1., 0., 1.);
	glScene->setPerformClear(true);
	
	glScene->setRenderCallback([selfPtr](const GLScene & n)	{
		Quad<VertXYZRGBA>		quad;
		quad.populateGeo(VVGL::Rect(50., 50., 50., 50.));
		quad.populateColor(VVGL::VT_RGBA(1., 0., 0., 1.));
		
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		
		glVertexPointer(3, GL_FLOAT, quad.stride(), &quad.bl.geo);
		glColorPointer(4, GL_FLOAT, quad.stride(), &quad.bl.color);
		
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	});
	
	glScene->setRenderPreLinkCallback([](const GLScene & n)	{
		GLint				myProgram = n.program();
		glProgramParameteriEXT(myProgram, GL_GEOMETRY_INPUT_TYPE_EXT, GL_TRIANGLE_STRIP);
		glProgramParameteriEXT(myProgram, GL_GEOMETRY_OUTPUT_TYPE_EXT, GL_TRIANGLE_STRIP);
		
		glProgramParameteriEXT(myProgram, GL_GEOMETRY_VERTICES_OUT_EXT, 1);
		//glProgramParameteriEXT(myProgram, GL_GEOMETRY_VERTICES_OUT_EXT, GL_TRIANGLE_STRIP);
		
		//glProgramParameteriEXT(myProgram, GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT, 1);
	});
}


- (void) initForModernGL	{
	NSLog(@"%s",__func__);
	/*
	//	make an NSImage from the PNG included with the app, create a GLBufferRef from it
	NSImage			*tmpImg = [[NSImage alloc] initWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"SampleImg" ofType:@"png"]];
	GLBufferRef	imgBuffer = CreateBufferForNSImage(tmpImg);
	[tmpImg release];
	tmpImg = nil;
	*/
	void			*selfPtr = (void*)self;
	
	NSBundle		*mb = [NSBundle mainBundle];
	NSString		*nsVSString = [NSString stringWithContentsOfFile:[mb pathForResource:@"GeoShaderTest_modern" ofType:@"vs"] encoding:NSUTF8StringEncoding error:nil];
	NSString		*nsGSString = [NSString stringWithContentsOfFile:[mb pathForResource:@"GeoShaderTest_modern" ofType:@"gs"] encoding:NSUTF8StringEncoding error:nil];
	NSString		*nsFSString = [NSString stringWithContentsOfFile:[mb pathForResource:@"GeoShaderTest_modern" ofType:@"fs"] encoding:NSUTF8StringEncoding error:nil];
	string			vsString([nsVSString UTF8String]);
	string			gsString([nsGSString UTF8String]);
	string			fsString([nsFSString UTF8String]);
	
	glScene->setVertexShaderString(vsString);
	glScene->setGeometryShaderString(gsString);
	glScene->setFragmentShaderString(fsString);
	
	glScene->setClearColor(0., 1., 0., 1.);
	glScene->setPerformClear(true);
	
	//	we're going to create a couple vars on the stack here- the vars themselves are shared 
	//	ptrs, so when they're copied by value in the callback blocks the copies will refer to 
	//	the same underlying vars, which will be retained until these callback blocks are 
	//	destroyed and shared between the callback lambdas...
	GLCachedAttribRef		xyzAttr = make_shared<GLCachedAttrib>("inXYZ");
	GLCachedAttribRef		rgbaAttr = make_shared<GLCachedAttrib>("inRGBA");
	
	//	the render prep callback needs to cache the location of the vertex attributes and uniforms
	glScene->setRenderPrepCallback([xyzAttr,rgbaAttr,self](const GLScene & n, const bool & inReshaped, const bool & inPgmChanged)	{
		//cout << __PRETTY_FUNCTION__ << endl;
		if (inPgmChanged)	{
			//	cache all the locations for the vertex attributes & uniform locations
			GLint				myProgram = n.program();
			xyzAttr->cacheTheLoc(myProgram);
			rgbaAttr->cacheTheLoc(myProgram);
			
			//	make a new VAO
			[self setVAO:CreateVAO(true)];
		}
	});
	
	//	the render callback passes all the data to the GL program
	glScene->setRenderCallback([xyzAttr,rgbaAttr,selfPtr](const GLScene & n)	{
		//cout << __PRETTY_FUNCTION__ << endl;
		//if (imgBuffer == nullptr)
		//	return;
		
		//	clear
		//glClearColor(0., 1., 0., 1.);
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		//VVGL::Size			orthoSize = n.orthoSize();
		Quad<VVGL::VertXYZRGBA>	targetQuad;
		//targetQuad.populateGeo(VVGL::Rect(0,0,orthoSize.width,orthoSize.height));
		targetQuad.populateGeo(VVGL::Rect(50., 50., 50, 50));
		targetQuad.populateColor(VVGL::VT_RGBA(1., 0., 0., 1.));
		
		
		//	bind the VAO
		GLBufferRef		tmpVAO = [(id)selfPtr vao];
		if (tmpVAO == nullptr)
			return;
		glBindVertexArray(tmpVAO->name);
		
		uint32_t			vbo = 0;
		if ([(id)selfPtr lastVBOCoords] != targetQuad)	{
			//	make a new VBO to contain vertex + texture coord data
			glGenBuffers(1, &vbo);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBufferData(GL_ARRAY_BUFFER, sizeof(targetQuad), (void*)&targetQuad, GL_STATIC_DRAW);
			//	configure the attribute pointers to use the VBO
			if (xyzAttr->loc >= 0)	{
				glVertexAttribPointer(xyzAttr->loc, 3, GL_FLOAT, GL_FALSE, targetQuad.stride(), BUFFER_OFFSET(targetQuad.geoOffset()));
				xyzAttr->enable();
			}
			else
				NSLog(@"\t\terr: couldn't locate xyz attr, %s",__func__);
			if (rgbaAttr->loc >= 0)	{
				glVertexAttribPointer(rgbaAttr->loc, 4, GL_FLOAT, GL_FALSE, targetQuad.stride(), BUFFER_OFFSET(targetQuad.colorOffset()));
				rgbaAttr->enable();
			}
			else
				NSLog(@"\t\terr: couldn't locate color attr, %s",__func__);
		}
		
		//	draw
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		//	un-bind the VAO
		glBindVertexArray(0);
		
		if ([(id)selfPtr lastVBOCoords] != targetQuad)	{
			//	delete the VBO we made earlier...
			glDeleteBuffers(1, &vbo);
			//	update the vbo coords ivar (we don't want to update the VBO contents every pass)
			[(id)selfPtr setLastVBOCoords:targetQuad];
		}
		
	});
	
}


@synthesize date;
@synthesize vao;
@synthesize lastVBOCoords;


@end




CVReturn displayLinkCallback(CVDisplayLinkRef displayLink, 
	const CVTimeStamp *inNow, 
	const CVTimeStamp *inOutputTime, 
	CVOptionFlags flagsIn, 
	CVOptionFlags *flagsOut, 
	void *displayLinkContext)
{
	NSAutoreleasePool		*pool =[[NSAutoreleasePool alloc] init];
	[(GeoShaderTestAppDelegate *)displayLinkContext renderCallback];
	[pool release];
	return kCVReturnSuccess;
}
