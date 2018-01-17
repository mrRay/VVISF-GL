	#include "VVGLBufferCopier.hpp"

#include <vector>
#include <iostream>

#include "VVGLContext.hpp"




namespace VVGL
{


using namespace std;




static VVGLBufferCopierRef * _globalBufferCopier = nullptr;




/*	========================================	*/
#pragma mark --------------------- global instance factory


VVGLBufferCopierRef CreateGlobalBufferCopier()	{
	//cout << __PRETTY_FUNCTION__ << endl;
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
VVGLBufferCopierRef CreateGlobalBufferCopier(const VVGLContextRef & inCtx)	{
	//	if there's already a global buffer copier, delete it
	if (_globalBufferCopier != nullptr)	{
		delete _globalBufferCopier;
		_globalBufferCopier = nullptr;
	}
	//	make a shared ptr that retains a buffer copier
	VVGLBufferCopierRef	newCopier = make_shared<VVGLBufferCopier>(inCtx);
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


/*	========================================	*/
#pragma mark --------------------- constructor/destructor


VVGLBufferCopier::VVGLBufferCopier() : VVGLScene()	{
	generalInit();
}
VVGLBufferCopier::VVGLBufferCopier(const VVGLContextRef & inCtx) : VVGLScene(inCtx)	{
	generalInit();
}


void VVGLBufferCopier::prepareToBeDeleted()	{
	//cout << __PRETTY_FUNCTION__ << "->" << this << endl;
	//	now call the super, which deletes the context
	VVGLScene::prepareToBeDeleted();
}

VVGLBufferCopier::~VVGLBufferCopier()	{
	//cout << __PRETTY_FUNCTION__ << "->" << this << endl;
	if (!deleted)
		prepareToBeDeleted();
	
#if ISF_TARGET_GL3PLUS || ISF_TARGET_GLES3
	vao = nullptr;
#elif ISF_TARGET_GLES
	vbo = nullptr;
#endif
}
void VVGLBufferCopier::generalInit()	{
	//cout << __PRETTY_FUNCTION__ << endl;
	//	set up simple frag & vert shaders that draw a tex
#if ISF_TARGET_GL3PLUS
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
#elif ISF_TARGET_GLES
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
#elif ISF_TARGET_GLES3
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
#endif
	
	if (getGLVersion() != GLVersion_2)	{
		setVertexShaderString(vsString);
		setFragmentShaderString(fsString);
	}
	
	setRenderPrepCallback([&](const VVGLScene & n, const bool inReshaped, const bool & inPgmChanged) {
		if (inPgmChanged)	{
			inputXYZLoc.cacheTheLoc(program);
			inputSTLoc.cacheTheLoc(program);
			inputImageLoc.cacheTheLoc(program);
			inputImageRectLoc.cacheTheLoc(program);
			isRectTexLoc.cacheTheLoc(program);
			vboContents = Quad<VertXYZST>();
		}
	});
}


/*	========================================	*/
#pragma mark --------------------- superclass overrides


void VVGLBufferCopier::_initialize()	{
	VVGLScene::_initialize();
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
		setOrthoSize(copySize);
	else
		setOrthoSize(n->srcRect.size);
	
	//	make the buffers i'll be rendering into
#if ISF_TARGET_MAC
	VVGLBufferRef		color = (copyToIOSurface) ? CreateRGBATexIOSurface(orthoSize) : CreateRGBATex(orthoSize);
#else
	VVGLBufferRef		color = CreateRGBATex(orthoSize);
#endif
	//	create a render target using the buffers i'm rendering into
	renderTarget = RenderTarget(CreateFBO(), color, nullptr);
	
	//	lock, then prep for render (this creates the ctx).
	lock_guard<recursive_mutex>		lock(renderLock);
	if (context == nullptr)	{
		cout << "\terr: bailing, ctx null, " << __PRETTY_FUNCTION__ << endl;
		return nullptr;
	}
	context->makeCurrentIfNotCurrent();
	
	//	prep for render
	_renderPrep();
	
	//	assemble a quad object that describes what we're going to draw
	Quad<VertXYZST>			targetQuad;
	targetQuad.populateGeo(Rect(0,0,orthoSize.width,orthoSize.height));
	targetQuad.populateTex(n->glReadySrcRect(), n->flipped);
	
	//	draw the texture in the target quad
	_drawBuffer(n, targetQuad);
	
	//	cleanup after render
	_renderCleanup();
	
	//	clear out the render target
	renderTarget = RenderTarget();
	
	return color;
}
bool VVGLBufferCopier::copyFromTo(const VVGLBufferRef & a, const VVGLBufferRef & b)	{
	
	if (a==nullptr || b==nullptr)
		return false;
	if ((a->srcRect.size != b->srcRect.size && !copyAndResize) || (copyAndResize && copySize != b->srcRect.size))	{
		//cout << "\tERR: bailing, size mismatch, " << __PRETTY_FUNCTION__ << endl;
		//cout << "\ta size is " << a->srcRect.size << ", b size is " << b->srcRect.size << ", copyAndResize is " << copyAndResize << endl;
		return false;
	}
	
	setOrthoSize(copyAndResize ? copySize : b->srcRect.size);
	
	bool				returnMe = true;
	//	create a render target using the buffers i'm rendering into
	renderTarget = RenderTarget(CreateFBO(), b, nullptr);
	
	//	lock, then prep for render (this creates the ctx).
	lock_guard<recursive_mutex>		lock(renderLock);
	if (context == nullptr)	{
		cout << "\terr: bailing, ctx null, " << __PRETTY_FUNCTION__ << endl;
		return false;
	}
	context->makeCurrentIfNotCurrent();
	//context->makeCurrent();
	//context->makeCurrentIfNull();
	
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
	renderTarget = RenderTarget();
	
	return returnMe;
	
	
}

void VVGLBufferCopier::sizeVariantCopy(const VVGLBufferRef & a, const VVGLBufferRef & b)	{
	
	if (a==nullptr || b==nullptr)
		return;
	
	setOrthoSize(b->srcRect.size);
	
	//	create a render target using the buffers i'm rendering into
	renderTarget = RenderTarget(CreateFBO(), b, nullptr);
	
	//	lock, then prep for render (this creates the ctx).
	lock_guard<recursive_mutex>		lock(renderLock);
	if (context == nullptr)	{
		cout << "\terr: bailing, ctx null, " << __PRETTY_FUNCTION__ << endl;
		return;
	}
	context->makeCurrentIfNotCurrent();
	//context->makeCurrent();
	//context->makeCurrentIfNull();
	
	//	prep for render
	_renderPrep();
	
	//	assemble a quad object that describes what we're going to draw
	Quad<VertXYZST>			targetQuad;
	Rect					geometryRect = ResizeRect(a->srcRect, Rect(0,0,orthoSize.width,orthoSize.height), copySizingMode);
	targetQuad.populateGeo(geometryRect);
	targetQuad.populateTex(a->glReadySrcRect(), a->flipped);
	
	//	draw the texture in the target quad
	_drawBuffer(a, targetQuad);
	
	//	cleanup after render
	_renderCleanup();
	
	//	clear out the render target
	renderTarget = RenderTarget();
	
}

void VVGLBufferCopier::ignoreSizeCopy(const VVGLBufferRef & a, const VVGLBufferRef & b)	{
	
	if (a==nullptr || b==nullptr)
		return;
	
	setOrthoSize(b->size);
	
	//	create a render target using the buffers i'm rendering into
	renderTarget = RenderTarget(CreateFBO(), b, nullptr);
	
	//	lock, then prep for render (this creates the ctx).
	lock_guard<recursive_mutex>		lock(renderLock);
	if (context == nullptr)	{
		cout << "\terr: bailing, ctx null, " << __PRETTY_FUNCTION__ << endl;
		return;
	}
	context->makeCurrentIfNotCurrent();
	//context->makeCurrent();
	//context->makeCurrentIfNull();
	
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
	renderTarget = RenderTarget();
	
}


void VVGLBufferCopier::copyBlackFrameTo(const VVGLBufferRef & n)	{
	
	if (n == nullptr)
		return;
	
	setOrthoSize(n->size);
	
	//	make a render target, populated with buffers to render into
	RenderTarget		newTarget = RenderTarget(CreateFBO(), n, nullptr);
	
	renderBlackFrame(newTarget);
	
}

void VVGLBufferCopier::copyOpaqueBlackFrameTo(const VVGLBufferRef & n)	{
	
	if (n == nullptr)
		return;
	
	setOrthoSize(n->size);
	
	//	make a render target, populated with buffers to render into
	RenderTarget		newTarget = RenderTarget(CreateFBO(), n, nullptr);
	
	renderOpaqueBlackFrame(newTarget);
	
}

void VVGLBufferCopier::copyRedFrameTo(const VVGLBufferRef & n)	{
	
	if (n == nullptr)
		return;
	
	setOrthoSize(n->size);
	
	//	make a render target, populated with buffers to render into
	RenderTarget		newTarget = RenderTarget(CreateFBO(), n, nullptr);
	
	renderRedFrame(newTarget);
	
}
/*
void VVGLBufferCopier::_drawBuffer(const VVGLBufferRef & inBufferRef, const Rect & inGLSrcRect, const Rect & inDstRect)	{
	cout << __PRETTY_FUNCTION__ << "- DISABLED!" << endl;
}
*/
#if ISF_TARGET_GL3PLUS || ISF_TARGET_GLES3
void VVGLBufferCopier::_drawBuffer(const VVGLBufferRef & inBufferRef, const Quad<VertXYZST> & inVertexStruct)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	
	//	if it's GL 2.x
	if (getGLVersion() == GLVersion_2)	{
#if !ISF_TARGET_GLES && !ISF_TARGET_GLES3
		glActiveTexture(GL_TEXTURE0);
		GLERRLOG
		glEnable(inBufferRef->desc.target);
		GLERRLOG
		/*
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
	//	else it's not GL 2- it's GL3+ or GLES3+
	else	{
		//	make the VAO if we don't already have one
		if (vao == nullptr)
			vao = CreateVAO(true);
	
		//	if the target quad doesn't match what's in the VAO now, we have to update the VAO now
		if (inVertexStruct != vboContents)	{
			//cout << "\tvbo contents updated, repopulating\n";
			//	bind the VAO
			if (vao != nullptr)	{
				glBindVertexArray(vao->name);
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
			if (inputXYZLoc.loc >= 0)	{
				inputXYZLoc.enable();
				glVertexAttribPointer(inputXYZLoc.loc, 3, GL_FLOAT, GL_FALSE, inVertexStruct.stride(), BUFFER_OFFSET(inVertexStruct.geoOffset()));
				GLERRLOG
			}
			if (inputSTLoc.loc >= 0)	{
				inputSTLoc.enable();
				glVertexAttribPointer(inputSTLoc.loc, 2, GL_FLOAT, GL_FALSE, inVertexStruct.stride(), BUFFER_OFFSET(inVertexStruct.texOffset()));
				GLERRLOG
			}
			//	un-bind the VAO, we're done assembling it
			glBindVertexArray(0);
			GLERRLOG
			//	delete the VBO
			glDeleteBuffers(1, &tmpVBO);
			GLERRLOG
		
			vboContents = inVertexStruct;
		}
	
		//	at this point we've got a VAO and it's guaranteed to have the correct geometry + texture coords- we just have to draw it
	
		//	bind the VAO
		if (vao != nullptr)	{
			glBindVertexArray(vao->name);
			GLERRLOG
		}
		//	pass the 2D texture to the program (if there is a 2D texture)
		glActiveTexture(GL_TEXTURE0);
		GLERRLOG
		glBindTexture(GL_TEXTURE_2D, (inBufferRef!=nullptr && inBufferRef->desc.target==VVGLBuffer::Target_2D) ? inBufferRef->name : 0);
		GLERRLOG
		//glBindTexture(VVGLBuffer::Target_2D, 0);
		//GLERRLOG
		if (inputImageLoc.loc >= 0)	{
			glUniform1i(inputImageLoc.loc, 0);
			GLERRLOG
		}
#if ISF_TARGET_MAC
		//	pass the RECT texture to the program (if there is a RECT texture)
		glActiveTexture(GL_TEXTURE1);
		GLERRLOG
		//glBindTexture(GL_TEXTURE_2D, 0);
		//GLERRLOG
		glBindTexture(VVGLBuffer::Target_Rect, (inBufferRef!=nullptr && inBufferRef->desc.target==VVGLBuffer::Target_Rect) ? inBufferRef->name : 0);
		GLERRLOG
		if (inputImageRectLoc.loc >= 0)	{
			glUniform1i(inputImageRectLoc.loc, 1);
			GLERRLOG
		}
#endif
		//	pass an int to the program that indicates whether we're passing a 2D or a RECT texture
		if (isRectTexLoc.loc >= 0)	{
			if (inBufferRef == nullptr)	{
				glUniform1i(isRectTexLoc.loc, 0);
				GLERRLOG
			}
			else	{
				switch (inBufferRef->desc.target)	{
				case VVGLBuffer::Target_2D:
					glUniform1i(isRectTexLoc.loc, 1);
					GLERRLOG
					break;
#if ISF_TARGET_MAC
				case VVGLBuffer::Target_Rect:
					glUniform1i(isRectTexLoc.loc, 2);
					GLERRLOG
					break;
#endif
				default:
					glUniform1i(isRectTexLoc.loc, 0);
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
	}
	
}
#elif ISF_TARGET_GLES
void VVGLBufferCopier::_drawBuffer(const VVGLBufferRef & inBufferRef, const Quad<VertXYZST> & inVertexStruct)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	
	//	if there's no VBO, or the passed vertex struct doesn't match the current VBO contents...
	//if (vbo==nullptr || inVertexStruct!=vboContents)	{
		//	create a new VBO with the passed vertex data
	//	vbo = CreateVBO((void*)&inVertexStruct, sizeof(inVertexStruct), GL_STATIC_DRAW, true);
		
	//	vboContents = inVertexStruct;
	//}
	
	//	at this point, we've got a VBO and it's guaranteed to have the correct geometry + texture coords- we just have to draw it
	//glClearColor(1., 0., 0., 1.);
	//glClear(GL_COLOR_BUFFER_BIT);
	
	//	bind the VBO
	//if (vbo != nullptr)	{
	//	glBindBuffer(GL_ARRAY_BUFFER, vbo->name);
	//	GLERRLOG
	//}
	//	configure the attribute pointers to work with the VBO
	if (inputXYZLoc.loc >= 0)	{
		inputXYZLoc.enable();
		glVertexAttribPointer(inputXYZLoc.loc, 3, GL_FLOAT, GL_FALSE, inVertexStruct.stride(), &inVertexStruct.bl.geo.x);
		GLERRLOG
	}
	if (inputSTLoc.loc >= 0)	{
		inputSTLoc.enable();
		glVertexAttribPointer(inputSTLoc.loc, 2, GL_FLOAT, GL_FALSE, inVertexStruct.stride(), &inVertexStruct.bl.tex.s);
		GLERRLOG
	}
	//	pass the 2D texture to the program (if there is a 2D texture)
	//glEnable(GL_TEXTURE_2D);
	//GLERRLOG
	glActiveTexture(GL_TEXTURE0);
	GLERRLOG
	glBindTexture(GL_TEXTURE_2D, (inBufferRef!=nullptr && inBufferRef->desc.target==VVGLBuffer::Target_2D) ? inBufferRef->name : 0);
	GLERRLOG
	//glBindTexture(VVGLBuffer::Target_2D, 0);
	//GLERRLOG
	if (inputImageLoc.loc >= 0)	{
		glUniform1i(inputImageLoc.loc, 0);
		GLERRLOG
	}
	//	pass an int to the program that indicates whether we're passing a 2D or a RECT texture
	if (isRectTexLoc.loc >= 0)	{
		if (inBufferRef == nullptr)	{
			glUniform1i(isRectTexLoc.loc, 0);
			GLERRLOG
		}
		else	{
			switch (inBufferRef->desc.target)	{
			case VVGLBuffer::Target_2D:
				glUniform1i(isRectTexLoc.loc, 1);
				GLERRLOG
				break;
			default:
				glUniform1i(isRectTexLoc.loc, 0);
				GLERRLOG
				break;
			}
		}
	}
	
	//	draw!
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	GLERRLOG
	
	//	disable the relevant attrib pointers & textures
	if (inputXYZLoc.loc >= 0)	{
		inputXYZLoc.disable();
	}
	if (inputSTLoc.loc >= 0)	{
		inputSTLoc.disable();
	}
	glDisable(GL_TEXTURE_2D);
	
	//	un-bind the VBO
	//if (vbo != nullptr)	{
	//	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//	GLERRLOG
	//}
}
#endif




}
