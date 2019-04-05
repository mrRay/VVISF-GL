#include "GLBufferWindow.hpp"




using namespace std;
using namespace VVGL;




/*	========================================	*/
#pragma mark --------------------- constructors/destructors


GLBufferWindow::GLBufferWindow(const GLContextRef & inShareCtx) : WindowGLObject(inShareCtx) {
	//cout << __PRETTY_FUNCTION__ << endl;
	//	the super creates the GL ctx in its constructor
	if (_ctx == nullptr)
		cout << "ERR: _ctx NULL in " << __PRETTY_FUNCTION__ << endl;
	else {
		//_scene = CreateGLSceneRefUsing(_ctx);
	}

	_updateContext();
}

GLBufferWindow::~GLBufferWindow() {
	//stopRendering();
	_scene = nullptr;
	_vao = nullptr;
	_buffer = nullptr;
}


/*	========================================	*/
#pragma mark --------------------- public methods




void GLBufferWindow::drawBuffer(const GLBufferRef & inBuffer) {
	//cout << __PRETTY_FUNCTION__ << ", " << *inBuffer << endl;
	lock_guard<recursive_mutex> lock(_ctxLock);
	_buffer = inBuffer;
	if (_rendering)
		InvalidateRect(_wnd, 0, TRUE);

	//glDrawCallback();
	//SwapBuffers(_dc);
}
void GLBufferWindow::glDrawCallback() {
	//cout << __PRETTY_FUNCTION__ << endl;

	//if (_ctx != nullptr)
	//	cout << "\trenderer is " << _ctx->getRenderer() << endl;

	//WindowGLObject::glDrawCallback();

	lock_guard<recursive_mutex>		lock(_ctxLock);

	if (_ctx == nullptr || _scene == nullptr)
		return;

	RECT			tmpRect;
	if (!GetWindowRect(_wnd, &tmpRect))
		cout << "ERR: unable to get window dims in " << __PRETTY_FUNCTION__ << endl;
	else {

		//_ctx->makeCurrentIfNotCurrent();

		if (_renderCallback != nullptr)
			_renderCallback(*this);

		_ctx->makeCurrentIfNotCurrent();

		if (_scene != nullptr) {
			_scene->setOrthoSize(windowSize());
			_scene->render();
		}
		
		//GLBufferPoolRef		bp = GetGlobalBufferPool();
		//if (bp != nullptr)
		//	bp->housekeeping();
	}
}

void GLBufferWindow::setRenderCallback(const function<void(const GLBufferWindow & inWindow)> & inCallback) {
	lock_guard<recursive_mutex>		lock(_ctxLock);
	_renderCallback = inCallback;
}


/*	========================================	*/
#pragma mark --------------------- private methods


