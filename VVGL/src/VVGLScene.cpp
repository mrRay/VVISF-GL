#include "VVGLScene.hpp"




namespace VVGL
{


using namespace std;




/*	========================================	*/
#pragma mark --------------------- create/destroy


VVGLScene::VVGLScene()	{
	//cout << __PRETTY_FUNCTION__ << endl;
	VVGLBufferPoolRef		bp = GetGlobalBufferPool();
	if (bp != nullptr)	{
		context = bp->getContext()->newContextSharingMe();
		//cout << "\tcontext is " << *context << endl;
	}
}
VVGLScene::VVGLScene(const VVGLContextRef & inCtx)	{
	//context = new VVGLContext(inCtx);
	context = inCtx;
}

void VVGLScene::prepareToBeDeleted()	{
	//cout << __PRETTY_FUNCTION__ << "->" << this << endl;
	
	//lock_guard<recursive_mutex>		lock(renderLock);
	
	renderCallback = nullptr;
	
	deleted = true;
}

#if !ISF_TARGET_IOS
VVGLScene::~VVGLScene()	{
	//cout << __PRETTY_FUNCTION__ << "->" << this << endl;
	if (!deleted)
		prepareToBeDeleted();
	
	if (context != nullptr)	{
		//delete context;
		context = nullptr;
	}
}
#endif


/*	========================================	*/
#pragma mark --------------------- rendering methods




VVGLBufferRef VVGLScene::createAndRenderABuffer(const Size & inSize, const VVGLBufferPoolRef & inPool)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	if (inPool == nullptr)
		return nullptr;
	
	//	set the size
	setSize(inSize);
	//	make the buffers i'll be rendering into
	RenderTarget		tmpTarget(CreateFBO(inPool), CreateRGBATex(size,inPool), CreateDepthBuffer(size,inPool));
	//RenderTarget		tmpTarget(CreateFBO(inPool), CreateBGRATex(size,inPool), CreateDepthBuffer(size,inPool));
	//	render
	render(tmpTarget);
	
	return tmpTarget.color;
}
void VVGLScene::renderToBuffer(const VVGLBufferRef & inBuffer)	{
	cout << __PRETTY_FUNCTION__ << ", passed buffer is " << inBuffer << endl;
	//	set the size
	if (inBuffer != nullptr)
		setSize(inBuffer->srcRect.size);
	//	make the buffers i'll be rendering into
	RenderTarget		tmpTarget(CreateFBO(), inBuffer, CreateDepthBuffer(size));
	//	render
	render(tmpTarget);
}

