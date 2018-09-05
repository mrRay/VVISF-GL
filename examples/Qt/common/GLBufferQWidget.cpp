#include "GLBufferQWidget.h"

//#include "VVGL.hpp"
//#include <QOpenGLContext>
//#include <QSurface>
#include <QDateTime>
#include <QDebug>




using namespace std;
using namespace VVGL;




GLBufferQWidget::GLBufferQWidget(QWidget * inParent) :
	QOpenGLWidget(inParent)
{
	//cout << __PRETTY_FUNCTION__ << endl;
	connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(aboutToQuit()));
}
GLBufferQWidget::~GLBufferQWidget()
{
	lock_guard<recursive_mutex>		lock(ctxLock);
	ctx = nullptr;
	buffer = nullptr;
}


void GLBufferQWidget::drawBuffer(const GLBufferRef & inBuffer)
{
	lock_guard<recursive_mutex>		lock(ctxLock);
	buffer = inBuffer;
	update();
}
GLBufferRef GLBufferQWidget::getBuffer()
{
	lock_guard<recursive_mutex>		lock(ctxLock);
	GLBufferRef		returnMe = buffer;
	return returnMe;
}




void GLBufferQWidget::_renderNow()	{
	//cout << __PRETTY_FUNCTION__ << endl;
	
	GLBufferPoolRef		bp = GetGlobalBufferPool();
	if (bp == nullptr)
		return;
	bool			renderAnotherFrame = false;
	{
		lock_guard<recursive_mutex>		lock(ctxLock);
		//cout<<"\tctxThread is "<<ctxThread<<", current thread is "<<QThread::currentThread()<<", main thread is "<< QCoreApplication::instance()->thread()<<endl;
		//if (ctxThread!=nullptr)	{
			if (ctxThread!=nullptr && ctxThread!=QThread::currentThread())	{
				cout << "err: ctxThread isnt currentThread, bailing, " << __PRETTY_FUNCTION__ << endl;
				return;
			}
			
			if (ctxThread != nullptr)
				renderAnotherFrame = true;
			
			
			if (scene != nullptr)	{
				scene->render();
			}
		//}
	}
	
	if (renderAnotherFrame)	{
		update();
	}

	bp->housekeeping();
	
}


/*	========================================	*/
#pragma mark --------------------- public methods


void GLBufferQWidget::startRendering()	{
	//cout << __PRETTY_FUNCTION__ << endl;
	
	bool					callDirectly = false;
	{
		lock_guard<recursive_mutex>		lock(ctxLock);
		//	if there's already a context thread then i'm already rendering and i can return
		if (ctxThread != nullptr)	{
			cout << "\terr: already a ctx thread, bailing" << endl;
			return;
		}
	
		//	...if i'm here there's no context thread, i'm not rendering, and i need to start
	
		if (QThread::currentThread() == this->thread() || this->thread() == nullptr)
			callDirectly = true;
	}
	
	//	if the current thread is the thread i'm bound to, i can call the slot directly
	if (callDirectly)	{
		//cout << "\tcalling directly..." << endl;
		startRenderingSlot();
	}
	//	else i'm on the "wrong thread", i need to use the meta object to send a signal to the slot
	else	{
		//cout << "\tstarting rendering from wrong thread, using meta to invoke slot..." << endl;
		QMetaObject::invokeMethod(this, "startRenderingSlot");
	}
	//cout << "\tFINISHED- " << __PRETTY_FUNCTION__ << endl;
}
void GLBufferQWidget::stopRendering()	{
	//cout << __PRETTY_FUNCTION__ << endl;
	
	bool					callDirectly = false;
	{
		lock_guard<recursive_mutex>		lock(ctxLock);
		//	if there isn't a context thread then i've already stopped rendering and should return
		if (ctxThread == nullptr)
			return;
	
		//	...if i'm here then there's a context thread, and i need to stop it
		
		if (QThread::currentThread() == this->thread() || this->thread() == nullptr)
			callDirectly = true;
	}
	
	//	if the current thread is the thread i'm bound to, i can call the slot directly
	if (callDirectly)	{
		stopRenderingSlot();
	}
	//	else i'm on the "wrong thread", i need to use the meta object to send a signal to the slot
	else	{
		QMetaObject::invokeMethod(this, "stopRenderingSlot");
	}
	
}
void GLBufferQWidget::stopRenderingImmediately()	{
	//cout << __PRETTY_FUNCTION__ << endl;
	{
		lock_guard<recursive_mutex>		lock(ctxLock);
		if (ctxThread == nullptr)
			return;
	}
	QMetaObject::invokeMethod(this, "stopRenderingSlot", Qt::BlockingQueuedConnection);
}


