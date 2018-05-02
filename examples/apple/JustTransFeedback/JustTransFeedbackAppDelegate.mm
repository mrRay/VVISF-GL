#import "JustTransFeedbackAppDelegate.h"


@implementation JustTransFeedbackAppDelegate

- (id) init	{
	self = [super init];
	if (self != nil)	{
		//sharedContext = CreateNewGLContextRef(NULL, CreateCompatibilityGLPixelFormat());
		sharedContext = CreateNewGLContextRef(NULL, CreateGL4PixelFormat());
		
		CreateGlobalBufferPool(sharedContext);
		
		srcGeoVBO = CreateVBO(nullptr, sizeof(Quad<VVGL::VertXYZ>), GL_STREAM_DRAW);
		srcColorVBO = CreateVBO(nullptr, sizeof(Quad<VVGL::VertRGBA>), GL_STREAM_DRAW);
		dstGeoVBO = CreateVBO(nullptr, sizeof(Quad<VVGL::VertXYZ>), GL_STREAM_DRAW);
		dstColorVBO = CreateVBO(nullptr, sizeof(Quad<VVGL::VertRGBA>), GL_STREAM_DRAW);
		
		scene = CreateGLSceneRefUsing(sharedContext->newContextSharingMe());
		
		//	init the scene differently depending on the version of GL running the backend
		if (scene->getGLVersion() == GLVersion_2)
			[self initForGL2];
		else
			[self initForModernGL];
		
		
		
		
		Quad<VVGL::VertXYZ>		posQuad;
		Quad<VVGL::VertRGBA>	colorQuad;
		float					*floatPtr = nullptr;
		//	log the src VBOs
		glBindBuffer(GL_ARRAY_BUFFER, srcGeoVBO->name);
		glGetBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(posQuad), &posQuad);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		cout << "srcGeoVBO:" << endl;
		floatPtr = (float*)&posQuad;
		for (int point=0; point<4; ++point)	{
			for (int comp=0; comp<3; ++comp)	{
				char		tmpChar = ' ';
				switch (comp)	{
				case 0:		tmpChar = 'X';		break;
				case 1:		tmpChar = 'Y';		break;
				case 2:		tmpChar = 'Z';		break;
				}
				cout << FmtString("[%d-%c] %0.2f\t\t",point,tmpChar,*floatPtr);
				++floatPtr;
			}
			cout << endl;
		}
		glBindBuffer(GL_ARRAY_BUFFER, srcColorVBO->name);
		glGetBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(colorQuad), &colorQuad);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		cout << "srcColorVBO:" << endl;
		floatPtr = (float*)&colorQuad;
		for (int point=0; point<4; ++point)	{
			for (int comp=0; comp<4; ++comp)	{
				char		tmpChar = ' ';
				switch (comp)	{
				case 0:		tmpChar = 'R';		break;
				case 1:		tmpChar = 'G';		break;
				case 2:		tmpChar = 'B';		break;
				case 3:		tmpChar = 'A';		break;
				}
				cout << FmtString("[%d-%c] %0.2f\t\t",point,tmpChar,*floatPtr);
				++floatPtr;
			}
			cout << endl;
		}
		
		
		
		
		//	tell the scene to render
		scene->render();
		
		
		
		
		//	log the dst VBOs
		glBindBuffer(GL_ARRAY_BUFFER, dstGeoVBO->name);
		glGetBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(posQuad), &posQuad);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		cout << "dstGeoVBO:" << endl;
		floatPtr = (float*)&posQuad;
		for (int point=0; point<4; ++point)	{
			for (int comp=0; comp<3; ++comp)	{
				char		tmpChar = ' ';
				switch (comp)	{
				case 0:		tmpChar = 'X';		break;
				case 1:		tmpChar = 'Y';		break;
				case 2:		tmpChar = 'Z';		break;
				}
				cout << FmtString("[%d-%c] %0.2f\t\t",point,tmpChar,*floatPtr);
				++floatPtr;
			}
			cout << endl;
		}
		glBindBuffer(GL_ARRAY_BUFFER, dstColorVBO->name);
		glGetBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(colorQuad), &colorQuad);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		cout << "dstColorVBO:" << endl;
		floatPtr = (float*)&colorQuad;
		for (int point=0; point<4; ++point)	{
			for (int comp=0; comp<4; ++comp)	{
				char		tmpChar = ' ';
				switch (comp)	{
				case 0:		tmpChar = 'R';		break;
				case 1:		tmpChar = 'G';		break;
				case 2:		tmpChar = 'B';		break;
				case 3:		tmpChar = 'A';		break;
				}
				cout << FmtString("[%d-%c] %0.2f\t\t",point,tmpChar,*floatPtr);
				++floatPtr;
			}
			cout << endl;
		}
		
	}
	return self;
}


- (IBAction) renderClicked:(id)sender	{
	NSLog(@"%s",__func__);
	scene->render();
	
	
}