void VVGLScene::render(const RenderTarget & inRenderTarget)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	//cout << "\trender target is " << inRenderTarget.fboName() << ", " << inRenderTarget.colorName() << ", " << inRenderTarget.depthName() << endl;
	//	get a lock, set the current GL context
	lock_guard<recursive_mutex>		lock(renderLock);
	if (context == nullptr)	{
		cout << "\terr: bailing, ctx null, " << __PRETTY_FUNCTION__ << endl;
		return;
	}
	context->makeCurrentIfNotCurrent();
	
	//	update the member var for the fbo attachments
	renderTarget = inRenderTarget;
	//	prep for render
	_renderPrep();
	
	//	execute the render callback
	if (renderCallback != nullptr)
		renderCallback(*this);
	
	//	cleanup after render
	_renderCleanup();
	//	update the member vars for the fbo attachments
	renderTarget = RenderTarget();
}
void VVGLScene::renderBlackFrame(const RenderTarget & inRenderTarget)	{
	//	get a lock, set the current GL context
	lock_guard<recursive_mutex>		lock(renderLock);
	if (context == nullptr)	{
		cout << "\terr: bailing, ctx null, " << __PRETTY_FUNCTION__ << endl;
		return;
	}
	context->makeCurrentIfNotCurrent();
	
	//	update the member var for the fbo attachments
	renderTarget = inRenderTarget;
	//	prep for render
	_renderPrep();
	
	//	get the context, clear
	
	glClearColor(0., 0., 0., 0.);
	GLERRLOG
	uint32_t		mask = GL_COLOR_BUFFER_BIT;
#if !ISF_TARGET_IOS
	mask |= GL_DEPTH_BUFFER_BIT;
#endif
	glClear(mask);
	GLERRLOG
	clearColorUpdated = true;
	
	//	cleanup after render
	_renderCleanup();
	//	update the member vars for the fbo attachments
	renderTarget = RenderTarget();
}
void VVGLScene::renderOpaqueBlackFrame(const RenderTarget & inRenderTarget)	{
	//	get a lock, set the current GL context
	lock_guard<recursive_mutex>		lock(renderLock);
	if (context == nullptr)	{
		cout << "\terr: bailing, ctx null, " << __PRETTY_FUNCTION__ << endl;
		return;
	}
	context->makeCurrentIfNotCurrent();
	
	//	update the member var for the fbo attachments
	renderTarget = inRenderTarget;
	//	prep for render
	_renderPrep();
	
	//	get the context, clear
	glClearColor(0., 0., 0., 1.);
	GLERRLOG
	uint32_t		mask = GL_COLOR_BUFFER_BIT;
#if !ISF_TARGET_IOS
	mask |= GL_DEPTH_BUFFER_BIT;
#endif
	glClear(mask);
	GLERRLOG
	clearColorUpdated = true;
	
	//	cleanup after render
	_renderCleanup();
	//	update the member vars for the fbo attachments
	renderTarget = RenderTarget();
}
void VVGLScene::renderRedFrame(const RenderTarget & inRenderTarget)	{
	//	get a lock, set the current GL context
	lock_guard<recursive_mutex>		lock(renderLock);
	if (context == nullptr)	{
		cout << "\terr: bailing, ctx null, " << __PRETTY_FUNCTION__ << endl;
		return;
	}
	context->makeCurrentIfNotCurrent();
	
	//	update the member var for the fbo attachments
	renderTarget = inRenderTarget;
	//	prep for render
	_renderPrep();
	
	//	get the context, clear
	glClearColor(1., 0., 0., 1.);
	GLERRLOG
	uint32_t		mask = GL_COLOR_BUFFER_BIT;
#if !ISF_TARGET_IOS
	mask |= GL_DEPTH_BUFFER_BIT;
#endif
	glClear(mask);
	GLERRLOG
	clearColorUpdated = true;
	
	//	cleanup after render
	_renderCleanup();
	//	update the member vars for the fbo attachments
	renderTarget = RenderTarget();
}


/*	========================================	*/
#pragma mark --------------------- setter/getter methods


void VVGLScene::setRenderCallback(const RenderCallback & n)	{
	renderCallback = n;
}


void VVGLScene::setAlwaysNeedsReshape(const bool & n)	{
	lock_guard<recursive_mutex>		lock(renderLock);
	alwaysNeedsReshape = n;
}
void VVGLScene::setSize(const Size & n)	{
	lock_guard<recursive_mutex>		lock(renderLock);
	size = n;
	needsReshape = true;
}
Size VVGLScene::getSize() const	{
	return size;
}
void VVGLScene::setFlipped(const bool & n)	{
	lock_guard<recursive_mutex>		lock(renderLock);
	flipped = n;
	needsReshape = true;
}
bool VVGLScene::getFlipped() const	{
	return flipped;
}


void VVGLScene::setClearColor(const GLColor & n)	{
	if (clearColor == n)
		return;
	clearColorUpdated = true;
	clearColor = n;
}
void VVGLScene::setClearColor(const float & r, const float & g, const float & b, const float & a)	{
	setClearColor(GLColor(r,g,b,a));
}
void VVGLScene::setClearColor(float * n)	{
	setClearColor(GLColor(*(n+0), *(n+1), *(n+2), *(n+3)));
}
void VVGLScene::setPerformClear(const bool & n)	{
	performClear = n;
}


/*	========================================	*/
#pragma mark --------------------- protected rendering methods


