#import "VVGLBufferGLView.h"
//#include "GLContext.hpp"




#define VVRELEASE(item) {if (item != nil)	{			\
	[item release];										\
	item = nil;											\
}}

@interface VVGLBufferGLView()	{
	
}
@property (assign,readwrite,setter=setVAO:,getter=vao) VVGL::GLBufferRef vao;
@property (assign,readwrite) BOOL initialized;
@property (readonly) VVGL::GLBufferRef retainDrawBuffer;
@property (assign,readwrite) VVGL::Quad<VertXYST> lastVBOCoords;
@end




@implementation VVGLBufferGLView


- (id) initWithFrame:(NSRect)f	{
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
}
- (void) dealloc	{
	pthread_mutex_destroy(&renderLock);
	OSSpinLockLock(&retainDrawLock);
	retainDrawBuffer = nullptr;
	OSSpinLockUnlock(&retainDrawLock);
	[super dealloc];
}
- (void) drawRect:(NSRect)r	{
	
	pthread_mutex_lock(&renderLock);
		if (!initialized)	{
			//using namespace VVISF;
			
			GLBufferPoolRef		bp = GetGlobalBufferPool();
			if (bp != nullptr)	{
				[self setSharedGLContext:bp->getContext()];
				initialized = YES;
			}
		}
	pthread_mutex_unlock(&renderLock);
	
	if (initialized)
		[self redraw];
	
}
- (void) redraw	{
	//using namespace VVISF;
	
	GLBufferRef	lastBuffer = nullptr;
	OSSpinLockLock(&retainDrawLock);
	lastBuffer = (!retainDraw || retainDrawBuffer==nullptr) ? nullptr : retainDrawBuffer;
	OSSpinLockUnlock(&retainDrawLock);
	
	[self drawBuffer:lastBuffer];
}
- (void) drawBuffer:(VVGL::GLBufferRef)b	{
	//NSLog(@"%s ... %p, %s",__func__,self,b->getDescriptionString().c_str());
	
	BOOL			bail = NO;
	
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
	NSRect		tmpBounds = [self backingBounds];
	scene->setOrthoSize(VVGL::Size(tmpBounds.size.width, tmpBounds.size.height));
	
	pthread_mutex_lock(&renderLock);
		if (initialized)	{
			[[self openGLContext] makeCurrentContext];
			scene->render();
		}
	pthread_mutex_unlock(&renderLock);
	
	
	OSSpinLockLock(&retainDrawLock);
	if (!retainDraw)	{
		retainDrawBuffer = nullptr;
	}
	OSSpinLockUnlock(&retainDrawLock);
}
- (void) setSharedGLContext:(const GLContextRef)n	{
	//NSLog(@"%s ... %p",__func__,self);
	pthread_mutex_lock(&renderLock);
	
	void			*selfPtr = (void*)self;
	//	make a new scene- this also makes a new GL context that shares the passed context
	scene = CreateGLSceneRefUsing(n->newContextSharingMe());
	
	if (scene->getGLVersion() == GLVersion_2)	{
		//NSLog(@"\t\tGL 2");
		scene->setRenderPrepCallback([](const GLScene & n, const bool & inReshaped, const bool & inPgmChanged)	{
			
		});
		scene->setRenderCallback([selfPtr](const GLScene & n)	{
			
			//CGLContextObj		cgl_ctx = [[self openGLContext] CGLContextObj];
			glEnableClientState(GL_VERTEX_ARRAY);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			
			//glBlendFunc(GL_ONE, GL_ZERO);
			//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glHint(GL_CLIP_VOLUME_CLIPPING_HINT_EXT, GL_FASTEST);
			glDisable(GL_DEPTH_TEST);
			//glClearColor(0.0, 0.0, 0.0, 1.0);
			
			//glActiveTexture(GL_TEXTURE0);
			//glEnable(target);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			
			//	bilinear filtering stuff
			//glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			//glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			//glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			//glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			
			//	set up the view to draw
			NSRect				bounds = [(id)selfPtr backingBounds];
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glViewport(0, 0, (GLsizei) bounds.size.width, (GLsizei) bounds.size.height);
			//if (flipped)
				glOrtho(bounds.origin.x, bounds.origin.x+bounds.size.width, bounds.origin.y, bounds.origin.y+bounds.size.height, 1.0, -1.0);
			//else
			//	glOrtho(bounds.origin.x, bounds.origin.x+bounds.size.width, bounds.origin.y+bounds.size.height, bounds.origin.y, 1.0, -1.0);
			//glEnable(GL_BLEND);
			//glBlendFunc(GL_SRC_ALPHA,GL_DST_ALPHA);
			//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glDisable(GL_BLEND);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
			//	clear the view
			//glClearColor(0.0,0.0,0.0,0.0);
			//glClear(GL_COLOR_BUFFER_BIT);
			
			
			//	get the buffer we want to draw
			GLBufferRef		bufferToDraw = [(id)selfPtr retainDrawBuffer];
			if (bufferToDraw != nullptr)	{
				//	make a quad struct that describes XYST geometry, populate it with the coords of the quad we want to draw and the coords of the texture we want to draw on it
				NSRect				rawBounds = [(id)selfPtr backingBounds];
				VVGL::Rect			viewBoundsRect = VVGL::Rect(0., 0., rawBounds.size.width, rawBounds.size.height);
				VVGL::Rect			geometryRect = ResizeRect(bufferToDraw->srcRect, viewBoundsRect, SizingMode_Fit);
				Quad<VertXYZST>		targetQuad;
				targetQuad.populateGeo(geometryRect);
				targetQuad.populateTex(bufferToDraw->glReadySrcRect(), bufferToDraw->flipped);
				
				glEnableClientState(GL_VERTEX_ARRAY);
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				glDisableClientState(GL_COLOR_ARRAY);
	
				glVertexPointer(2, GL_FLOAT, targetQuad.stride(), (float*)&targetQuad);
				//glTexCoordPointer(2, GL_FLOAT, targetQuad.stride(), (float*)&targetQuad + (2*sizeof(float)));
				glTexCoordPointer(2, GL_FLOAT, targetQuad.stride(), &targetQuad.bl.tex);
				glActiveTexture(GL_TEXTURE0);
				glEnable(bufferToDraw->desc.target);
				glBindTexture(bufferToDraw->desc.target, bufferToDraw->name);
				//glDrawArrays(GL_QUADS, 0, 4);
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
				glBindTexture(bufferToDraw->desc.target, 0);
				glDisable(bufferToDraw->desc.target);
			}
		});
		
	}
	else	{
		//NSLog(@"\t\tGL 3+");
		//	load the frag/vert shaders
		string			vsString("\r\
#version 330 core\r\
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
#version 330 core\r\
in vec2		programST;\r\
uniform sampler2D		inputImage;\r\
uniform sampler2DRect	inputImageRect;\r\
uniform int		isRectTex;\r\
out vec4		FragColor;\r\
void main()	{\r\
if (isRectTex==0)\r\
	FragColor = vec4(0,0,0,1);\r\
else if (isRectTex==1)\r\
	FragColor = texture(inputImage,programST);\r\
else\r\
	FragColor = texture(inputImageRect,programST);\r\
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
		GLCachedUniRef		inputImageUni = make_shared<GLCachedUni>("inputImage");
		GLCachedUniRef		inputImageRectUni = make_shared<GLCachedUni>("inputImageRect");
		GLCachedUniRef		isRectTexUni = make_shared<GLCachedUni>("isRectTex");
		//	the render prep callback needs to create & populate a VAO, and cache the location of the vertex attributes and uniforms
		scene->setRenderPrepCallback([xyzAttr,stAttr,inputImageUni,inputImageRectUni,isRectTexUni,selfPtr](const GLScene & n, const bool & inReshaped, const bool & inPgmChanged)	{
			//cout << __PRETTY_FUNCTION__ << endl;
			if (inPgmChanged)	{
				//	cache all the locations for the vertex attributes & uniform locations
				GLint				myProgram = n.getProgram();
				xyzAttr->cacheTheLoc(myProgram);
				stAttr->cacheTheLoc(myProgram);
				inputImageUni->cacheTheLoc(myProgram);
				inputImageRectUni->cacheTheLoc(myProgram);
				isRectTexUni->cacheTheLoc(myProgram);
			}
		});
		//	the render callback passes all the data to the GL program
		scene->setRenderCallback([xyzAttr,stAttr,inputImageUni,inputImageRectUni,isRectTexUni,selfPtr](const GLScene & n)	{
			//cout << __PRETTY_FUNCTION__ << endl;
			//	clear
			glClearColor(0., 0., 0., 1.);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
			//	get the buffer we want to draw
			GLBufferRef		bufferToDraw = [(id)selfPtr retainDrawBuffer];
			if (bufferToDraw == nullptr)
				return;
			//	try to get the VAO.  if the VAO's null, create it and store it in the VVGLBufferGLView as an ivar. 
			GLBufferRef		tmpVAO = [(id)selfPtr vao];
			if (tmpVAO == nullptr)	{
				GLBufferPoolRef		bp = (bufferToDraw==nullptr) ? nullptr : bufferToDraw->parentBufferPool;
				if (bp != nullptr)	{
					tmpVAO = CreateVAO(true, bp);
					[(id)selfPtr setVAO:tmpVAO];
				}
			}
			//	if there's still no VAO, something's wrong- bail
			if (tmpVAO == nullptr)	{
				cout << "\terr: null VAO, bailing " << __PRETTY_FUNCTION__ << endl;
				return;
			}
			
			//	make a quad struct that describes XYST geometry, populate it with the coords of the quad we want to draw and the coords of the texture we want to draw on it
			NSRect				rawBounds = [(id)selfPtr backingBounds];
			VVGL::Rect			viewBoundsRect = VVGL::Rect(0., 0., rawBounds.size.width, rawBounds.size.height);
			VVGL::Rect			geometryRect = ResizeRect((bufferToDraw==nullptr) ? viewBoundsRect : bufferToDraw->srcRect, viewBoundsRect, SizingMode_Fit);
			Quad<VertXYST>		targetQuad;
			targetQuad.populateGeo(geometryRect);
			targetQuad.populateTex((bufferToDraw==nullptr) ? geometryRect : bufferToDraw->glReadySrcRect(), (bufferToDraw==nullptr) ? false : bufferToDraw->flipped);
		
			//	pass the 2D texture to the program (if there's a 2D texture)
			glActiveTexture(GL_TEXTURE0);
			GLERRLOG
			glBindTexture(GLBuffer::Target_2D, (bufferToDraw!=nullptr && bufferToDraw->desc.target==GLBuffer::Target_2D) ? bufferToDraw->name : 0);
			GLERRLOG
			glBindTexture(GLBuffer::Target_Rect, 0);
			GLERRLOG
			if (inputImageUni->loc >= 0)	{
				glUniform1i(inputImageUni->loc, 0);
				GLERRLOG
			}
			//	pass the RECT texture to the program (if there's a RECT texture)
			glActiveTexture(GL_TEXTURE1);
			GLERRLOG
			glBindTexture(GLBuffer::Target_2D, 0);
			GLERRLOG
			glBindTexture(GLBuffer::Target_Rect, (bufferToDraw!=nullptr && bufferToDraw->desc.target==GLBuffer::Target_Rect) ? bufferToDraw->name : 0);
			GLERRLOG
			if (inputImageRectUni->loc >= 0)	{
				glUniform1i(inputImageRectUni->loc, 1);
				GLERRLOG
			}
			//	pass an int to the program that indicates whether we're passing no texture (0), a 2D texture (1) or a RECT texture (2)
			if (isRectTexUni->loc >= 0)	{
				if (bufferToDraw == nullptr)
					glUniform1i(isRectTexUni->loc, 0);
				else	{
					switch (bufferToDraw->desc.target)	{
					case GLBuffer::Target_2D:
						glUniform1i(isRectTexUni->loc, 1);
						break;
					case GLBuffer::Target_Rect:
						glUniform1i(isRectTexUni->loc, 2);
						break;
					default:
						glUniform1i(isRectTexUni->loc, 0);
						break;
					}
				}
				GLERRLOG
			}
		
			//	bind the VAO
			glBindVertexArray(tmpVAO->name);
			GLERRLOG
			
			uint32_t			vbo = 0;
			if ([(id)selfPtr lastVBOCoords] != targetQuad)	{
				//	make a new VBO to contain vertex + texture coord data
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
			}
			
			//	draw
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			GLERRLOG
			//	un-bind the VAO
			glBindVertexArray(0);
			GLERRLOG
			
			if ([(id)selfPtr lastVBOCoords] != targetQuad)	{
				//	delete the VBO we made earlier...
				glDeleteBuffers(1, &vbo);
				GLERRLOG
				//	update the vbo coords ivar (we don't want to update the VBO contents every pass)
				[(id)selfPtr setLastVBOCoords:targetQuad];
			}
			
		});
	
	}
	
	NSOpenGLContext		*sceneCtxWrapper = [[NSOpenGLContext alloc] initWithCGLContextObj:scene->getContext()->ctx];
	if (sceneCtxWrapper != nil)	{
		[self setOpenGLContext:sceneCtxWrapper];
		[sceneCtxWrapper setView:self];
		[sceneCtxWrapper release];
		long				swap = 1;
		[[self openGLContext] setValues:(GLint *)&swap forParameter:NSOpenGLCPSwapInterval];
		initialized = YES;
	}
	
	pthread_mutex_unlock(&renderLock);
}


@synthesize initialized;
@synthesize sizingMode;
@synthesize retainDrawBuffer;
@synthesize vao;
@synthesize lastVBOCoords;


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








@implementation NSOpenGLView (NSOpenGLViewVVGLBufferViewAdditions)
- (NSRect) backingBounds	{
	return [(id)self convertRectToBacking:[self bounds]];
}
@end

