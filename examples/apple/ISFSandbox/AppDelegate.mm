#import "AppDelegate.h"

#include <iostream>




VVGL::VVGLBufferRef		newTex = nullptr;




@implementation AppDelegate


- (id) init	{
	self = [super init];
	if (self != nil)	{
		sharedContext = nullptr;
		scene = nullptr;
		vao = nullptr;
	}
	return self;
}
- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
	[self loadBackendFromDefaults];
	//	make the displaylink, which will drive rendering
	CVReturn				err = kCVReturnSuccess;
	CGOpenGLDisplayMask		totalDisplayMask = 0;
	GLint					virtualScreen = 0;
	GLint					displayMask = 0;
	NSOpenGLPixelFormat		*format = [[[NSOpenGLPixelFormat alloc] initWithCGLPixelFormatObj:CreateCompatibilityGLPixelFormat()] autorelease];
	
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

- (void) loadBackendFromDefaults	{
	//NSLog(@"%s",__func__);
	@synchronized (self)	{
		sharedContext = nullptr;
		scene = nullptr;
	
		//NSUserDefaults		*def = [NSUserDefaults standardUserDefaults];
		//NSNumber			*tmpNum = [def objectForKey:@"glVers"];
		//if (tmpNum == nil)
		//	tmpNum = [NSNumber numberWithInteger:GLVersion_2];
	
		//switch ([tmpNum intValue])	{
		//case GLVersion_2:
			sharedContext = make_shared<VVGLContext>(nullptr, CreateCompatibilityGLPixelFormat());
		//	break;
		//default:
		//	sharedContext = make_shared<VVGLContext>(nullptr, CreateGL4PixelFormat());
		//	break;
		//}
		
		CreateGlobalBufferPool(sharedContext);
		
		scene = make_shared<VVGLScene>();
		
		[glView setSharedGLContext:sharedContext];
		[glView setRetainDraw:YES];
		
		scene->setClearColor(0., 0., 0., 1.);
		scene->setPerformClear(true);
		
		switch (scene->getGLVersion())	{
		case GLVersion_2:
			[self initGL2];
			break;
		case GLVersion_4:
		default:
			[self initGL4];
			break;
		}
	}
}
- (void) initGL2	{
	NSLog(@"%s",__func__);
	scene->setRenderCallback([](const VVGLScene & n)	{
		Quad<VertXYRGBA>		quad;
		VVGL::Rect			geoRect(0, 0, 100, 100);
		VVGL::VT_RGBA		tmpColor(0., 0., 1., 1.);
		quad.populateGeo(geoRect);
		quad.populateColor(tmpColor);
		
		glEnableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		
		glVertexPointer(2, GL_FLOAT, quad.stride(), &quad);
		glColorPointer(4, GL_FLOAT, quad.stride(), &quad.bl.color.r);
		
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	});
}
- (void) initGL4	{
	NSLog(@"%s",__func__);
	string			vsString("\r\
#version 330 core\r\
in vec3		inXYZ;\r\
in vec4		inRGBA;\r\
uniform mat4	vvglOrthoProj;\r\
out vec4		programColor;\r\
void main()	{\r\
	gl_Position = vec4(inXYZ.x, inXYZ.y, inXYZ.z, 1.0) * vvglOrthoProj;\r\
	programColor = inRGBA;\r\
}\r\
");
	string			fsString("\r\
#version 330 core\r\
in vec4		programColor;\r\
out vec4		FragColor;\r\
void main()	{\r\
	FragColor = programColor;\r\
}\r\
");
	scene->setVertexShaderString(vsString);
	scene->setFragmentShaderString(fsString);
	
	VVGLCachedAttribRef		aXYZ = make_shared<VVGLCachedAttrib>("inXYZ");
	VVGLCachedAttribRef		aRGBA = make_shared<VVGLCachedAttrib>("inRGBA");
	
	void					*selfPtr = (void*)self;
	scene->setRenderPrepCallback([aXYZ,aRGBA,selfPtr](const VVGLScene & n, const bool & inReshaped, const bool & inPgmChanged)	{
		if (inPgmChanged)	{
			GLint		myProgram = n.getProgram();
			aXYZ->cacheTheLoc(myProgram);
			aRGBA->cacheTheLoc(myProgram);
			[(id)selfPtr setVAO:CreateVAO(true)];
		}
	});
	scene->setRenderCallback([aXYZ,aRGBA,selfPtr](const VVGLScene & n)	{
		
		Quad<VertXYZRGBA>	targetQuad;
		VVGL::Rect			geoRect(0, 0, 100, 100);
		VVGL::VT_RGBA		tmpColor(0., 0., 1., 1.);
		targetQuad.populateGeo(geoRect);
		targetQuad.populateColor(tmpColor);
		
		//	bind the VAO
		VVGLBufferRef		tmpVAO = [(id)selfPtr vao];
		glBindVertexArray(tmpVAO->name);
		
		uint32_t			vbo = 0;
		//if ([(id)selfPtr lastVBOCoords] != targetQuad)	{
			//	make a new VBO to contain vertex + texture coord data
			glGenBuffers(1, &vbo);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBufferData(GL_ARRAY_BUFFER, sizeof(targetQuad), (void*)&targetQuad, GL_STATIC_DRAW);
			//	configure the attribute pointers to use the VBO
			if (aXYZ->loc >= 0)	{
				glVertexAttribPointer(aXYZ->loc, 3, GL_FLOAT, GL_FALSE, targetQuad.stride(), (void*)0);
				aXYZ->enable();
			}
			if (aRGBA->loc >= 0)	{
				glVertexAttribPointer(aRGBA->loc, 4, GL_FLOAT, GL_FALSE, targetQuad.stride(), (void*)(3*sizeof(float)));
				aRGBA->enable();
			}
		//}
		
		//	draw
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		//	un-bind the VAO
		glBindVertexArray(0);
		
		//if ([(id)selfPtr lastVBOCoords] != targetQuad)	{
			//	delete the VBO we made earlier...
			glDeleteBuffers(1, &vbo);
			//	update the vbo coords ivar (we don't want to update the VBO contents every pass)
		//	[(id)selfPtr setLastVBOCoords:targetQuad];
		//}
	});
}