QThread * GLBufferQWidget::getRenderThread()	{
	lock_guard<recursive_mutex>		lock(ctxLock);
	return ctxThread;
}




/*	========================================	*/
#pragma mark --------------------- public slots




void GLBufferQWidget::startRenderingSlot()
{
	//cout << __PRETTY_FUNCTION__ << " startRenderingSlot" << endl;
	//cout<<"\tcurrent thread is "<<QThread::currentThread()<<", main thread is "<<QCoreApplication::instance()->thread() << endl;
	lock_guard<recursive_mutex>		lock(ctxLock);
	
	if (ctxThread != nullptr)
		return;
	
	//ctxThread = new QThread;
	//ctxThread->start();
	ctxThread = QCoreApplication::instance()->thread();
	//qDebug()<<"\tctxThread is now "<<ctxThread;
	//this->moveToThread(ctxThread);
	//if (ctx != nullptr)
	//	ctx->moveToThread(ctxThread);
	//connect(ctxThread, SIGNAL(started()), this, SLOT(requestUpdate()));
	//connect(ctxThread, &QThread::started, this, &GLBufferQWidget::requestUpdate);
	//ctxThread->start();
	update();
	//cout << "\tFINISHED- " << __PRETTY_FUNCTION__ << endl;
}
void GLBufferQWidget::stopRenderingSlot()
{
	//cout << __PRETTY_FUNCTION__ << " stopRenderingSlot" << endl;
	//cout<<"\tcurrent thread is "<<QThread::currentThread()<<", main thread is "<<QCoreApplication::instance()->thread()<<endl;
	lock_guard<recursive_mutex>		lock(ctxLock);
	
	if (ctxThread == nullptr)
		return;
	
	//	get rid of the scene now (it has to delete its program and other related vars, which must be done on this thread because this is the context's thread right now)
	vao = nullptr;
	buffer = nullptr;
	scene = nullptr;
	//	now move everything back to the main thread
	//QThread		*mainThread = QCoreApplication::instance()->thread();
	//if (mainThread != nullptr)	{
	//	this->moveToThread(mainThread);
	//	if (ctx != nullptr)
	//		ctx->moveToThread(mainThread);
	//}
	//	shut down the thread we were using to render
	//ctxThread->quit();
	//ctxThread->deleteLater();
	ctxThread = nullptr;
	//qDebug()<<"\tctxThread is now "<<ctxThread;
}
void GLBufferQWidget::aboutToQuit()	{
	//cout << __PRETTY_FUNCTION__ << endl;
	stopRendering();
}


/*	========================================	*/
#pragma mark --------------------- superclass overrides


