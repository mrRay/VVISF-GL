#include "VVBufferGLWindow.h"
#include <QDebug>
//#include <QString>
//#include "VVGL.hpp"
#include <QCoreApplication>


/*	========================================	*/
#pragma mark --------------------- constructors/destructors


VVBufferGLWindow::VVBufferGLWindow(VVGLContextRef & inSharedContext, QWindow * inParent) : QWindow(inParent)
{
	setSurfaceType(QWindow::OpenGLSurface);
	//lock_guard<mutex>		lock(ctxLock);
	//ctx = (inSharedContext==nullptr) ? nullptr : inSharedContext->newContextSharingMe();
	//ctx->setSurface(this);
	setContext((inSharedContext==nullptr) ? nullptr : inSharedContext->newContextSharingMe());
	
	//connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(aboutToQuit()));
}
VVBufferGLWindow::~VVBufferGLWindow()
{
	//qDebug()<<__PRETTY_FUNCTION__;
	stopRenderingImmediately();
	//makeCurrent();
	//teardownGL();
	
	lock_guard<mutex>		lock(ctxLock);
	ctx = nullptr;
	vao = nullptr;
	buffer = nullptr;
}


/*	========================================	*/
#pragma mark --------------------- private methods


void VVBufferGLWindow::renderNow()
{
	//qDebug() << __PRETTY_FUNCTION__;
	
	bool		renderedSomething = false;
	{
		lock_guard<mutex>		lock(ctxLock);
		//qDebug()<<"\tctxThread is "<<ctxThread<<", current thread is "<<QThread::currentThread()<<", main thread is "<< QCoreApplication::instance()->thread();
		if (ctxThread!=nullptr && ctxThread==QThread::currentThread())	{
			if (isExposed())	{
				if (scene != nullptr)	{
					double		ltbbm = devicePixelRatio();
					Size		tmpSize(width()*ltbbm,height()*ltbbm);
					scene->setOrthoSize(tmpSize);
					scene->render();
				
					//	swap the context
					if (ctx != nullptr)
						ctx->swap();
				
				}
				/*
				if (ctx != nullptr)	{
					ctx->makeCurrent();
					//qDebug() << "current ctx is " << VVGLContext::GetCurrentContext();
					if (lastFill)
						glClearColor(0., 0., 0., 1.);
					else
						glClearColor(1., 1., 1., 1.);
					lastFill = !lastFill;
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
					glFlush();
					ctx->swap();
				}
				*/
				
				renderedSomething = true;
				
			}
		}
	}
	//requestUpdate();
	
	requestUpdate();
	if (renderedSomething)	{
		//update();
		emit renderedAFrame();
	}
	GetGlobalBufferPool()->housekeeping();
}


/*	========================================	*/
#pragma mark --------------------- public methods


void VVBufferGLWindow::startRendering()
{
	//qDebug() << __PRETTY_FUNCTION__;
	
	bool					callDirectly = false;
	{
		lock_guard<mutex>		lock(ctxLock);
		//	if there's already a context thread then i'm already rendering and i can return
		if (ctxThread != nullptr)
			return;
	
		//	...if i'm here there's no context thread, i'm not rendering, and i need to start
	
		if (QThread::currentThread() == this->thread())
			callDirectly = true;
	}
	
	//	if the current thread is the thread i'm bound to, i can call the slot directly
	if (callDirectly)
		startRenderingSlot();
	//	else i'm on the "wrong thread", i need to use the meta object to send a signal to the slot
	else
		QMetaObject::invokeMethod(this, "startRenderingSlot");
	
}
void VVBufferGLWindow::stopRendering()
{
	//qDebug() << __PRETTY_FUNCTION__;
	
	bool					callDirectly = false;
	{
		lock_guard<mutex>		lock(ctxLock);
		//	if there isn't a context thread then i've already stopped rendering and should return
		if (ctxThread == nullptr)
			return;
	
		//	...if i'm here then there's a context thread, and i need to stop it
		
		if (QThread::currentThread() == this->thread())
			callDirectly = true;
	}
	
	//	if the current thread is the thread i'm bound to, i can call the slot directly
	if (callDirectly)
		stopRenderingSlot();
	//	else i'm on the "wrong thread", i need to use the meta object to send a signal to the slot
	else
		QMetaObject::invokeMethod(this, "stopRenderingSlot");
	
}
void VVBufferGLWindow::stopRenderingImmediately()	{
	{
		lock_guard<mutex>		lock(ctxLock);
		if (ctxThread == nullptr)
			return;
	}
	QMetaObject::invokeMethod(this, "stopRenderingSlot", Qt::BlockingQueuedConnection);
}