- (void) initForGL2	{

}
- (void) initForModernGL	{
	scene->getContext()->makeCurrentIfNotCurrent();
	
	void		*selfPtr = (void*)self;
	
	//	make a target quad, dump it to the src VBO
	//Quad<VVGL::VertXYZRGBA>		posColorQuad;
	Quad<VVGL::VertXYZ>		posQuad;
	Quad<VVGL::VertRGBA>	colorQuad;
	posQuad.populateGeo(VVGL::Rect(50., 50., 50., 50.));
	colorQuad.populateColor(VT_RGBA(1., 0., 0., 1.));
	
	glBindBuffer(GL_ARRAY_BUFFER, srcGeoVBO->name);
	glBufferData(GL_ARRAY_BUFFER, sizeof(posQuad), &posQuad, GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glBindBuffer(GL_ARRAY_BUFFER, srcColorVBO->name);
	glBufferData(GL_ARRAY_BUFFER, sizeof(colorQuad), &colorQuad, GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	//	load the vertex shader from the app bundle, pass it to the scene
	NSString		*nsVSString = [NSString stringWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"TransFeedback_Feedback" ofType:@"vs"] encoding:NSUTF8StringEncoding error:nil];
	string			vsString = string([nsVSString UTF8String]);
	scene->setVertexShaderString(vsString);
	scene->setPerformClear(false);
	
	//	make a VAO and the attrib caches we'll need to render stuff
	GLBufferRef	vao = CreateVAO(true);
	GLCachedAttribRef		inXYZAttr = make_shared<GLCachedAttrib>("inXYZ");
	GLCachedAttribRef		inRGBAAttr = make_shared<GLCachedAttrib>("inRGBA");
	//GLCachedAttribRef		outXYZAttr = make_shared<GLCachedAttrib>("outXYZ");
	//	we also need an FBO- if we don't have one, we get a GL error- and of course the FBO needs an attachment or there's an error for that, too.
	GLBufferRef			fbo = CreateFBO();
	GLBufferRef			fboTex = CreateBGRATex(VVGL::Size(50.,50.));
	//	transform feedbacks require a special step after shader compilation but before program linking
	scene->setRenderPreLinkCallback([](const GLScene & n)	{
		NSLog(@"\t\trender pre-link called...");
		const GLchar * feedbackVaryings[] = { "outXYZ", "outRGBA" };
		glTransformFeedbackVaryings(n.getProgram(), 2, feedbackVaryings, GL_SEPARATE_ATTRIBS);
	});
	scene->setRenderPrepCallback([=](const GLScene & n, const bool & inReshaped, const bool & inPgmChanged)	{
		NSLog(@"\t\trender prep called...");
		if (inPgmChanged)	{
			GLint				myProgram = n.getProgram();
			inXYZAttr->cacheTheLoc(myProgram);
			inRGBAAttr->cacheTheLoc(myProgram);
			//outXYZAttr->cacheTheLoc(myProgram);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, fbo->name);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, fboTex->desc.target, fboTex->name, 0);
		/*
		GLenum		check = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (check == GL_FRAMEBUFFER_COMPLETE)	{
			//	intentionally blank, FBO complete is good!
		}
		else	{
			GLenum		tmp = GL_ONE;
			
			switch (check)	{
			case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:	NSLog(@"\t\terr: incomplete attachment framebuffer"); break;
			//case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:	NSLog(@"\t\terr: incomplete dimensions framebuffer"); break;
			case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:	NSLog(@"\t\terr: incomplete missing attachment framebuffer"); break;
			case GL_FRAMEBUFFER_UNSUPPORTED:	NSLog(@"\t\terr: unsupported framebuffer"); break;
			default:	NSLog(@"\t\terr: unrecognized framebuffer error, %X",check); break;
			}
		}
		*/
	});
	scene->setRenderCallback([=](const GLScene & n)	{
		NSLog(@"\t\trender called...");
		//	bind the VAO
		glBindVertexArray(vao->name);
		
		//	bind the VBO with the data i'm passing to the vertex shader
		glBindBuffer(GL_ARRAY_BUFFER, [(id)selfPtr srcGeoVBO]->name);
		//	configure the attribute pointers to use the VBO
		if (inXYZAttr->loc >= 0)	{
			inXYZAttr->enable();
			glVertexAttribPointer(inXYZAttr->loc, 3, GL_FLOAT, GL_FALSE, posQuad.stride(), BUFFER_OFFSET(posQuad.geoOffset()));
		}
		else
			NSLog(@"\t\terr: couldn't locate inXYZ attr, %s",__func__);
		
		glBindBuffer(GL_ARRAY_BUFFER, [(id)selfPtr srcColorVBO]->name);
		if (inRGBAAttr->loc >= 0)	{
			inRGBAAttr->enable();
			glVertexAttribPointer(inRGBAAttr->loc, 3, GL_FLOAT, GL_FALSE, colorQuad.stride(), BUFFER_OFFSET(colorQuad.colorOffset()));
		}
		else
			NSLog(@"\t\terr: couldn't locate inRGBA attr, %s",__func__);
		//	un-bind the src VBO
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	
		//	bind the dst VBO
		//glBindBuffer(GL_ARRAY_BUFFER, [(id)selfPtr dstVBO]->name);
	
		//	disable the rasterizer!
		glEnable(GL_RASTERIZER_DISCARD);
	
		//	configure the attribute pointers to read back into the VBO
		//if (outXYZAttr->loc >= 0)	{
		//	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, outXYZAttr->loc, [(id)selfPtr dstVBO]->name);
		//}
		//else	{
		//	NSLog(@"\t\terr: couldn't locate outXYZ attr, %s",__func__);
			glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, [(id)selfPtr dstGeoVBO]->name);
			//	i don't know why this isn't necessary?  is it because we're using an interleaved transform feedback and all the outputs are expected to go to a single buffer?  but then why are inputs handled so radically differently?
			glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, [(id)selfPtr dstColorVBO]->name);
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
	scene->setRenderCleanupCallback([=](const GLScene & n)	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	});
}


@synthesize srcGeoVBO;
@synthesize srcColorVBO;
@synthesize dstGeoVBO;
@synthesize dstColorVBO;

@end