void GLBufferWindow::_renderNow() {

}
void GLBufferWindow::stopRenderingImmediately() {

}
void GLBufferWindow::_updateContext() {
	//cout << __PRETTY_FUNCTION__ << endl;

	//	...if we're here, the ctx changed- we need to trash the old scene, make a new scene, then supply it with a draw lambda
	_scene = nullptr;
	_vao = nullptr;

	if (_ctx == nullptr) {
		cout << "\tbailing, no ctx, " << __PRETTY_FUNCTION__ << endl;
		return;
	}

	_scene = CreateGLSceneRefUsing(_ctx);
	_scene->setPerformClear(true);
	//_scene->setClearColor(0., 0., 0., 0.);
	_scene->setClearColor(0., 0., 1., 0.);

	//cout << "configuring program" << endl;
	if (_ctx->version == GLVersion_2) {
		//_scene->setRenderCleanupCallback([](const GLScene & n) {
		//	//glFinish();
		//	//wglSwapIntervalEXT(1);
		//});
		_scene->setRenderPrepCallback([](const GLScene & /*n*/, const bool & /*inReshaped*/, const bool & /*inPgmChanged*/) {
			//cout << __PRETTY_FUNCTION__ << " render callback" << endl;

			//wglSwapIntervalEXT(1);
		});
		_scene->setRenderCallback([&](const GLScene & n) {
			//cout << __PRETTY_FUNCTION__ << " render callback" << endl;
			//double		ltbbm = devicePixelRatio();
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
			/*
			//	set up the view to draw
			//NSRect				bounds = [(id)selfPtr backingBounds];
			VVGL::Rect				bounds(0., 0., width()*ltbbm, height()*ltbbm);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glViewport(0, 0, (GLsizei) bounds.size.width, (GLsizei) bounds.size.height);
			//if (flipped)
				glOrtho(bounds.origin.x, bounds.origin.x+bounds.size.width, bounds.origin.y, bounds.origin.y+bounds.size.height, 1.0, -1.0);
			//else
			//	glOrtho(bounds.origin.x, bounds.origin.x+bounds.size.width, bounds.origin.y+bounds.size.height, bounds.origin.y, 1.0, -1.0);
			*/
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
			GLBufferRef		bufferToDraw = _getBuffer();
			if (bufferToDraw != nullptr) {
				//	make a quad struct that describes XYST geometry, populate it with the coords of the quad we want to draw and the coords of the texture we want to draw on it
				//NSRect				rawBounds = [(id)selfPtr backingBounds];
				VVGL::Rect			imgBounds = bufferToDraw->srcRect;
				VVGL::Rect			viewBoundsRect = VVGL::Rect(0., 0., 0., 0.);
				viewBoundsRect.size = n.orthoSize();
				VVGL::Rect			geometryRect = ResizeRect(imgBounds, viewBoundsRect, SizingMode_Fit);
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
				//wglActiveTexture(GL_TEXTURE0);
				glEnable(bufferToDraw->desc.target);
				glBindTexture(bufferToDraw->desc.target, bufferToDraw->name);
				//glDrawArrays(GL_QUADS, 0, 4);
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
				glBindTexture(bufferToDraw->desc.target, 0);
				glDisable(bufferToDraw->desc.target);
			}
		});
	}
	else {
#if defined(VVGL_TARGETENV_GL3PLUS)
		//	load the frag/vert shaders
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
		_scene->setVertexShaderString(vsString);
		_scene->setFragmentShaderString(fsString);
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
		_scene->setRenderPrepCallback([=](const GLScene & n, const bool & /*inReshaped*/, const bool & inPgmChanged) {
			//cout << __PRETTY_FUNCTION__ << " render prep callback" << endl;
			if (inPgmChanged) {
				//	cache all the locations for the vertex attributes & uniform locations
				GLint				myProgram = n.program();
				xyzAttr->cacheTheLoc(myProgram);
				stAttr->cacheTheLoc(myProgram);
				inputImageUni->cacheTheLoc(myProgram);
				inputImageRectUni->cacheTheLoc(myProgram);
				isRectTexUni->cacheTheLoc(myProgram);
			}
			//wglSwapIntervalEXT(1);
		});
		//	the render callback passes all the data to the GL program
		_scene->setRenderCallback([=](const GLScene & n) {
			//cout << __PRETTY_FUNCTION__ << " render callback" << endl;

			//	clear
			glClearColor(0., 0., 0., 1.);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			//	get the buffer we want to draw
			GLBufferRef		bufferToDraw = _getBuffer();
			if (bufferToDraw == nullptr)
				return;
			//cout << "\tbufferToDraw is " << bufferToDraw->getDescriptionString() << endl;
			//	try to get the VAO.  if the VAO's null, create it and store it in the GLBufferQWindow as an ivar. 
			if (_vao == nullptr) {
				GLBufferPoolRef		bp = (bufferToDraw == nullptr) ? nullptr : bufferToDraw->parentBufferPool;
				if (bp != nullptr) {
					_vao = CreateVAO(true, bp);
				}
				else
					cout << "\terr: bufferpool nil, bailing " << __PRETTY_FUNCTION__ << endl;
			}
			//	if there's still no VAO, something's wrong- bail
			if (_vao == nullptr) {
				cout << "\terr: null VAO, bailing " << __PRETTY_FUNCTION__ << endl;
				return;
			}

			//	make a quad struct that describes XYST geometry, populate it with the coords of the quad we want to draw and the coords of the texture we want to draw on it
			VVGL::Rect			imgBounds = bufferToDraw->srcRect;
			VVGL::Rect			viewBoundsRect = VVGL::Rect(0., 0., 0., 0.);
			viewBoundsRect.size = n.orthoSize();
			VVGL::Rect			geometryRect = ResizeRect(imgBounds, viewBoundsRect, SizingMode_Fit);
			Quad<VertXYST>		targetQuad;
			targetQuad.populateGeo(geometryRect);
			targetQuad.populateTex((bufferToDraw == nullptr) ? geometryRect : bufferToDraw->glReadySrcRect(), (bufferToDraw == nullptr) ? false : bufferToDraw->flipped);

			//	pass the 2D texture to the program (if there's a 2D texture)
			glActiveTexture(GL_TEXTURE0);
			GLERRLOG
				glBindTexture(GLBuffer::Target_2D, (bufferToDraw != nullptr && bufferToDraw->desc.target == GLBuffer::Target_2D) ? bufferToDraw->name : 0);
			GLERRLOG
				glBindTexture(GLBuffer::Target_Rect, 0);
			GLERRLOG
				if (inputImageUni->loc >= 0) {
					glUniform1i(inputImageUni->loc, 0);
					GLERRLOG
				}
			//	pass the RECT texture to the program (if there's a RECT texture)
			glActiveTexture(GL_TEXTURE1);
			GLERRLOG
				glBindTexture(GLBuffer::Target_2D, 0);
			GLERRLOG
				glBindTexture(GLBuffer::Target_Rect, (bufferToDraw != nullptr && bufferToDraw->desc.target == GLBuffer::Target_Rect) ? bufferToDraw->name : 0);
			GLERRLOG
				if (inputImageRectUni->loc >= 0) {
					glUniform1i(inputImageRectUni->loc, 1);
					GLERRLOG
				}
			//	pass an int to the program that indicates whether we're passing no texture (0), a 2D texture (1) or a RECT texture (2)
			if (isRectTexUni->loc >= 0) {
				if (bufferToDraw == nullptr)
					glUniform1i(isRectTexUni->loc, 0);
				else {
					switch (bufferToDraw->desc.target) {
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
			glBindVertexArray(_vao->name);
			GLERRLOG

				uint32_t			vbo = 0;
			if (_lastVBOCoords != targetQuad) {
				//	make a new VBO to contain vertex + texture coord data
				glGenBuffers(1, &vbo);
				GLERRLOG
					glBindBuffer(GL_ARRAY_BUFFER, vbo);
				GLERRLOG
					glBufferData(GL_ARRAY_BUFFER, sizeof(targetQuad), (void*)&targetQuad, GL_STATIC_DRAW);
				GLERRLOG
					//	configure the attribute pointers to use the VBO
					if (xyzAttr->loc >= 0) {
						glVertexAttribPointer(xyzAttr->loc, 2, GL_FLOAT, GL_FALSE, targetQuad.stride(), BUFFER_OFFSET(targetQuad.geoOffset()));
						GLERRLOG
							xyzAttr->enable();
					}
				if (stAttr->loc >= 0) {
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

				if (_lastVBOCoords != targetQuad) {
					//	delete the VBO we made earlier...
					glDeleteBuffers(1, &vbo);
					GLERRLOG
						//	update the vbo coords ivar (we don't want to update the VBO contents every pass)
						_lastVBOCoords = targetQuad;
				}

		});
#endif	//	VVGL_TARGETENV_GL3PLUS
	}
}