//	this method is called from the displaylink callback
- (void) renderCallback	{
	NSLog(@"%s",__func__);
	@synchronized (self)	{
		if (scene == nullptr)
			return;
		NSRect				viewFrame = [glView frame];
		//VVGLBufferRef		tmpBuffer = scene->createAndRenderABuffer(VVGL::Size(viewFrame.size.width, viewFrame.size.height));
		VVGLBufferRef		tmpBuffer = CreateRGBATex(VVGL::Size(viewFrame.size.width,viewFrame.size.height));
		//VVGLBufferRef		tmpBuffer = CreateRGBARectTex(VVGL::Size(viewFrame.size.width,viewFrame.size.height));
		
		scene->renderToBuffer(tmpBuffer);
		
		lastRenderedBuffer = tmpBuffer;
		if (lastRenderedBuffer != nullptr)
			cout << "\tlastRenderedBuffer is " << *lastRenderedBuffer << endl;
		
		VVGLBufferCopierRef	copier = GetGlobalBufferCopier();
		if (copier != nullptr)	{
			copier->setCopyAndResize(false);
			lastCopiedBuffer = copier->copyToNewBuffer(lastRenderedBuffer);
			if (lastCopiedBuffer != nullptr)
				cout << "\tlastCopiedBuffer is " << *lastCopiedBuffer << endl;
		}
		
		[glView drawBuffer:lastRenderedBuffer];
		//	tell the buffer pool to do its housekeeping (releases any "old" resources in the pool that have been sticking around for a while)
		GetGlobalBufferPool()->housekeeping();
	}
}


@synthesize vao;


- (IBAction) flushClicked:(id)sender	{
	NSLog(@"%s",__func__);
	if (scene == nullptr)
		return;
	VVGLContextRef		ctx = scene->getContext();
	if (ctx == nullptr)
		return;
	ctx->makeCurrentIfNotCurrent();
	glFlush();
}
- (IBAction) renderClicked:(id)sender	{
	NSLog(@"%s",__func__);
	[self renderCallback];
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
	//[(AppDelegate *)displayLinkContext renderCallback];
	[pool release];
	return kCVReturnSuccess;
}