void GLBufferQWidget::paintGL()
{
	//cout << __PRETTY_FUNCTION__ << endl;
	//qDebug() << "\t" << QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
	_renderNow();
}
void GLBufferQWidget::initializeGL()
{
	//cout << __PRETTY_FUNCTION__ << endl;
	
	makeCurrent();
	
	//	make a GLContextRef that wraps the widget's context.  this doesn't create a new context, it just wraps an existing context.
	lock_guard<recursive_mutex>		lock(ctxLock);
	ctx = make_shared<GLContext>(context());
	
	if (ctx != nullptr)	{
		//ctx->makeCurrent();
		
		//	make a scene that uses the context
		scene = CreateGLSceneRefUsing(ctx);
		
		scene->setPerformClear(true);
		scene->setClearColor(0., 0., 0., 0.);
		
		if (scene != nullptr)	{
			if (ctx->version==GLVersion_2)	{
				
				scene->setRenderPrepCallback([](const GLScene & /*n*/, const bool & /*inReshaped*/, const bool & /*inPgmChanged*/){
					//cout << __PRETTY_FUNCTION__ << " render callback" << endl;
				});
				scene->setRenderCallback([&](const GLScene & n){
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
					if (bufferToDraw != nullptr)	{
						//	make a quad struct that describes XYST geometry, populate it with the coords of the quad we want to draw and the coords of the texture we want to draw on it
						//NSRect				rawBounds = [(id)selfPtr backingBounds];
						VVGL::Rect			imgBounds = bufferToDraw->srcRect;
						VVGL::Rect			viewBoundsRect = VVGL::Rect(0., 0., 0., 0.);
						viewBoundsRect.size = n.getOrthoSize();
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
		#if defined(VVGL_TARGETENV_GL3PLUS)
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
				scene->setRenderPrepCallback([=](const GLScene & n, const bool & /*inReshaped*/, const bool & inPgmChanged)	{
					//cout << __PRETTY_FUNCTION__ << " render prep callback" << endl;
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
				scene->setRenderCallback([=](const GLScene & n)	{
					//cout << __PRETTY_FUNCTION__ << " render callback" << endl;
					//	clear
					glClearColor(0., 0., 0., 1.);
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
					//	get the buffer we want to draw
					GLBufferRef		bufferToDraw = _getBuffer();
					if (bufferToDraw == nullptr)
						return;
					//cout << "\tbufferToDraw is " << bufferToDraw->getDescriptionString() << endl;
					//	try to get the VAO.  if the VAO's null, create it and store it in the GLBufferQWidget as an ivar. 
					if (vao == nullptr)	{
						GLBufferPoolRef		bp = (bufferToDraw==nullptr) ? nullptr : bufferToDraw->parentBufferPool;
						if (bp != nullptr)	{
							vao = CreateVAO(true, bp);
						}
						else
							cout << "\terr: bufferpool nil, bailing " << __PRETTY_FUNCTION__ << endl;
					}
					//	if there's still no VAO, something's wrong- bail
					if (vao == nullptr)	{
						cout << "\terr: null VAO, bailing " << __PRETTY_FUNCTION__ << endl;
						return;
					}
		
					//	make a quad struct that describes XYST geometry, populate it with the coords of the quad we want to draw and the coords of the texture we want to draw on it
					VVGL::Rect			imgBounds = bufferToDraw->srcRect;
					VVGL::Rect			viewBoundsRect = VVGL::Rect(0., 0., 0., 0.);
					viewBoundsRect.size = n.getOrthoSize();
					VVGL::Rect			geometryRect = ResizeRect(imgBounds, viewBoundsRect, SizingMode_Fit);
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
					glBindVertexArray(vao->name);
					GLERRLOG
		
					uint32_t			vbo = 0;
					if (lastVBOCoords != targetQuad)	{
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
		
					if (lastVBOCoords != targetQuad)	{
						//	delete the VBO we made earlier...
						glDeleteBuffers(1, &vbo);
						GLERRLOG
						//	update the vbo coords ivar (we don't want to update the VBO contents every pass)
						lastVBOCoords = targetQuad;
					}
		
				});
		#endif	//	VVGL_TARGETENV_GL3PLUS
			}
		
		}
	}
}
void GLBufferQWidget::resizeGL(int w, int h)
{
	//cout << __PRETTY_FUNCTION__ << endl;
	lock_guard<recursive_mutex>		lock(ctxLock);
	if (scene != nullptr)	{
		scene->setOrthoSize(Size(w,h));
	}
}
