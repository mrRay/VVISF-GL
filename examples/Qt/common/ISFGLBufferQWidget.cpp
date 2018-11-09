#include "ISFGLBufferQWidget.h"

//#include "VVGL.hpp"
//#include <QOpenGLContext>
//#include <QSurface>
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QWindow>




using namespace std;
using namespace VVGL;
using namespace VVISF;




ISFGLBufferQWidget::ISFGLBufferQWidget(QWidget * inParent) :
	QOpenGLWidget(inParent)
{
	//cout << __PRETTY_FUNCTION__ << endl;
	connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(aboutToQuit()));
}
ISFGLBufferQWidget::~ISFGLBufferQWidget()
{
	lock_guard<recursive_mutex>		lock(ctxLock);
	ctx = nullptr;
	buffer = nullptr;
}


void ISFGLBufferQWidget::drawBuffer(const GLBufferRef & inBuffer)
{
	lock_guard<recursive_mutex>		lock(ctxLock);
	buffer = inBuffer;
	update();
}
GLBufferRef ISFGLBufferQWidget::getBuffer()
{
	lock_guard<recursive_mutex>		lock(ctxLock);
	GLBufferRef		returnMe = buffer;
	return returnMe;
}




void ISFGLBufferQWidget::_renderNow()	{
	//cout << __PRETTY_FUNCTION__ << endl;
	
	GLBufferPoolRef		bp = GetGlobalBufferPool();
	bool			renderAnotherFrame = true;
	if (bp != nullptr)	{
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
					if (buffer != nullptr)	{
						scene->setFilterInputBuffer(buffer);
					}
					scene->render();
				}
			//}
		}
	}
	
	if (renderAnotherFrame)	{
		update();
	}
	
}


/*	========================================	*/
#pragma mark --------------------- public methods


void ISFGLBufferQWidget::startRendering()	{
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
void ISFGLBufferQWidget::stopRendering()	{
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
void ISFGLBufferQWidget::stopRenderingImmediately()	{
	//cout << __PRETTY_FUNCTION__ << endl;
	{
		lock_guard<recursive_mutex>		lock(ctxLock);
		if (ctxThread == nullptr)
			return;
	}
	QMetaObject::invokeMethod(this, "stopRenderingSlot", Qt::BlockingQueuedConnection);
}


QThread * ISFGLBufferQWidget::getRenderThread()	{
	lock_guard<recursive_mutex>		lock(ctxLock);
	return ctxThread;
}




/*	========================================	*/
#pragma mark --------------------- public slots




void ISFGLBufferQWidget::startRenderingSlot()
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
	//connect(ctxThread, &QThread::started, this, &ISFGLBufferQWidget::requestUpdate);
	//ctxThread->start();
	update();
	//cout << "\tFINISHED- " << __PRETTY_FUNCTION__ << endl;
}
void ISFGLBufferQWidget::stopRenderingSlot()
{
	//cout << __PRETTY_FUNCTION__ << " stopRenderingSlot" << endl;
	//cout<<"\tcurrent thread is "<<QThread::currentThread()<<", main thread is "<<QCoreApplication::instance()->thread()<<endl;
	lock_guard<recursive_mutex>		lock(ctxLock);
	
	if (ctxThread == nullptr)
		return;
	
	//	get rid of the scene now (it has to delete its program and other related vars, which must be done on this thread because this is the context's thread right now)
	//vao = nullptr;
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
void ISFGLBufferQWidget::aboutToQuit()	{
	//cout << __PRETTY_FUNCTION__ << endl;
	stopRendering();
}


/*	========================================	*/
#pragma mark --------------------- superclass overrides


void ISFGLBufferQWidget::paintGL()
{
	//cout << __PRETTY_FUNCTION__ << endl;
	//qDebug() << "\t" << QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
	
	emit aboutToRedraw(this);
	
	QOpenGLWidget::paintGL();
	makeCurrent();
	_renderNow();
	
	GLBufferPoolRef		bp = GetGlobalBufferPool();
	if (bp != nullptr)
		bp->housekeeping();
	
}
void ISFGLBufferQWidget::initializeGL()
{
	//cout << __PRETTY_FUNCTION__ << endl;
	
	makeCurrent();
	
	//	make a GLContextRef that wraps the widget's context.  this doesn't create a new context, it just wraps an existing context.
	lock_guard<recursive_mutex>		lock(ctxLock);
	ctx = make_shared<GLContext>(context());
	
	if (ctx != nullptr)	{
		//ctx->makeCurrent();
		
		//	make a scene that uses the context
		scene = CreateISFSceneRefUsing(ctx);
		
		if (scene != nullptr)	{
			//	we want to load an ISF file that was included in the qrc file, and is only accessible to non-Qt SDKs as a string...
			QFile			fragShaderFile(":/resources/AlphaOverCheckerboard.fs");
			QString			fragShaderString;
			if (fragShaderFile.open(QFile::ReadOnly))	{
				QTextStream		rStream(&fragShaderFile);
				fragShaderString = rStream.readAll();
			}
			else
				fragShaderString = QString("");
			ISFDocRef		isfDoc = CreateISFDocRefWith(fragShaderString.toStdString());
			scene->useDoc(isfDoc);
			
			
			
			//scene->setPerformClear(true);
			//scene->setClearColor(0., 0., 0., 0.);
		}
		
		
	}
}
void ISFGLBufferQWidget::resizeGL(int w, int h)
{
	QWidget				*winWidget = window();
	QWindow				*myWin = (winWidget==nullptr) ? nullptr : winWidget->windowHandle();
	qreal				devicePixelRatio = 1.0;
	
	if (myWin != nullptr)
		devicePixelRatio = myWin->devicePixelRatio();
	
	lock_guard<recursive_mutex>		lock(ctxLock);
	if (scene != nullptr)	{
		scene->setOrthoSize(Size(w*devicePixelRatio, h*devicePixelRatio));
	}
}
