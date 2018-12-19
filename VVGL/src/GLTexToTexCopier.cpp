	#include "GLTexToTexCopier.hpp"

#include <vector>
#include <iostream>

#include "GLContext.hpp"




namespace VVGL
{


using namespace std;




/*	========================================	*/
#pragma mark --------------------- constructor/destructor


GLTexToTexCopier::GLTexToTexCopier() : GLScene()	{
	generalInit();
}
GLTexToTexCopier::GLTexToTexCopier(const GLContextRef & inCtx) : GLScene(inCtx)	{
	generalInit();
}


void GLTexToTexCopier::prepareToBeDeleted()	{
	//cout << __PRETTY_FUNCTION__ << "->" << this << endl;
	//	now call the super, which deletes the context
	GLScene::prepareToBeDeleted();
}

GLTexToTexCopier::~GLTexToTexCopier()	{
	//cout << __PRETTY_FUNCTION__ << "->" << this << endl;
	if (!_deleted)
		prepareToBeDeleted();
	
#if defined(VVGL_TARGETENV_GL3PLUS) || defined(VVGL_TARGETENV_GLES3)
	_vao = nullptr;
#elif defined(VVGL_TARGETENV_GLES)
	_vbo = nullptr;
#endif
}
void GLTexToTexCopier::generalInit()	{
	//cout << __PRETTY_FUNCTION__ << endl;
	//	set up simple frag & vert shaders that draw a tex
#if defined(VVGL_TARGETENV_GL3PLUS)
	string			vsString("\r\n\
#version 330 core\r\n\
in vec3		inXYZ;\r\n\
in vec2		inST;\r\n\
uniform mat4	vvglOrthoProj;\r\n\
out vec2		programST;\r\n\
void main()	{\r\n\
	gl_Position = vec4(inXYZ.x, inXYZ.y, inXYZ.z, 1.0) * vvglOrthoProj;\r\n\
	programST = inST;\r\n\
}\r\n\
");
	string			fsString("\r\n\
#version 330 core\r\n\
in vec2		programST;\r\n\
uniform sampler2D		inputImage;\r\n\
uniform sampler2DRect	inputImageRect;\r\n\
uniform int		isRectTex;\r\n\
out vec4		FragColor;\r\n\
void main()	{\r\n\
	if (isRectTex==0)\r\n\
		FragColor = vec4(0,0,0,1);\r\n\
	else if (isRectTex==1)\r\n\
		FragColor = texture(inputImage,programST);\r\n\
	else\r\n\
		FragColor = texture(inputImageRect,programST);\r\n\
}\r\n\
");
#elif defined(VVGL_TARGETENV_GLES)
	string			vsString("\n\
attribute vec3		inXYZ;\n\
attribute vec2		inST;\n\
uniform mat4	vvglOrthoProj;\n\
varying vec2		programST;\n\
void main()	{\n\
	gl_Position = vec4(inXYZ.x, inXYZ.y, inXYZ.z, 1.0) * vvglOrthoProj;\n\
	programST = inST;\n\
}\n\
");
	string			fsString("\n\
precision mediump	float;\n\
varying vec2		programST;\n\
uniform sampler2D		inputImage;\n\
//uniform sampler2DRect	inputImageRect;\n\
uniform int		isRectTex;\n\
void main()	{\n\
	//gl_FragColor = vec4(0,0,1,1);\n\
	if (isRectTex==0)\n\
		gl_FragColor = vec4(0,0,0,1);\n\
	else if (isRectTex==1)\n\
		gl_FragColor = texture2D(inputImage,programST);\n\
	//else\n\
		//gl_FragColor = texture2DRect(inputImageRect,programST);\n\
}\n\
");
#elif defined(VVGL_TARGETENV_GLES3)
	string			vsString("\r\n\
#version 300 es\r\n\
in vec3		inXYZ;\r\n\
in vec2		inST;\r\n\
uniform mat4	vvglOrthoProj;\r\n\
out vec2		programST;\r\n\
void main()	{\r\n\
	gl_Position = vec4(inXYZ.x, inXYZ.y, inXYZ.z, 1.0) * vvglOrthoProj;\r\n\
	programST = inST;\r\n\
}\r\n\
");
	string			fsString("\r\n\
#version 300 es\r\n\
precision mediump		float;\r\n\
in vec2		programST;\r\n\
uniform sampler2D		inputImage;\r\n\
uniform sampler2DRect	inputImageRect;\r\n\
uniform int		isRectTex;\r\n\
out vec4		FragColor;\r\n\
void main()	{\r\n\
	if (isRectTex==0)\r\n\
		FragColor = vec4(0,0,0,1);\r\n\
	else if (isRectTex==1)\r\n\
		FragColor = texture(inputImage,programST);\r\n\
	else\r\n\
		FragColor = texture(inputImageRect,programST);\r\n\
}\r\n\
");
#elif defined(VVGL_TARGETENV_GL2)
//	intentionally blank, no shaders used
#endif
	
	if (glVersion() != GLVersion_2)	{
		setVertexShaderString(vsString);
		setFragmentShaderString(fsString);
	}
	
	setRenderPrepCallback([&](const GLScene & /*n*/, const bool /*inReshaped*/, const bool & inPgmChanged) {
		if (inPgmChanged)	{
			_inputXYZLoc.cacheTheLoc(_program);
			_inputSTLoc.cacheTheLoc(_program);
			_inputImageLoc.cacheTheLoc(_program);
			_inputImageRectLoc.cacheTheLoc(_program);
			_isRectTexLoc.cacheTheLoc(_program);
			_vboContents = Quad<VertXYZST>();
		}
	});
}


/*	========================================	*/
#pragma mark --------------------- superclass overrides


void GLTexToTexCopier::_initialize()	{
	GLScene::_initialize();
}


/*	========================================	*/
#pragma mark --------------------- getter/setter


void GLTexToTexCopier::setCopyToIOSurface(const bool & n)	{
	lock_guard<recursive_mutex>		lock(_renderLock);
	_copyToIOSurface = n;
}
bool GLTexToTexCopier::copyToIOSurface()	{
	lock_guard<recursive_mutex>		lock(_renderLock);
	return _copyToIOSurface;
}
void GLTexToTexCopier::setCopyAndResize(const bool & n)	{
	lock_guard<recursive_mutex>		lock(_renderLock);
	_copyAndResize = n;
}
bool GLTexToTexCopier::copyAndResize()	{
	lock_guard<recursive_mutex>		lock(_renderLock);
	return _copyAndResize;
}
void GLTexToTexCopier::setCopySize(const Size & n)	{
	lock_guard<recursive_mutex>		lock(_renderLock);
	_copySize = n;
}
Size GLTexToTexCopier::copySize()	{
	lock_guard<recursive_mutex>		lock(_renderLock);
	return _copySize;
}
void GLTexToTexCopier::setCopySizingMode(const SizingMode & n)	{
	lock_guard<recursive_mutex>		lock(_renderLock);
	_copySizingMode = n;
}
SizingMode GLTexToTexCopier::copySizingMode()	{
	lock_guard<recursive_mutex>		lock(_renderLock);
	return _copySizingMode;
}


/*	========================================	*/
#pragma mark --------------------- interface methods


GLBufferRef GLTexToTexCopier::copyToNewBuffer(const GLBufferRef & n)	{
	if (n == nullptr)
		return nullptr;
	
	if (_copyAndResize)
		setOrthoSize(_copySize);
	else
		setOrthoSize(n->srcRect.size);
	
	//	make the buffers i'll be rendering into
	GLBufferPoolRef		bp = (_privatePool!=nullptr) ? _privatePool : GetGlobalBufferPool();
#if defined(VVGL_SDK_MAC)
	GLBufferRef		color = (_copyToIOSurface) ? CreateRGBATexIOSurface(_orthoSize, false, bp) : CreateRGBATex(_orthoSize, false, bp);
#else
	GLBufferRef		color = CreateRGBATex(_orthoSize, false, bp);
#endif
	//	create a render target using the buffers i'm rendering into
	_renderTarget = RenderTarget(CreateFBO(false, bp), color, nullptr);
	
	//	lock, then prep for render (this creates the ctx).
	lock_guard<recursive_mutex>		lock(_renderLock);
	if (_context == nullptr)	{
		cout << "\terr: bailing, ctx null, " << __PRETTY_FUNCTION__ << endl;
		return nullptr;
	}
	_context->makeCurrentIfNotCurrent();
	
	//	prep for render
	_renderPrep();
	
	//	assemble a quad object that describes what we're going to draw
	Quad<VertXYZST>			targetQuad;
	targetQuad.populateGeo(Rect(0,0,_orthoSize.width,_orthoSize.height));
	targetQuad.populateTex(n->glReadySrcRect(), n->flipped);
	
	//	draw the texture in the target quad
	_drawBuffer(n, targetQuad);
	
	//	cleanup after render
	_renderCleanup();
	
	//	clear out the render target
	_renderTarget = RenderTarget();
	
	return color;
}
bool GLTexToTexCopier::copyFromTo(const GLBufferRef & a, const GLBufferRef & b)	{
	
	if (a==nullptr || b==nullptr)
		return false;
	if ((a->srcRect.size != b->srcRect.size && !_copyAndResize) || (_copyAndResize && _copySize != b->srcRect.size))	{
		//cout << "\tERR: bailing, size mismatch, " << __PRETTY_FUNCTION__ << endl;
		//cout << "\ta size is " << a->srcRect.size << ", b size is " << b->srcRect.size << ", _copyAndResize is " << _copyAndResize << endl;
		return false;
	}
	
	GLBufferPoolRef		bp = (_privatePool!=nullptr) ? _privatePool : GetGlobalBufferPool();
	
	setOrthoSize(_copyAndResize ? _copySize : b->srcRect.size);
	
	//	create a render target using the buffers i'm rendering into
	_renderTarget = RenderTarget(CreateFBO(false, bp), b, nullptr);
	
	//	lock, then prep for render (this creates the ctx).
	lock_guard<recursive_mutex>		lock(_renderLock);
	if (_context == nullptr)	{
		cout << "\terr: bailing, ctx null, " << __PRETTY_FUNCTION__ << endl;
		return false;
	}
	_context->makeCurrentIfNotCurrent();
	//_context->makeCurrent();
	//_context->makeCurrentIfNull();
	
	//	prep for render
	_renderPrep();
	
	//	assemble a quad object that describes what we're going to draw
	Quad<VertXYZST>			targetQuad;
	targetQuad.populateGeo(Rect(0,0,a->srcRect.size.width,a->srcRect.size.height));
	targetQuad.populateTex(a->glReadySrcRect(), a->flipped);
	
	//	draw the texture in the target quad
	_drawBuffer(a, targetQuad);
	
	//	cleanup after render
	_renderCleanup();
	
	//	clear out the render target
	_renderTarget = RenderTarget();
	
	return true;
	
	
}

void GLTexToTexCopier::sizeVariantCopy(const GLBufferRef & a, const GLBufferRef & b)	{
	
	if (a==nullptr || b==nullptr)
		return;
	
	GLBufferPoolRef		bp = (_privatePool!=nullptr) ? _privatePool : GetGlobalBufferPool();
	setOrthoSize(b->srcRect.size);
	
	//	create a render target using the buffers i'm rendering into
	_renderTarget = RenderTarget(CreateFBO(false, bp), b, nullptr);
	
	//	lock, then prep for render (this creates the ctx).
	lock_guard<recursive_mutex>		lock(_renderLock);
	if (_context == nullptr)	{
		cout << "\terr: bailing, ctx null, " << __PRETTY_FUNCTION__ << endl;
		return;
	}
	_context->makeCurrentIfNotCurrent();
	//_context->makeCurrent();
	//_context->makeCurrentIfNull();
	
	//	prep for render
	_renderPrep();
	
	//	assemble a quad object that describes what we're going to draw
	Quad<VertXYZST>			targetQuad;
	Rect					geometryRect = ResizeRect(a->srcRect, Rect(0,0,_orthoSize.width,_orthoSize.height), _copySizingMode);
	targetQuad.populateGeo(geometryRect);
	targetQuad.populateTex(a->glReadySrcRect(), a->flipped);
	
	//	draw the texture in the target quad
	_drawBuffer(a, targetQuad);
	
	//	cleanup after render
	_renderCleanup();
	
	//	clear out the render target
	_renderTarget = RenderTarget();
	
}

void GLTexToTexCopier::ignoreSizeCopy(const GLBufferRef & a, const GLBufferRef & b)	{
	
	if (a==nullptr || b==nullptr)
		return;
	
	GLBufferPoolRef		bp = (_privatePool!=nullptr) ? _privatePool : GetGlobalBufferPool();
	setOrthoSize(b->size);
	
	//	create a render target using the buffers i'm rendering into
	_renderTarget = RenderTarget(CreateFBO(false, bp), b, nullptr);
	
	//	lock, then prep for render (this creates the ctx).
	lock_guard<recursive_mutex>		lock(_renderLock);
	if (_context == nullptr)	{
		cout << "\terr: bailing, ctx null, " << __PRETTY_FUNCTION__ << endl;
		return;
	}
	_context->makeCurrentIfNotCurrent();
	//_context->makeCurrent();
	//_context->makeCurrentIfNull();
	
	//	prep for render
	_renderPrep();
	
	//	assemble a quad object that describes what we're going to draw
	Quad<VertXYZST>			targetQuad;
	targetQuad.populateGeo(Rect(0,0,a->srcRect.size.width,a->srcRect.size.height));
	targetQuad.populateTex(a->glReadySrcRect(), a->flipped);
	
	//	draw the texture in the target quad
	_drawBuffer(a, targetQuad);
	
	//	cleanup after render
	_renderCleanup();
	
	//	clear out the render target
	_renderTarget = RenderTarget();
	
}


void GLTexToTexCopier::copyBlackFrameTo(const GLBufferRef & n)	{
	
	if (n == nullptr)
		return;
	
	GLBufferPoolRef		bp = (_privatePool!=nullptr) ? _privatePool : GetGlobalBufferPool();
	setOrthoSize(n->size);
	
	//	make a render target, populated with buffers to render into
	RenderTarget		newTarget = RenderTarget(CreateFBO(false, bp), n, nullptr);
	
	renderBlackFrame(newTarget);
	
}

void GLTexToTexCopier::copyOpaqueBlackFrameTo(const GLBufferRef & n)	{
	
	if (n == nullptr)
		return;
	
	GLBufferPoolRef		bp = (_privatePool!=nullptr) ? _privatePool : GetGlobalBufferPool();
	setOrthoSize(n->size);
	
	//	make a render target, populated with buffers to render into
	RenderTarget		newTarget = RenderTarget(CreateFBO(false, bp), n, nullptr);
	
	renderOpaqueBlackFrame(newTarget);
	
}

void GLTexToTexCopier::copyRedFrameTo(const GLBufferRef & n)	{
	
	if (n == nullptr)
		return;
	
	GLBufferPoolRef		bp = (_privatePool!=nullptr) ? _privatePool : GetGlobalBufferPool();
	setOrthoSize(n->size);
	
	//	make a render target, populated with buffers to render into
	RenderTarget		newTarget = RenderTarget(CreateFBO(false, bp), n, nullptr);
	
	renderRedFrame(newTarget);
	
}
void GLTexToTexCopier::_drawBuffer(const GLBufferRef & inBufferRef, const Quad<VertXYZST> & inVertexStruct)	{
	GLVersion			myVers = glVersion();
	GLBufferPoolRef		bp = (_privatePool!=nullptr) ? _privatePool : GetGlobalBufferPool();
	if (myVers==GLVersion_ES3 || myVers==GLVersion_33 || myVers==GLVersion_4)	{
#if defined(VVGL_TARGETENV_GL3PLUS) || defined(VVGL_TARGETENV_GLES3)
		//	make the VAO if we don't already have one
		if (_vao == nullptr)
			_vao = CreateVAO(true, bp);
	
		//	if the target quad doesn't match what's in the VAO now, we have to update the VAO now
		if (inVertexStruct != _vboContents)	{
			//cout << "\tvbo contents updated, repopulating\n";
			//	bind the VAO
			if (_vao != nullptr)	{
				glBindVertexArray(_vao->name);
				GLERRLOG
			}
			//	make a VBO, populate it with vertex data
			uint32_t		tmpVBO = -1;
			glGenBuffers(1, &tmpVBO);
			GLERRLOG
			glBindBuffer(GL_ARRAY_BUFFER, tmpVBO);
			GLERRLOG
			glBufferData(GL_ARRAY_BUFFER, sizeof(inVertexStruct), (void*)&inVertexStruct, GL_STATIC_DRAW);
			GLERRLOG
			//	configure the attribute pointers to work with the VBO
			if (_inputXYZLoc.loc >= 0)	{
				_inputXYZLoc.enable();
				glVertexAttribPointer(_inputXYZLoc.loc, 3, GL_FLOAT, GL_FALSE, inVertexStruct.stride(), BUFFER_OFFSET(inVertexStruct.geoOffset()));
				GLERRLOG
			}
			if (_inputSTLoc.loc >= 0)	{
				_inputSTLoc.enable();
				glVertexAttribPointer(_inputSTLoc.loc, 2, GL_FLOAT, GL_FALSE, inVertexStruct.stride(), BUFFER_OFFSET(inVertexStruct.texOffset()));
				GLERRLOG
			}
			//	un-bind the VAO, we're done assembling it
			glBindVertexArray(0);
			GLERRLOG
			//	delete the VBO
			glDeleteBuffers(1, &tmpVBO);
			GLERRLOG
		
			_vboContents = inVertexStruct;
		}
	
		//	at this point we've got a VAO and it's guaranteed to have the correct geometry + texture coords- we just have to draw it
	
		//	bind the VAO
		if (_vao != nullptr)	{
			glBindVertexArray(_vao->name);
			GLERRLOG
		}
		//	pass the 2D texture to the program (if there is a 2D texture)
		glActiveTexture(GL_TEXTURE0);
		GLERRLOG
		glBindTexture(GL_TEXTURE_2D, (inBufferRef!=nullptr && inBufferRef->desc.target==GLBuffer::Target_2D) ? inBufferRef->name : 0);
		GLERRLOG
		//glBindTexture(GLBuffer::Target_2D, 0);
		//GLERRLOG
		if (_inputImageLoc.loc >= 0)	{
			glUniform1i(_inputImageLoc.loc, 0);
			GLERRLOG
		}
#if defined(VVGL_SDK_MAC)
		//	pass the RECT texture to the program (if there is a RECT texture)
		glActiveTexture(GL_TEXTURE1);
		GLERRLOG
		//glBindTexture(GL_TEXTURE_2D, 0);
		//GLERRLOG
		glBindTexture(GLBuffer::Target_Rect, (inBufferRef!=nullptr && inBufferRef->desc.target==GLBuffer::Target_Rect) ? inBufferRef->name : 0);
		GLERRLOG
		if (_inputImageRectLoc.loc >= 0)	{
			glUniform1i(_inputImageRectLoc.loc, 1);
			GLERRLOG
		}
#endif	//	VVGL_SDK_MAC
		//	pass an int to the program that indicates whether we're passing a 2D or a RECT texture
		if (_isRectTexLoc.loc >= 0)	{
			if (inBufferRef == nullptr)	{
				glUniform1i(_isRectTexLoc.loc, 0);
				GLERRLOG
			}
			else	{
				switch (inBufferRef->desc.target)	{
				case GLBuffer::Target_2D:
					glUniform1i(_isRectTexLoc.loc, 1);
					GLERRLOG
					break;
#if defined(VVGL_SDK_MAC)
				case GLBuffer::Target_Rect:
					glUniform1i(_isRectTexLoc.loc, 2);
					GLERRLOG
					break;
#endif	//	VVGL_SDK_MAC
				default:
					glUniform1i(_isRectTexLoc.loc, 0);
					GLERRLOG
					break;
				}
			}
		}
	
		//	draw!
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		GLERRLOG
	
		//	unbind the VAO
		glBindVertexArray(0);
		GLERRLOG
#endif	//	VVGL_TARGETENV_GL3PLUS || VVGL_TARGETENV_GLES3
	}
	else if (myVers==GLVersion_ES)	{
#if defined(VVGL_TARGETENV_GLES)
		//	if there's no VBO, or the passed vertex struct doesn't match the current VBO contents...
		//if (_vbo==nullptr || inVertexStruct!=_vboContents)	{
			//	create a new VBO with the passed vertex data
		//	_vbo = CreateVBO((void*)&inVertexStruct, sizeof(inVertexStruct), GL_STATIC_DRAW, true, bp);
		
		//	_vboContents = inVertexStruct;
		//}
	
		//	at this point, we've got a VBO and it's guaranteed to have the correct geometry + texture coords- we just have to draw it
		//glClearColor(1., 0., 0., 1.);
		//GLERRLOG
		//glClear(GL_COLOR_BUFFER_BIT);
		//GLERRLOG
	
		//	bind the VBO
		//if (_vbo != nullptr)	{
		//	glBindBuffer(GL_ARRAY_BUFFER, _vbo->name);
		//	GLERRLOG
		//}
		//	configure the attribute pointers to work with the VBO
		if (_inputXYZLoc.loc >= 0)	{
			_inputXYZLoc.enable();
			glVertexAttribPointer(_inputXYZLoc.loc, 3, GL_FLOAT, GL_FALSE, inVertexStruct.stride(), &inVertexStruct.bl.geo.x);
			GLERRLOG
		}
		if (_inputSTLoc.loc >= 0)	{
			_inputSTLoc.enable();
			glVertexAttribPointer(_inputSTLoc.loc, 2, GL_FLOAT, GL_FALSE, inVertexStruct.stride(), &inVertexStruct.bl.tex.s);
			GLERRLOG
		}
		//	pass the 2D texture to the program (if there is a 2D texture)
		//glEnable(GL_TEXTURE_2D);
		//GLERRLOG
		glActiveTexture(GL_TEXTURE0);
		GLERRLOG
		glBindTexture(GL_TEXTURE_2D, (inBufferRef!=nullptr && inBufferRef->desc.target==GLBuffer::Target_2D) ? inBufferRef->name : 0);
		GLERRLOG
		//glBindTexture(GLBuffer::Target_2D, 0);
		//GLERRLOG
		if (_inputImageLoc.loc >= 0)	{
			glUniform1i(_inputImageLoc.loc, 0);
			GLERRLOG
		}
		//	pass an int to the program that indicates whether we're passing a 2D or a RECT texture
		if (_isRectTexLoc.loc >= 0)	{
			if (inBufferRef == nullptr)	{
				glUniform1i(_isRectTexLoc.loc, 0);
				GLERRLOG
			}
			else	{
				switch (inBufferRef->desc.target)	{
				case GLBuffer::Target_2D:
					glUniform1i(_isRectTexLoc.loc, 1);
					GLERRLOG
					break;
				default:
					glUniform1i(_isRectTexLoc.loc, 0);
					GLERRLOG
					break;
				}
			}
		}
	
		//	draw!
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		GLERRLOG
	
		//	disable the relevant attrib pointers & textures
		if (_inputXYZLoc.loc >= 0)	{
			_inputXYZLoc.disable();
		}
		if (_inputSTLoc.loc >= 0)	{
			_inputSTLoc.disable();
		}
		glDisable(GL_TEXTURE_2D);
		GLERRLOG
	
		//	un-bind the VBO
		//if (_vbo != nullptr)	{
		//	glBindBuffer(GL_ARRAY_BUFFER, 0);
		//	GLERRLOG
		//}
#endif
	}
	else if (myVers==GLVersion_2)	{
#if defined(VVGL_TARGETENV_GL2)
		glActiveTexture(GL_TEXTURE0);
		GLERRLOG
		glEnable(inBufferRef->desc.target);
		GLERRLOG
		/*
		float			verts[] = {
			(float)inDstRect.minX(), (float)inDstRect.minY(), 0.0,
			(float)inDstRect.maxX(), (float)inDstRect.minY(), 0.0,
			(float)inDstRect.maxX(), (float)inDstRect.maxY(), 0.0,
			(float)inDstRect.minX(), (float)inDstRect.maxY(), 0.0
		};
		float			texs[] = {
			(float)inGLSrcRect.minX(), (flipped) ? (float)inGLSrcRect.maxY() : (float)inGLSrcRect.minY(),
			(float)inGLSrcRect.maxX(), (flipped) ? (float)inGLSrcRect.maxY() : (float)inGLSrcRect.minY(),
			(float)inGLSrcRect.maxX(), (flipped) ? (float)inGLSrcRect.minY() : (float)inGLSrcRect.maxY(),
			(float)inGLSrcRect.minX(), (flipped) ? (float)inGLSrcRect.minY() : (float)inGLSrcRect.maxY()
		};
		*/
		glEnableClientState(GL_VERTEX_ARRAY);
		GLERRLOG
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		GLERRLOG
		glDisableClientState(GL_COLOR_ARRAY);
		GLERRLOG
	
		glVertexPointer(3, GL_FLOAT, (int)inVertexStruct.stride(), (float*)&inVertexStruct);
		GLERRLOG
		glTexCoordPointer(2, GL_FLOAT, (int)inVertexStruct.stride(), &inVertexStruct.bl.tex.s);
		GLERRLOG
		glBindTexture(inBufferRef->desc.target, inBufferRef->name);
		GLERRLOG
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		GLERRLOG
		glBindTexture(inBufferRef->desc.target, 0);
		GLERRLOG
		glDisable(inBufferRef->desc.target);
		GLERRLOG
	
		glDisable(inBufferRef->desc.target);
		GLERRLOG
#endif
	}
}




}
