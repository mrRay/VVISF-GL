#include "VVGLBufferCopier.hpp"

#include <vector>
#include <iostream>




namespace VVGL
{


using namespace std;




static VVGLBufferCopierRef * _globalBufferCopier = nullptr;




/*	========================================	*/
#pragma mark --------------------- constructor/destructor


#if !ISF_TARGET_RPI
VVGLBufferCopier::VVGLBufferCopier() : VVGLScene()	{
}
VVGLBufferCopier::VVGLBufferCopier(const VVGLContext * inCtx) : VVGLScene(inCtx)	{
}
#else
VVGLBufferCopier::VVGLBufferCopier() : VVGLShaderScene()	{
	//	if we're running on a raspberry pi (if we're in opengl es) then we need to set up a simple shader that draws a texture
	generalInit();
}
VVGLBufferCopier::VVGLBufferCopier(const VVGLContext * inCtx) : VVGLShaderScene(inCtx)	{
	//	if we're running on a raspberry pi (if we're in opengl es) then we need to set up a simple shader that draws a texture
	generalInit();
}
#endif

void VVGLBufferCopier::prepareToBeDeleted()	{
	//cout << __PRETTY_FUNCTION__ << "->" << this << endl;
	//	now call the super, which deletes the context
#if !ISF_TARGET_RPI
	VVGLScene::prepareToBeDeleted();
#else
	VVGLShaderScene::prepareToBeDeleted();
#endif
}

VVGLBufferCopier::~VVGLBufferCopier()	{
	//cout << __PRETTY_FUNCTION__ << "->" << this << endl;
	if (!deleted)
		prepareToBeDeleted();
	
	
}
void VVGLBufferCopier::generalInit()	{
#if ISF_TARGET_RPI
	cout << __PRETTY_FUNCTION__ << endl;
	//	set up simple frag & vert shaders that draw a tex
	string			fsString("\
precision mediump		float;\
uniform sampler2D		inputImage;\
varying vec2			vertSTVar;\
\
void main()	{\
	gl_FragColor = texture2D(inputImage, vertSTVar);\
}\
");
	string			vsString("\
attribute vec4			vertXYZ;\
attribute vec2			vertST;\
uniform vec4			orthoRect;\
varying vec2			vertSTVar;\
\
void main()	{\
	gl_Position = vertXYZ;\
	mat4			projectionMatrix = mat4(2./orthoRect[2], 0., 0., -1.,\
		0., 2./orthoRect[3], 0., -1.,\
		0., 0., -1., 0.,\
		0., 0., 0., 1.);\
	gl_Position *= projectionMatrix;\
	vertSTVar = vertST;\
}\
");
	setFragmentShaderString(fsString);
	setVertexShaderString(vsString);
#endif
}


/*	========================================	*/
#pragma mark --------------------- superclass overrides


void VVGLBufferCopier::_initialize()	{
#if !ISF_TARGET_RPI
	VVGLScene::_initialize();
#else
	VVGLShaderScene::_initialize();
#endif
#if !ISF_TARGET_IOS && !ISF_TARGET_RPI
	//CGLContextObj		cgl_ctx = context;
	glEnableClientState(GL_VERTEX_ARRAY);
	GLERRLOG
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	GLERRLOG
	glHint(GL_CLIP_VOLUME_CLIPPING_HINT_EXT, GL_FASTEST);
	GLERRLOG
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	GLERRLOG
#endif
	glDisable(GL_DEPTH_TEST);
	GLERRLOG
	glDisable(GL_BLEND);
	GLERRLOG
}


/*	========================================	*/
#pragma mark --------------------- getter/setter


#if ISF_TARGET_MAC
void VVGLBufferCopier::setCopyToIOSurface(const bool & n)	{
	lock_guard<recursive_mutex>		lock(renderLock);
	copyToIOSurface = n;
}
bool VVGLBufferCopier::getCopyToIOSurface()	{
	lock_guard<recursive_mutex>		lock(renderLock);
	return copyToIOSurface;
}
#endif
void VVGLBufferCopier::setCopyAndResize(const bool & n)	{
	lock_guard<recursive_mutex>		lock(renderLock);
	copyAndResize = n;
}
bool VVGLBufferCopier::getCopyAndResize()	{
	lock_guard<recursive_mutex>		lock(renderLock);
	return copyAndResize;
}
void VVGLBufferCopier::setCopySize(const Size & n)	{
	lock_guard<recursive_mutex>		lock(renderLock);
	copySize = n;
}
Size VVGLBufferCopier::getCopySize()	{
	lock_guard<recursive_mutex>		lock(renderLock);
	return copySize;
}
void VVGLBufferCopier::setCopySizingMode(const SizingMode & n)	{
	lock_guard<recursive_mutex>		lock(renderLock);
	copySizingMode = n;
}
SizingMode VVGLBufferCopier::getCopySizingMode()	{
	lock_guard<recursive_mutex>		lock(renderLock);
	return copySizingMode;
}


/*	========================================	*/
#pragma mark --------------------- interface methods


VVGLBufferRef VVGLBufferCopier::copyToNewBuffer(const VVGLBufferRef & n)	{
	if (n == nullptr)
		return nullptr;
	
	if (copyAndResize)
		setSize(copySize);
	else
		setSize(n->srcRect.size);
	
	//	make the buffers i'll be rendering into
	VVGLBufferRef		fbo = CreateFBO();
#if ISF_TARGET_MAC
	VVGLBufferRef		color = (copyToIOSurface) ? CreateRGBATexIOSurface(size) : CreateRGBATex(size);
#else
	VVGLBufferRef		color = CreateRGBATex(size);
#endif
	//VVGLBufferRef		depth = CreateDepthBuffer(size);
	VVGLBufferRef		depth = nullptr;
	//	create a render target using the buffers i'm rendering into
	renderTarget = RenderTarget(fbo, color, depth);
	
	context->makeCurrentIfNotCurrent();
	//context->makeCurrent();
	//context->makeCurrentIfNull();
	
	//	lock, then prep for render (this creates the ctx).
	lock_guard<recursive_mutex>		lock(renderLock);
	_renderPrep();
	//	get the GL context, proceed with rendering
	if (context == nullptr)	{
		color = nullptr;
	}
	else	{
		//	tell the buffer to draw
		Rect			bounds = Rect(0., 0., size.width, size.height);
		Rect			glSrcRect = n->glReadySrcRect();
		Rect			dstRect = ResizeRect(glSrcRect, bounds, copySizingMode);
//#if !ISF_TARGET_IOS
		//glEnable(n->desc.target);
		//n->draw(dstRect);
		//glDisable(n->desc.target);
		_drawBuffer(n, glSrcRect, dstRect);
//#else
//		_drawBuffer(n, glSrcRect, dstRect);
//#endif
		//	cleanup after render
		_renderCleanup();
	}
	//	clear out the render target
	renderTarget = RenderTarget();
	
	return color;
}
bool VVGLBufferCopier::copyFromTo(const VVGLBufferRef & a, const VVGLBufferRef & b)	{
	if (a==nullptr || b==nullptr)
		return false;
	if ((a->size != b->size && !copyAndResize) || (copyAndResize && copySize != b->size))	{
		//cout << "\tERR: bailing, size mismatch, " << __PRETTY_FUNCTION__ << endl;
		//cout << "\ta size is " << a->size << ", b size is " << b->size << ", copyAndResize is " << copyAndResize << endl;
		return false;
	}
	
	setSize(copyAndResize ? copySize : b->size);
	
	bool				returnMe = true;
	//	make the buffers i'll be rendering into
	VVGLBufferRef		fbo = CreateFBO();
	//VVGLBufferRef		depth = CreateDepthBuffer(size);
	VVGLBufferRef		depth = nullptr;
	//	create a render target using the buffers i'm rendering into
	renderTarget = RenderTarget(fbo, b, depth);
	
	context->makeCurrentIfNotCurrent();
	//context->makeCurrent();
	//context->makeCurrentIfNull();
	
	//	lock, then prep for render (this creates the ctx).
	lock_guard<recursive_mutex>		lock(renderLock);
	_renderPrep();
	//	get the GL context, proceed with rendering
	if (context == nullptr)	{
		returnMe = false;
		cout << "\tERR: failing, no ctx, " << __PRETTY_FUNCTION__ << endl;
	}
	else	{
		//	tell the buffer to draw
		Rect			dstRect(0., 0., b->size.width, b->size.height);
//#if !ISF_TARGET_IOS
		//glEnable(a->desc.target);
		//a->draw(dstRect);
		//glDisable(a->desc.target);
		_drawBuffer(a, a->glReadySrcRect(), dstRect);
//#else
//		_drawBuffer(a, a->glReadySrcRect(), dstRect);
//#endif
		//	cleanup after render
		_renderCleanup();
	}
	//	clear out the render target
	renderTarget = RenderTarget();
	
	return returnMe;
}

void VVGLBufferCopier::sizeVariantCopy(const VVGLBufferRef & a, const VVGLBufferRef & b)	{
	if (a==nullptr || b==nullptr)
		return;
	
	setSize(b->size);
	
	//	make the buffers i'll be rendering into
	VVGLBufferRef		fbo = CreateFBO();
	//VVGLBufferRef		depth = CreateDepthBuffer(size);
	VVGLBufferRef		depth = nullptr;
	//	create a render target using the buffers i'm rendering into
	renderTarget = RenderTarget(fbo, b, depth);
	
	context->makeCurrentIfNotCurrent();
	//context->makeCurrent();
	//context->makeCurrentIfNull();
	
	//	lock, then prep for render (this creates the ctx).
	lock_guard<recursive_mutex>		lock(renderLock);
	_renderPrep();
	//	get the GL context, proceed with rendering
	if (context == nullptr)	{
		cout << "\tERR: failing, no ctx, " << __PRETTY_FUNCTION__ << endl;
	}
	else	{
		//	tell the buffer to draw
		Rect			bounds = Rect(0., 0., size.width, size.height);
		Rect			glSrcRect = a->glReadySrcRect();
		Rect			dstRect = ResizeRect(glSrcRect, bounds, copySizingMode);
//#if !ISF_TARGET_IOS
		//glEnable(a->desc.target);
		//a->draw(dstRect);
		//glDisable(a->desc.target);
		_drawBuffer(a, glSrcRect, dstRect);
//#else
//		_drawBuffer(a, glSrcRect, dstRect);
//#endif
		//	cleanup after render
		_renderCleanup();
	}
	//	clear out the render target
	renderTarget = RenderTarget();
}

void VVGLBufferCopier::ignoreSizeCopy(const VVGLBufferRef & a, const VVGLBufferRef & b)	{
	if (a==nullptr || b==nullptr)
		return;
	
	setSize(b->size);
	
	//	make the buffers i'll be rendering into
	VVGLBufferRef		fbo = CreateFBO();
	//VVGLBufferRef		depth = CreateDepthBuffer(size);
	VVGLBufferRef		depth = nullptr;
	//	create a render target using the buffers i'm rendering into
	renderTarget = RenderTarget(fbo, b, depth);
	
	context->makeCurrentIfNotCurrent();
	//context->makeCurrent();
	//context->makeCurrentIfNull();
	
	//	lock, then prep for render (this creates the ctx).
	lock_guard<recursive_mutex>		lock(renderLock);
	_renderPrep();
	//	get the GL context, proceed with rendering
	if (context == nullptr)	{
		cout << "\tERR: failing, no ctx, " << __PRETTY_FUNCTION__ << endl;
	}
	else	{
		//	tell the buffer to draw
//#if !ISF_TARGET_IOS
		//glEnable(a->desc.target);
		//a->draw(a->srcRect);
		//glDisable(a->desc.target);
		_drawBuffer(a, a->glReadySrcRect(), a->srcRect);
//#else
//		_drawBuffer(a, a->glReadySrcRect(), a->srcRect);
//#endif
		//	cleanup after render
		_renderCleanup();
	}
	//	clear out the render target
	renderTarget = RenderTarget();
}


void VVGLBufferCopier::copyBlackFrameTo(const VVGLBufferRef & n)	{
	if (n == nullptr)
		return;
	
	setSize(n->size);
	
	//	make a render target, populated with buffers to render into
	VVGLBufferRef		fbo = CreateFBO();
	//VVGLBufferRef		depth = CreateDepthBuffer(size);
	VVGLBufferRef		depth = nullptr;
	RenderTarget		newTarget = RenderTarget(fbo, n, depth);
	
	renderBlackFrame(newTarget);
}

void VVGLBufferCopier::copyOpaqueBlackFrameTo(const VVGLBufferRef & n)	{
	if (n == nullptr)
		return;
	
	setSize(n->size);
	
	//	make a render target, populated with buffers to render into
	VVGLBufferRef		fbo = CreateFBO();
	//VVGLBufferRef		depth = CreateDepthBuffer(size);
	VVGLBufferRef		depth = nullptr;
	RenderTarget		newTarget = RenderTarget(fbo, n, depth);
	
	renderOpaqueBlackFrame(newTarget);
}

void VVGLBufferCopier::copyRedFrameTo(const VVGLBufferRef & n)	{
	if (n == nullptr)
		return;
	
	setSize(n->size);
	
	//	make a render target, populated with buffers to render into
	VVGLBufferRef		fbo = CreateFBO();
	//VVGLBufferRef		depth = CreateDepthBuffer(size);
	VVGLBufferRef		depth = nullptr;
	RenderTarget		newTarget = RenderTarget(fbo, n, depth);
	
	renderRedFrame(newTarget);
}

#if ISF_TARGET_MAC || ISF_TARGET_GLFW
void VVGLBufferCopier::_drawBuffer(const VVGLBufferRef & inBufferRef, const Rect & inGLSrcRect, const Rect & inDstRect)	{
	glEnable(inBufferRef->desc.target);
	GLERRLOG
	
	float			verts[] = {
		(float)MinX(inDstRect), (float)MinY(inDstRect), 0.0,
		(float)MaxX(inDstRect), (float)MinY(inDstRect), 0.0,
		(float)MaxX(inDstRect), (float)MaxY(inDstRect), 0.0,
		(float)MinX(inDstRect), (float)MaxY(inDstRect), 0.0
	};
	float			texs[] = {
		(float)MinX(inGLSrcRect), (flipped) ? (float)MaxY(inGLSrcRect) : (float)MinY(inGLSrcRect),
		(float)MaxX(inGLSrcRect), (flipped) ? (float)MaxY(inGLSrcRect) : (float)MinY(inGLSrcRect),
		(float)MaxX(inGLSrcRect), (flipped) ? (float)MinY(inGLSrcRect) : (float)MaxY(inGLSrcRect),
		(float)MinX(inGLSrcRect), (flipped) ? (float)MinY(inGLSrcRect) : (float)MaxY(inGLSrcRect)
	};
	glEnableClientState(GL_VERTEX_ARRAY);
	GLERRLOG
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	GLERRLOG
	glDisableClientState(GL_COLOR_ARRAY);
	GLERRLOG
	
	glVertexPointer(3, GL_FLOAT, 0, verts);
	GLERRLOG
	glTexCoordPointer(2, GL_FLOAT, 0, texs);
	GLERRLOG
	glBindTexture(inBufferRef->desc.target, inBufferRef->name);
	GLERRLOG
	glDrawArrays(GL_QUADS, 0, 4);
	GLERRLOG
	glBindTexture(inBufferRef->desc.target, 0);
	GLERRLOG
	
	glDisable(inBufferRef->desc.target);
	GLERRLOG
}
#elif ISF_TARGET_RPI
void VVGLBufferCopier::_drawBuffer(const VVGLBufferRef & inBufferRef, const Rect & inGLSrcRect, const Rect & inDstRect)	{
	if (inBufferRef == nullptr)
		return;
	//cout << __PRETTY_FUNCTION__ << endl;
	//cout << "\tsrc coords: " << inGLSrcRect << ", dst coords: " << inDstRect << ", buffer: " << *inBufferRef << endl;
	//	set up some basic data- rects/coords/etc
	bool			flipped = inBufferRef->flipped;
	GLfloat			geoCoords[] = {
		(GLfloat)MinX(inDstRect), (GLfloat)MinY(inDstRect),
		(GLfloat)MaxX(inDstRect), (GLfloat)MinY(inDstRect),
		(GLfloat)MinX(inDstRect), (GLfloat)MaxY(inDstRect),
		(GLfloat)MaxX(inDstRect), (GLfloat)MaxY(inDstRect)
	};
	GLfloat			texCoords[] = {
		(GLfloat)MinX(inGLSrcRect), (flipped) ? (GLfloat)MaxY(inGLSrcRect) : (GLfloat)MinY(inGLSrcRect),
		(GLfloat)MaxX(inGLSrcRect), (flipped) ? (GLfloat)MaxY(inGLSrcRect) : (GLfloat)MinY(inGLSrcRect),
		(GLfloat)MinX(inGLSrcRect), (flipped) ? (GLfloat)MinY(inGLSrcRect) : (GLfloat)MaxY(inGLSrcRect),
		(GLfloat)MaxX(inGLSrcRect), (flipped) ? (GLfloat)MinY(inGLSrcRect) : (GLfloat)MaxY(inGLSrcRect)
	};
	
	//	if i don't have a VBO containing geometry for a quad, make one now
	if (geoXYVBO == nullptr)	{
		geoXYVBO = CreateVBO(geoCoords, 8*sizeof(GLfloat), GL_DYNAMIC_DRAW);
	}
	if (geoSTVBO == nullptr)	{
		geoSTVBO = CreateVBO(texCoords, 8*sizeof(GLfloat), GL_DYNAMIC_DRAW);
	}
	
	//glClearColor(1., 0., 0., 1.);
	//glClear(GL_COLOR_BUFFER_BIT);
	
	GLint			geometryIndex;
	GLint			texCoordsIndex;
	GLint			orthoIndex;
	GLint			samplerIndex;
	//	populate the vertex VBO and attribute
	//glBindBuffer(GL_ARRAY_BUFFER, geoXYVBO->name);
	//GLERRLOG
	//glBufferData(GL_ARRAY_BUFFER, 8*(sizeof(GLfloat)), geoCoords, GL_DYNAMIC_DRAW);
	//GLERRLOG
	geometryIndex = glGetAttribLocation(program, "vertXYZ");
	GLERRLOG
	glEnableVertexAttribArray(geometryIndex);
	GLERRLOG
	//glVertexAttribPointer(geometryIndex, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glVertexAttribPointer(geometryIndex, 2, GL_FLOAT, GL_FALSE, 0, geoCoords);
	GLERRLOG
	//	populate the tex coords VBO and attribute
	//glBindBuffer(GL_ARRAY_BUFFER, geoSTVBO->name);
	//GLERRLOG
	//glBufferData(GL_ARRAY_BUFFER, 8*(sizeof(GLfloat)), texCoords, GL_DYNAMIC_DRAW);
	//GLERRLOG
	texCoordsIndex = glGetAttribLocation(program, "vertST");
	GLERRLOG
	glEnableVertexAttribArray(texCoordsIndex);
	GLERRLOG
	//glVertexAttribPointer(texCoordsIndex, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glVertexAttribPointer(texCoordsIndex, 2, GL_FLOAT, GL_FALSE, 0, texCoords);
	GLERRLOG
	//	pass the coords of the orthogonal viewport to the shader
	orthoIndex = glGetUniformLocation(program, "orthoRect");
	GLERRLOG
	glUniform4f(orthoIndex, inDstRect.origin.x, inDstRect.origin.y, inDstRect.size.width, inDstRect.size.height);
	GLERRLOG
	//	pass the texture to its uniform
	glActiveTexture(GL_TEXTURE0);
	GLERRLOG
	glBindTexture(inBufferRef->desc.target, inBufferRef->name);
	GLERRLOG
	samplerIndex = glGetUniformLocation(program, "inputImage");
	GLERRLOG
	glUniform1i(samplerIndex, 0);
	GLERRLOG
	//cout << "\tprogram is " << program << ", geometryIndex is " << geometryIndex << ", texCoordsIndex is " << texCoordsIndex << ", samplerIndex is " << samplerIndex << endl;
	//	draw
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	GLERRLOG
	//	unbind stuff
	glBindTexture(inBufferRef->desc.target, 0);
	GLERRLOG
	glDisableVertexAttribArray(geometryIndex);
	GLERRLOG
	glDisableVertexAttribArray(texCoordsIndex);
	GLERRLOG
	
	glBindTexture(inBufferRef->desc.target, 0);
	GLERRLOG
	
}
#endif




VVGLBufferCopierRef CreateGlobalBufferCopier()	{
	cout << __PRETTY_FUNCTION__ << endl;
	//	if there's already a global buffer copier, delete it
	if (_globalBufferCopier != nullptr)	{
		delete _globalBufferCopier;
		_globalBufferCopier = nullptr;
	}
	//	get the vars i need to create the buffer copier
	VVGLBufferPoolRef	bp = GetGlobalBufferPool();
	if (bp == nullptr)	{
		cout << "\tERR: no global buffer pool, can't make global buffer copier: " << __PRETTY_FUNCTION__ << endl;
		return nullptr;
	}
	//	make a shared ptr that retains a buffer copier
	VVGLBufferCopierRef	newCopier = make_shared<VVGLBufferCopier>();
	//	make the global buffer copier
	_globalBufferCopier = new shared_ptr<VVGLBufferCopier>();
	*_globalBufferCopier = newCopier;
	return *_globalBufferCopier;
}

VVGLBufferCopierRef GetGlobalBufferCopier()	{
	if (_globalBufferCopier == nullptr)	{
		cout << "\tERR: copier null, " << __PRETTY_FUNCTION__ << endl;
		return nullptr;
	}
	return *_globalBufferCopier;
}




}
