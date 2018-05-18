#import "TransFeedbackAppDelegate.h"




@interface TransFeedbackAppDelegate ()
@property (assign,readwrite) VVGL::Quad<VVGL::VertXYZRGBA> lastVBOCoords;
@end




@implementation TransFeedbackAppDelegate


- (id) init	{
	self = [super init];
	if (self != nil)	{
		
		//	make the shared context using the vsn of GL you need to target.  all GL contexts are going to share this so they can share textures/etc with one another
		//sharedContext = CreateNewGLContextRef(NULL, CreateCompatibilityGLPixelFormat());
		sharedContext = CreateNewGLContextRef(NULL, CreateGL4PixelFormat());
		
		//	make the global buffer pool.  if there's a global buffer pool, calls to create textures/etc will be shorter.  the global buffer pool will use the shared context to create any GL resources
		CreateGlobalBufferPool(sharedContext);
		
		feedbackScene = CreateGLSceneRefUsing(sharedContext->newContextSharingMe());
		rasterScene = CreateGLSceneRefUsing(sharedContext->newContextSharingMe());
		//	set up the GL context- this will vary significantly based on the version of GL you chose to use when making the shared context above
		if (rasterScene->getGLVersion() == GLVersion_2)
			[self initForGL2];
		else
			[self initForModernGL];
		
		//[self setDate:[NSDate date]];
	}
	return self;
}


- (void)applicationDidFinishLaunching:(NSNotification *)aNotification	{
	date = [[NSDate date] retain];
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
	
	//	tell the feedback scene to render.  this reads from 'vertVBO' and writes to 'feedbackVBO'.
	feedbackScene->render();
	
	//	swap the buffers- we just wrote to 'feedbackVBO', make that the new 'vertVBO'.  next time we render the feedback scene will create a new 'feedbackVBO'.
	vertVBO = feedbackVBO;
	feedbackVBO = nullptr;
	
	//	tell the raster scene to render.  this reads from 'feedbackVBO'.
	GLBufferRef		newTex = rasterScene->createAndRenderABuffer(VVGL::Size(150.,150.));
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

}


