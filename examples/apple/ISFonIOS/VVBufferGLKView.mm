#import "VVBufferGLKView.h"




#define VVRELEASE(item) {if (item != nil)	{			\
	[item release];										\
	item = nil;											\
}}

@interface VVBufferGLKView()	{
	
}
@property (assign,readwrite,setter=setVAO:,getter=vao) VVGL::GLBufferRef vao;
@property (assign,readwrite) BOOL initialized;
@property (readonly) VVGL::GLBufferRef retainDrawBuffer;
@end




@implementation VVBufferGLKView


- (id) initWithFrame:(CGRect)f	{
	NSLog(@"%s",__func__);
	if (self = [super initWithFrame:f])	{
		pthread_mutexattr_t		attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
		pthread_mutex_init(&renderLock, &attr);
		pthread_mutexattr_destroy(&attr);
		initialized = NO;
		scene = nullptr;
		vao = nullptr;
		sizingMode = VVGL::SizingMode_Fit;
		retainDraw = NO;
		retainDrawLock = OS_SPINLOCK_INIT;
		retainDrawBuffer = nullptr;
		onlyDrawNewStuff = NO;
		onlyDrawNewStuffLock = OS_SPINLOCK_INIT;
		onlyDrawNewStuffTimestamp = VVGL::Timestamp();
		return self;
	}
	if (self != nil)
		[self release];
	return nil;
}
- (id) initWithCoder:(NSCoder *)c	{
	NSLog(@"%s",__func__);
	if (self = [super initWithCoder:c])	{
		pthread_mutexattr_t		attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
		pthread_mutex_init(&renderLock, &attr);
		pthread_mutexattr_destroy(&attr);
		initialized = NO;
		scene = nullptr;
		vao = nullptr;
		sizingMode = VVGL::SizingMode_Fit;
		retainDraw = NO;
		retainDrawLock = OS_SPINLOCK_INIT;
		retainDrawBuffer = nullptr;
		onlyDrawNewStuff = NO;
		onlyDrawNewStuffLock = OS_SPINLOCK_INIT;
		onlyDrawNewStuffTimestamp = VVGL::Timestamp();
		return self;
	}
	if (self != nil)
		[self release];
	return nil;
}
- (void) awakeFromNib	{
	initialized = NO;
	[super awakeFromNib];
}
- (void) dealloc	{
	pthread_mutex_destroy(&renderLock);
	OSSpinLockLock(&retainDrawLock);
	retainDrawBuffer = nullptr;
	OSSpinLockUnlock(&retainDrawLock);
	[super dealloc];
}
- (void) drawRect:(CGRect)r	{
	//NSLog(@"%s",__func__);
	
	pthread_mutex_lock(&renderLock);
		if (initialized && scene!=nullptr)	{
			scene->render(GLScene::RenderTarget());
		}
	pthread_mutex_unlock(&renderLock);
}
- (void) redraw	{
	GLBufferRef	lastBuffer = nullptr;
	OSSpinLockLock(&retainDrawLock);
	lastBuffer = (!retainDraw || retainDrawBuffer==nullptr) ? nullptr : retainDrawBuffer;
	OSSpinLockUnlock(&retainDrawLock);
	
	[self drawBuffer:lastBuffer];
}
- (void) drawBuffer:(VVGL::GLBufferRef)b	{
	//NSLog(@"%s ... %s",__func__,b->getDescriptionString().c_str());
	BOOL			bail = NO;
	pthread_mutex_lock(&renderLock);
	if (!initialized || scene==nullptr)	{
		NSLog(@"\t\tneed to initialize %@",[self class]);
		auto			bp = GetGlobalBufferPool();
		auto			bpCtx = (bp==nullptr) ? nullptr : bp->getContext();
		if (bpCtx != nullptr)	{
			[self setSharedGLContext:bpCtx];
			initialized = YES;
		}
		
		if (!initialized || scene==nullptr)	{
			bail = YES;
			NSLog(@"\t\tbailing, view not initialized, %s",__func__);
		}
	}
	pthread_mutex_unlock(&renderLock);
	if (bail)
		return;
	
	OSSpinLockLock(&retainDrawLock);
	//if (retainDraw)	{
		retainDrawBuffer = b;
	//}
	OSSpinLockUnlock(&retainDrawLock);
	
	OSSpinLockLock(&onlyDrawNewStuffLock);
	if (onlyDrawNewStuff)	{
		if (onlyDrawNewStuffTimestamp == b->contentTimestamp)
			bail = YES;
	}
	OSSpinLockUnlock(&onlyDrawNewStuffLock);
	if (bail)
		return;
	
	//	we probably shouldn't do this every time we draw a buffer.
	CGRect		tmpBounds = CGRectMake(0,0,[self drawableWidth],[self drawableHeight]);
	scene->setOrthoSize(VVGL::Size(tmpBounds.size.width, tmpBounds.size.height));
	
	pthread_mutex_lock(&renderLock);
		if (initialized)	{
			[self display];
		}
	pthread_mutex_unlock(&renderLock);
	
	
	OSSpinLockLock(&retainDrawLock);
	if (!retainDraw)	{
		retainDrawBuffer = nullptr;
	}
	OSSpinLockUnlock(&retainDrawLock);
}
- (void) setSharedGLContext:(const GLContextRef)n	{
	
	pthread_mutex_lock(&renderLock);
	
	void			*selfPtr = (void*)self;
	//	make a new scene- this also makes a new GL context that shares the passed context
	scene = make_shared<GLScene>(n->newContextSharingMe());
	//	load the frag/vert shaders
	string			vsString("\r\
#version 300 es\r\
in vec3		inXYZ;\r\
in vec2		inST;\r\
uniform mat4	vvglOrthoProj;\r\
out vec2		programST;\r\
void main()	{\r\
gl_Position = vec4(inXYZ.x, inXYZ.y, inXYZ.z, 1.0) * vvglOrthoProj;\r\
programST = inST;\r\
}\r\
");
	string			fsString("\r\
#version 300 es\r\
precision mediump		float;\r\
in vec2		programST;\r\
uniform sampler2D		inputImage;\r\
//uniform sampler2DRect	inputImageRect;\r\
uniform int		isRectTex;\r\
out vec4		FragColor;\r\
void main()	{\r\
if (isRectTex==0)\r\
	FragColor = vec4(0,0,0,1);\r\
else if (isRectTex==1)\r\
	FragColor = texture(inputImage,programST);\r\
//else\r\
//	FragColor = texture(inputImageRect,programST);\r\
}\r\
");
	scene->setVertexShaderString(vsString);
	scene->setFragmentShaderString(fsString);
	//	we're going to create a couple vars on the stack here- the vars themselves are shared 
	//	ptrs, so when they're copied by value in the callback blocks the copies will refer to 
	//	the same underlying vars, which will be retained until these callback blocks are 
	//	destroyed and shared between the callback lambdas...
	GLCachedAttribRef		xyzAttr = make_shared<GLCachedAttrib>("inXYZ");
	GLCachedAttribRef		stAttr = make_shared<GLCachedAttrib>("inST");
	GLCachedUniRef		inputImage = make_shared<GLCachedUni>("inputImage");
	//GLCachedUniRef		inputImageRect = make_shared<GLCachedUni>("inputImageRect");
	GLCachedUniRef		isRectTex = make_shared<GLCachedUni>("isRectTex");
	//	the render prep callback needs to create & populate a VAO, and cache the location of the vertex attributes and uniforms
	scene->setRenderPrepCallback([xyzAttr,stAttr,inputImage,isRectTex,selfPtr](const GLScene & n, const bool & inReshaped, const bool & inPgmChanged)	{
		//cout << __PRETTY_FUNCTION__ << endl;
		
		if (inPgmChanged)	{
			//	cache all the locations for the vertex attributes & uniform locations
			GLint				myProgram = n.getProgram();
			xyzAttr->cacheTheLoc(myProgram);
			stAttr->cacheTheLoc(myProgram);
			inputImage->cacheTheLoc(myProgram);
			//inputImageRect->cacheTheLoc(myProgram);
			isRectTex->cacheTheLoc(myProgram);
			
			//	make a quad struct that describes XYST geometry.  we don't have to populate it now (we'll update it during the render pass)
			Quad<VertXYST>		targetQuad;
			
			//	create a new VAO, store it in the VVBufferGLKView as an ivar.  don't bother populating it now.
			GLBufferRef		tmpVAO = CreateVAO(true);
			[(id)selfPtr setVAO:tmpVAO];
		}
		
		
	});
	//	the render callback passes all the data to the GL program
	scene->setRenderCallback([xyzAttr,stAttr,inputImage,isRectTex,selfPtr](const GLScene & n)	{
		//cout << __PRETTY_FUNCTION__ << endl;
		
		//	get the buffer we want to draw
		GLBufferRef		bufferToDraw = [(id)selfPtr retainDrawBuffer];
		//	make a quad struct that describes XYST geometry, populate it with the coords of the quad we want to draw and the coords of the texture we want to draw on it
		CGRect				rawBounds = CGRectMake(0,0,[(VVBufferGLKView *)selfPtr drawableWidth],[(VVBufferGLKView *)selfPtr drawableHeight]);
		VVGL::Rect			viewBoundsRect = VVGL::Rect(0., 0., rawBounds.size.width, rawBounds.size.height);
		VVGL::Rect			geometryRect = ResizeRect((bufferToDraw==nullptr) ? viewBoundsRect : bufferToDraw->srcRect, viewBoundsRect, SizingMode_Fit);
		VVGL::Quad<VertXYST>	targetQuad;
		//GLBufferQuadPopulate(&targetQuad, geometryRect, (bufferToDraw==nullptr) ? geometryRect : bufferToDraw->glReadySrcRect(), (bufferToDraw==nullptr) ? false : bufferToDraw->flipped);
		targetQuad.populateGeo(geometryRect);
		targetQuad.populateTex((bufferToDraw==nullptr) ? geometryRect : bufferToDraw->glReadySrcRect(), (bufferToDraw==nullptr) ? false : bufferToDraw->flipped);
		
		//	pass the 2D texture to the program (if there's a 2D texture)
		glActiveTexture(GL_TEXTURE0);
		GLERRLOG
		glBindTexture(GLBuffer::Target_2D, (bufferToDraw!=nullptr && bufferToDraw->desc.target==GLBuffer::Target_2D) ? bufferToDraw->name : 0);
		GLERRLOG
		//glBindTexture(GLBuffer::Target_Rect, 0);
		//GLERRLOG
		if (inputImage->loc >= 0)	{
			glUniform1i(inputImage->loc, 0);
			GLERRLOG
		}
		//	pass the RECT texture to the program (if there's a RECT texture)
		//glActiveTexture(GL_TEXTURE1);
		//GLERRLOG
		//glBindTexture(GLBuffer::Target_2D, 0);
		//GLERRLOG
		//glBindTexture(GLBuffer::Target_Rect, (bufferToDraw!=nullptr && bufferToDraw->desc.target==GLBuffer::Target_Rect) ? bufferToDraw->name : 0);
		//GLERRLOG
		//if (inputImageRect->loc >= 0)	{
		//	glUniform1i(inputImageRect->loc, 1);
		//	GLERRLOG
		//}
		//	pass an int to the program that indicates whether we're passing no texture (0), a 2D texture (1) or a RECT texture (2)
		if (isRectTex->loc >= 0)	{
			if (bufferToDraw == nullptr)
				glUniform1i(isRectTex->loc, 0);
			else	{
				switch (bufferToDraw->desc.target)	{
				case GLBuffer::Target_2D:
					glUniform1i(isRectTex->loc, 1);
					break;
				//case GLBuffer::Target_Rect:
				//	glUniform1i(isRectTex->loc, 2);
				//	break;
				default:
					glUniform1i(isRectTex->loc, 0);
					break;
				}
			}
			GLERRLOG
		}
		
		//	bind the VAO
		GLBufferRef		tmpVAO = [(id)selfPtr vao];
		glBindVertexArray(tmpVAO->name);
		GLERRLOG
		
		//	make a new VBO to contain vertex + texture coord data
		uint32_t			vbo = 0;
		glGenBuffers(1, &vbo);
		GLERRLOG
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		GLERRLOG
		glBufferData(GL_ARRAY_BUFFER, sizeof(targetQuad), (void*)&targetQuad, GL_STATIC_DRAW);
		GLERRLOG
		//	configure the attribute pointers to use the VBO
		if (xyzAttr->loc >= 0)	{
			glVertexAttribPointer(xyzAttr->loc, 2, GL_FLOAT, GL_FALSE, targetQuad.stride(), BUFFER_OFFSET(targetQuad.geoOffset()));
			GLERRLOG
			xyzAttr->enable();
		}
		if (stAttr->loc >= 0)	{
			glVertexAttribPointer(stAttr->loc, 2, GL_FLOAT, GL_FALSE, targetQuad.stride(), BUFFER_OFFSET(targetQuad.texOffset()));
			GLERRLOG
			stAttr->enable();
		}
		
		//	draw
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		GLERRLOG
		//	un-bind the VAO
		glBindVertexArray(0);
		GLERRLOG
		
		//	delete the VBO we made earlier...
		glDeleteBuffers(1, &vbo);
		GLERRLOG
		
	});
	
	GLContextRef	sceneCtx = scene->getContext();
	EAGLContext		*sceneGLCtx = (sceneCtx==nullptr) ? nil : (EAGLContext *)(sceneCtx->ctx);
	if (sceneGLCtx != nil)	{
		[self setContext:sceneGLCtx];
		initialized = YES;
	}
	
	pthread_mutex_unlock(&renderLock);
}


@synthesize initialized;
@synthesize sizingMode;
@synthesize retainDrawBuffer;
@synthesize vao;
- (void) setOnlyDrawNewStuff:(BOOL)n	{
	//using namespace VVISF;
	OSSpinLockLock(&onlyDrawNewStuffLock);
	onlyDrawNewStuff = n;
	onlyDrawNewStuffTimestamp = Timestamp();
	OSSpinLockUnlock(&onlyDrawNewStuffLock);
}
- (void) setRetainDraw:(BOOL)n	{
	OSSpinLockLock(&retainDrawLock);
	retainDraw = n;
	OSSpinLockUnlock(&retainDrawLock);
}
- (void) setRetainDrawBuffer:(VVGL::GLBufferRef)n	{
	OSSpinLockLock(&retainDrawLock);
	retainDrawBuffer = n;
	OSSpinLockUnlock(&retainDrawLock);
}
- (BOOL) onlyDrawNewStuff	{
	BOOL		returnMe = NO;
	OSSpinLockLock(&onlyDrawNewStuffLock);
	returnMe = onlyDrawNewStuff;
	OSSpinLockUnlock(&onlyDrawNewStuffLock);
	return returnMe;
}


@end