void VVBufferGLWindow::setContext(const VVGLContextRef & inCtx)
{
	lock_guard<mutex>		lock(ctxLock);
	ctx = inCtx;
	scene = nullptr;
	
	if (ctx != nullptr)	{
		ctx->setSurface(this);
		scene = make_shared<VVGLScene>(ctx);
		scene->setPerformClear(true);
		scene->setClearColor(0., 0., 0., 0.);
		
		if (ctx->version == GLVersion_2)	{
			
			scene->setRenderPrepCallback([](const VVGLScene & n, const bool & inReshaped, const bool & inPgmChanged){
			});
			scene->setRenderCallback([&](const VVGLScene & n){
				double		ltbbm = devicePixelRatio();
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
				VVGLBufferRef		bufferToDraw = _getBuffer();
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
#if ISF_TARGET_GL3PLUS
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
			VVGLCachedAttribRef		xyzAttr = make_shared<VVGLCachedAttrib>("inXYZ");
			VVGLCachedAttribRef		stAttr = make_shared<VVGLCachedAttrib>("inST");
			VVGLCachedUniRef		inputImageUni = make_shared<VVGLCachedUni>("inputImage");
			VVGLCachedUniRef		inputImageRectUni = make_shared<VVGLCachedUni>("inputImageRect");
			VVGLCachedUniRef		isRectTexUni = make_shared<VVGLCachedUni>("isRectTex");
			//	the render prep callback needs to create & populate a VAO, and cache the location of the vertex attributes and uniforms
			scene->setRenderPrepCallback([=](const VVGLScene & n, const bool & inReshaped, const bool & inPgmChanged)	{
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
			scene->setRenderCallback([=](const VVGLScene & n)	{
				//cout << __PRETTY_FUNCTION__ << endl;
				//	clear
				glClearColor(0., 0., 0., 1.);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
				//	get the buffer we want to draw
				VVGLBufferRef		bufferToDraw = _getBuffer();
				if (bufferToDraw == nullptr)
					return;
				//cout << "\tbufferToDraw is " << bufferToDraw->getDescriptionString() << endl;
				//	try to get the VAO.  if the VAO's null, create it and store it in the VVGLBufferGLView as an ivar. 
				if (vao == nullptr)	{
					VVGLBufferPoolRef		bp = (bufferToDraw==nullptr) ? nullptr : bufferToDraw->parentBufferPool;
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
				glBindTexture(VVGLBuffer::Target_2D, (bufferToDraw!=nullptr && bufferToDraw->desc.target==VVGLBuffer::Target_2D) ? bufferToDraw->name : 0);
				GLERRLOG
				glBindTexture(VVGLBuffer::Target_Rect, 0);
				GLERRLOG
				if (inputImageUni->loc >= 0)	{
					glUniform1i(inputImageUni->loc, 0);
					GLERRLOG
				}
				//	pass the RECT texture to the program (if there's a RECT texture)
				glActiveTexture(GL_TEXTURE1);
				GLERRLOG
				glBindTexture(VVGLBuffer::Target_2D, 0);
				GLERRLOG
				glBindTexture(VVGLBuffer::Target_Rect, (bufferToDraw!=nullptr && bufferToDraw->desc.target==VVGLBuffer::Target_Rect) ? bufferToDraw->name : 0);
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
						case VVGLBuffer::Target_2D:
							glUniform1i(isRectTexUni->loc, 1);
							break;
						case VVGLBuffer::Target_Rect:
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
#endif//	ISF_TARGET_GL3PLUS
		}

	}
}

QThread * VVBufferGLWindow::getRenderThread()
{
	lock_guard<mutex>		lock(ctxLock);
	return ctxThread;
}
/*
void VVBufferGLWindow::drawBuffer(VVGLBufferRef & inBuffer)
{
	lock_guard<mutex>		lock(ctxLock);
	buffer = inBuffer;
}
VVGLBufferRef VVBufferGLWindow::getBuffer()
{
	lock_guard<mutex>		lock(ctxLock);
	return buffer;
}
*/

/*	========================================	*/
#pragma mark --------------------- public slots


void VVBufferGLWindow::startRenderingSlot()
{
	lock_guard<mutex>		lock(ctxLock);
	//qDebug() << __PRETTY_FUNCTION__;
	//qDebug()<<"\tcurrent thread is "<<QThread::currentThread()<<", main thread is "<<QCoreApplication::instance()->thread();
	
	if (ctxThread != nullptr)
		return;
	
	ctxThread = new QThread;
	//qDebug()<<"\tctxThread is now "<<ctxThread;
	this->moveToThread(ctxThread);
	if (ctx != nullptr)
		ctx->moveToThread(ctxThread);
	//connect(ctxThread, SIGNAL(started()), this, SLOT(requestUpdate()));
	connect(ctxThread, &QThread::started, this, &VVBufferGLWindow::requestUpdate);
	ctxThread->start();
}
void VVBufferGLWindow::stopRenderingSlot()
{
	lock_guard<mutex>		lock(ctxLock);
	//qDebug() << __PRETTY_FUNCTION__;
	//qDebug()<<"\tcurrent thread is "<<QThread::currentThread()<<", main thread is "<<QCoreApplication::instance()->thread();
	
	if (ctxThread == nullptr)
		return;
	
	//	get rid of the scene now (it has to delete its program and other related vars, which must be done on this thread because this is the context's thread right now)
	vao = nullptr;
	buffer = nullptr;
	scene = nullptr;
	//	now move everything back to the main thread
	QThread		*mainThread = QCoreApplication::instance()->thread();
	if (mainThread != nullptr)	{
		this->moveToThread(mainThread);
		if (ctx != nullptr)
			ctx->moveToThread(mainThread);
	}
	//	shut down the thread we were using to render
	ctxThread->quit();
	ctxThread->deleteLater();
	ctxThread = nullptr;
	//qDebug()<<"\tctxThread is now "<<ctxThread;
}
/*
void VVBufferGLWindow::aboutToQuit()	{
	qDebug() << __PRETTY_FUNCTION__;
	stopRendering();
}
*/


/*	========================================	*/
#pragma mark --------------------- superclass overrides


bool VVBufferGLWindow::event(QEvent * inEvent)
{
	//qDebug() << __PRETTY_FUNCTION__;
	switch (inEvent->type()) {
	case QEvent::UpdateRequest:
		//m_update_pending = false;
		renderNow();
		return true;
	default:
		return QWindow::event(inEvent);
	}
}
void VVBufferGLWindow::exposeEvent(QExposeEvent * inEvent)
{
	//qDebug() << __PRETTY_FUNCTION__;
	Q_UNUSED(inEvent);
	if (isExposed())
		renderNow();
}