- (void) initForModernGL	{
	NSLog(@"%s",__func__);
	
	{
		rasterScene->getContext()->makeCurrentIfNotCurrent();
		//vao = CreateVAO(true);
		
		
	}
	
	void			*selfPtr = (void*)self;
	
	NSBundle		*mb = [NSBundle mainBundle];
	NSString		*nsVSString = nil;
	NSString		*nsFSString = nil;
	string			vsString;
	string			fsString;
	
	//	we're going to create a couple vars on the stack here- the vars themselves are shared 
	//	ptrs, so when they're copied by value in the callback blocks the copies will refer to 
	//	the same underlying vars, which will be retained until these callback blocks are 
	//	destroyed and shared between the callback lambdas...
	GLBufferRef			vao = nullptr;
	GLCachedAttribRef		inXYZAttr = nullptr;
	GLCachedAttribRef		inRGBAAttr = nullptr;
	//	we also need an FBO- if we don't have one, we get a GL error- and of course the FBO needs an attachment or there's an error for that, too.
	GLBufferRef			fbo = nullptr;
	GLBufferRef			fboTex = nullptr;
	
	
	
	
	//	make the feedback scene's context current
	feedbackScene->getContext()->makeCurrentIfNotCurrent();
	
	//	make a target quad, use it to populate the vertVBO.  initialize the feedbackVBO to nil.
	Quad<VVGL::VertXYZRGBA>			targetQuad;
	targetQuad.populateGeo(VVGL::Rect(50., 50., 50, 50));
	targetQuad.populateColor(VVGL::VT_RGBA(1., 0., 0., 1.));
	vertVBO = CreateVBO(&targetQuad, sizeof(targetQuad), GL_STREAM_DRAW);
	feedbackVBO = nullptr;
	
	//	load the vertex shader for the feedback scene
	nsVSString = [NSString stringWithContentsOfFile:[mb pathForResource:@"TransFeedback_Feedback" ofType:@"vs"] encoding:NSUTF8StringEncoding error:nil];
	vsString = std::string([nsVSString UTF8String]);
	feedbackScene->setVertexShaderString(vsString);
	feedbackScene->setPerformClear(false);
	
	//	make a vao and new attrib refs to simplify tracking shader attributes
	vao = CreateVAO(true);
	inXYZAttr = make_shared<GLCachedAttrib>("inXYZ");
	inRGBAAttr = make_shared<GLCachedAttrib>("inRGBA");
	fbo = CreateFBO();
	fboTex = CreateBGRATex(VVGL::Size(50.,50.));
	
	//	transform feedbacks require a special step after shader compilation but before program linking
	feedbackScene->setRenderPreLinkCallback([](const GLScene & n)	{
		const GLchar * feedbackVaryings[] = { "outXYZ", "outRGBA" };
		glTransformFeedbackVaryings(n.getProgram(), 2, feedbackVaryings, GL_INTERLEAVED_ATTRIBS);
	});
	//	render prep only has to make sure the attrib locations are cached
	feedbackScene->setRenderPrepCallback([=](const GLScene & n, const bool & inReshaped, const bool & inPgmChanged)	{
		if (inPgmChanged)	{
			//	cache all the locations for the vertex attributes & uniform locations
			GLint				myProgram = n.getProgram();
			inXYZAttr->cacheTheLoc(myProgram);
			inRGBAAttr->cacheTheLoc(myProgram);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, fbo->name);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, fboTex->desc.target, fboTex->name, 0);
	});
	feedbackScene->setRenderCallback([=](const GLScene & n)	{
		//	bind the VAO
		glBindVertexArray(vao->name);
		
		Quad<VVGL::VertXYZRGBA>			tmpQuad;	//	unpopulated- matches data format in VBO, only used to query stride and offset
		
		//	bind the VBO with the data i'm passing to the vertex shader
		glBindBuffer(GL_ARRAY_BUFFER, [(id)selfPtr vertVBO]->name);
		//	configure the attribute pointers to use the VBO
		if (inXYZAttr->loc >= 0)	{
			inXYZAttr->enable();
			glVertexAttribPointer(inXYZAttr->loc, 3, GL_FLOAT, GL_FALSE, tmpQuad.stride(), BUFFER_OFFSET(tmpQuad.geoOffset()));
		}
		else
			NSLog(@"\t\terr: couldn't locate inXYZ attr, %s",__func__);
		if (inRGBAAttr->loc >= 0)	{
			inRGBAAttr->enable();
			glVertexAttribPointer(inRGBAAttr->loc, 4, GL_FLOAT, GL_FALSE, tmpQuad.stride(), BUFFER_OFFSET(tmpQuad.colorOffset()));
		}
		else
			NSLog(@"\t\terr: couldn't locate inRGBA attr, %s",__func__);
		//	un-bind the VBO
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		
		
		//	make a new VBO- the transform feedback will render into this.  bind it.
		GLBufferRef			feedbackBuffer = CreateVBO(NULL, sizeof(Quad<VVGL::VertXYZRGBA>), GL_STREAM_DRAW, true);
		[(id)selfPtr setFeedbackVBO:feedbackBuffer];
		//glBindBuffer(GL_ARRAY_BUFFER, feedbackBuffer->name);
		
		//	disable the rasterizer!
		glEnable(GL_RASTERIZER_DISCARD);
		
		//	configure the attribute pointers to read back into the VBO
		//if (outXYZAttr->loc >= 0)	{
		//	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, outXYZAttr->loc, feedbackBuffer->name);
		//}
		//else	{
		//	NSLog(@"\t\terr: couldn't locate outXYZ attr, %s",__func__);
			glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, feedbackBuffer->name);
		//}
		//if (outRGBAAttr->loc >= 0)	{
		//	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, outRGBAAttr->loc, feedbackBuffer->name);
		//}
		//else	{
		//	NSLog(@"\t\terr: couldn't local outRGBA attr, %s",__func__);
		//	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, feedbackBuffer->name);
		//}
		
		//	un-bind the dst VBO
		//glBindBuffer(GL_ARRAY_BUFFER, 0);
		
		//	begin the transform feedback using the same primitive type that we're going to use for the draw call
		glBeginTransformFeedback(GL_POINTS);
		
		//	draw
		glDrawArrays(GL_POINTS, 0, 4);
		
		//	end the transform feedback
		glEndTransformFeedback();
		
		//	un-bind the VAO
		glBindVertexArray(0);
	});
	feedbackScene->setRenderCleanupCallback([=](const GLScene & n)	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	});
	
	
	
	
	//	now set up the raster scene- first make its context current
	feedbackScene->getContext()->makeCurrentIfNotCurrent();
	
	//	load the vertex and frag shaders from disk
	nsVSString = [NSString stringWithContentsOfFile:[mb pathForResource:@"TransFeedback_Renderer" ofType:@"vs"] encoding:NSUTF8StringEncoding error:nil];
	nsFSString = [NSString stringWithContentsOfFile:[mb pathForResource:@"TransFeedback_Renderer" ofType:@"fs"] encoding:NSUTF8StringEncoding error:nil];
	fsString = std::string([nsFSString UTF8String]);
	vsString = std::string([nsVSString UTF8String]);
	
	rasterScene->setVertexShaderString(vsString);
	rasterScene->setFragmentShaderString(fsString);
	
	rasterScene->setClearColor(0., 1., 0., 1.);
	rasterScene->setPerformClear(true);
	
	//	create a new VAO/new attrib caches for the new scene
	vao = CreateVAO(true);
	inXYZAttr = make_shared<GLCachedAttrib>("inXYZ");
	inRGBAAttr = make_shared<GLCachedAttrib>("inRGBA");
	
	//	the render prep callback needs to cache the location of the vertex attributes and uniforms
	rasterScene->setRenderPrepCallback([=](const GLScene & n, const bool & inReshaped, const bool & inPgmChanged)	{
		//cout << __PRETTY_FUNCTION__ << endl;
		if (inPgmChanged)	{
			//	cache all the locations for the vertex attributes & uniform locations
			GLint				myProgram = n.getProgram();
			inXYZAttr->cacheTheLoc(myProgram);
			inRGBAAttr->cacheTheLoc(myProgram);
		}
	});
	//	the render callback passes all the data to the GL program
	rasterScene->setRenderCallback([=](const GLScene & n)	{
		//cout << __PRETTY_FUNCTION__ << endl;
		
		glBindVertexArray(vao->name);
		
		Quad<VVGL::VertXYZRGBA>			targetQuad;
		
		glBindBuffer(GL_ARRAY_BUFFER, [(id)selfPtr vertVBO]->name);
		//	configure the attribute pointers to use the VBO
		if (inXYZAttr->loc >= 0)	{
			inXYZAttr->enable();
			glVertexAttribPointer(inXYZAttr->loc, 3, GL_FLOAT, GL_FALSE, targetQuad.stride(), BUFFER_OFFSET(targetQuad.geoOffset()));
		}
		else
			NSLog(@"\t\terr: couldn't locate xyz attr, %s",__func__);
		if (inRGBAAttr->loc >= 0)	{
			inRGBAAttr->enable();
			glVertexAttribPointer(inRGBAAttr->loc, 4, GL_FLOAT, GL_FALSE, targetQuad.stride(), BUFFER_OFFSET(targetQuad.colorOffset()));
		}
		else
			NSLog(@"\t\terr: couldn't locate color attr, %s",__func__);
		
		//	un-bind the VBO
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		
		//	draw
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		
		//	un-bind the VAO
		glBindVertexArray(0);
		
	});
}


@synthesize date;
//@synthesize vao;
@synthesize lastVBOCoords;
@synthesize vertVBO;
@synthesize feedbackVBO;


@end




CVReturn displayLinkCallback(CVDisplayLinkRef displayLink, 
	const CVTimeStamp *inNow, 
	const CVTimeStamp *inOutputTime, 
	CVOptionFlags flagsIn, 
	CVOptionFlags *flagsOut, 
	void *displayLinkContext)
{
	NSAutoreleasePool		*pool =[[NSAutoreleasePool alloc] init];
	[(TransFeedbackAppDelegate *)displayLinkContext renderCallback];
	[pool release];
	return kCVReturnSuccess;
}