void VVGLScene::_renderPrep()	{
	//cout << __PRETTY_FUNCTION__ << endl;
	//cout << "\tthis is " << this << endl;
	if (deleted)	{
		//cout << "\t" << __PRETTY_FUNCTION__ << "- FINISHED" << endl;
		return;
	}
	
	//	if the context is nil, create a context- bail if i can't
	if (context == nullptr)	{
		cout << "\terr: bailing, ctx null, " << __PRETTY_FUNCTION__ << endl;
		return;
	}
	
	//	initialize & reshape as needed
	if (!initialized)
		_initialize();
	if (alwaysNeedsReshape || needsReshape)
		_reshape();
	
	//	bind the attachments in the render target to the FBO (also in the render target)
	if (renderTarget.fboName() > 0)	{
		glBindFramebuffer(GL_FRAMEBUFFER, renderTarget.fboName());
		GLERRLOG
		
		//	attach the depth buffer
		if (renderTarget.depthName() > 0)	{
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, renderTarget.depthTarget(), renderTarget.depthName(), 0);
			GLERRLOG
		}
		else	{
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, renderTarget.depthTarget(), 0, 0);
			GLERRLOG
		}
		
		//	attach the color buffer
		if (renderTarget.colorName() > 0)	{
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderTarget.colorTarget(), renderTarget.colorName(), 0);
			GLERRLOG
		}
		else	{
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderTarget.colorTarget(), 0, 0);
			GLERRLOG
		}
	}
	
	//	clear
	if (clearColorUpdated)	{
		glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
		GLERRLOG
		clearColorUpdated = false;
	}
	if (performClear)	{
		uint32_t		mask = GL_COLOR_BUFFER_BIT;
#if !ISF_TARGET_IOS
		mask |= GL_DEPTH_BUFFER_BIT;
#endif
		glClear(mask);
		GLERRLOG
	}
	
	//cout << "\t" << __PRETTY_FUNCTION__ << "- FINISHED" << endl;
}

void VVGLScene::_initialize()	{
	//cout << __PRETTY_FUNCTION__ << endl;
	if (deleted)
		return;
	if (context == nullptr)
		return;
	
	//glEnable(GL_TEXTURE_RECTANGLE_EXT);
	//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glEnable(GL_BLEND);
	GLERRLOG
	//glDisable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	GLERRLOG
#if !ISF_TARGET_IOS && !ISF_TARGET_RPI
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	GLERRLOG
#endif
	//const int32_t		swap = 0;
	//CGLSetParameter(cgl_ctx, kCGLCPSwapInterval, &swap);
	
	//glHint(GL_CLIP_VOLUME_CLIPPING_HINT_EXT, GL_FASTEST);
	//glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);
	//glDisable(GL_DEPTH_TEST);
	
	//glClearColor(0.0, 0.0, 0.0, 0.0);
	
	initialized = true;
	clearColorUpdated = true;
}

void VVGLScene::_reshape()	{
	//cout << __PRETTY_FUNCTION__ << endl;
	//cout << "\tthis is " << this << endl;
	{
		lock_guard<mutex>		lock(projMatrixLock);
		double			left = 0.0;
		double			right = size.width;
		double			top = size.height;
		double			bottom = 0.0;
		double			far = 1.0;
		double			near = -1.0;
		if (flipped)	{
			top = 0.0;
			bottom = size.height;
		}
		
		projMatrix[0] = 2.0/(right-left);
		projMatrix[4] = 0.0;
		projMatrix[8] = 0.0;
		projMatrix[12] = -1.0*(right + left) / (right - left);
	
		projMatrix[1] = 0.0;
		projMatrix[5] = 2.0/(top-bottom);
		projMatrix[9] = 0.0;
		projMatrix[13] = -1.0*(top + bottom) / (top - bottom);
	
		projMatrix[2] = 0.0;
		projMatrix[6] = 0.0;
		projMatrix[10] = -2.0 / (far - near);
		projMatrix[14] = -1.0*(far + near) / (far - near);
	
		projMatrix[3] = 0.0;
		projMatrix[7] = 0.0;
		projMatrix[11] = 0.0;
		projMatrix[15] = 1.0;
		
#if ISF_TARGET_IOS
		_configProjMatrixEffect();
#endif
	}
	
#if !ISF_TARGET_IOS && !ISF_TARGET_RPI
	glMatrixMode(GL_MODELVIEW);
	GLERRLOG
	glLoadIdentity();
	GLERRLOG
	glMatrixMode(GL_PROJECTION);
	GLERRLOG
	glLoadIdentity();
	GLERRLOG
	if (!flipped)	{
		glOrtho(0, size.width, 0, size.height, 1.0, -1.0);
		GLERRLOG
	}
	else	{
		glOrtho(0, size.width, size.height, 0, 1.0, -1.0);
		GLERRLOG
	}
#endif
	glViewport(0,0,size.width,size.height);
	GLERRLOG
	needsReshape = false;
}

void VVGLScene::_renderCleanup()	{
	//	flush
	glFlush();
	GLERRLOG
	//	unbind the render target's attachments
	if (renderTarget.fboName() > 0)	{
		if (renderTarget.depthName() > 0)	{
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 0, 0, 0);
			GLERRLOG
		}
		if (renderTarget.colorName() > 0)	{
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 0, 0, 0);
			GLERRLOG
		}
		//	unbind the framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER,0);
		GLERRLOG
	}
	
}


}
