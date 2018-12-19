#include "VVGLRenderQThread.h"

#include <QMutexLocker>
#include <QAbstractEventDispatcher>
#include <QDebug>




using namespace std;
using namespace VVGL;




VVGLRenderQThread::VVGLRenderQThread(const VVGL::GLContextRef & ctxToUse, QObject * parent) :
	QThread(parent),
	_ctx(ctxToUse)
{
	//_ctx->moveToThread(this);
	_bp = make_shared<GLBufferPool>(ctxToUse);
	_tc = CreateGLTexToTexCopierRefUsing(_ctx);
	_tc->setPrivatePool(_bp);
	
	connect(this, &QThread::finished, [&]()	{
		//	move the contexts back to the main thread before we exit
		QThread		*mainThread = qApp->thread();
		if (mainThread != nullptr)	{
			if (_ctx != nullptr)
				_ctx->moveToThread(mainThread);
			if (_bp != nullptr)
				_bp->context()->moveToThread(mainThread);
			if (_tc != nullptr)
				_tc->context()->moveToThread(mainThread);
		}
	});
}


void VVGLRenderQThread::performRender()	{
	_cond.wakeOne();
}


void VVGLRenderQThread::setRenderCallback(const RenderCallback & n)	{
	QMutexLocker		tmpLock(&_condLock);
	_renderCallback = n;
}
VVGL::GLContextRef VVGLRenderQThread::renderContext()	{
	return _ctx;
}
VVGL::GLBufferPoolRef VVGLRenderQThread::bufferPool()	{
	return _bp;
}
VVGL::GLTexToTexCopierRef VVGLRenderQThread::texCopier()	{
	return _tc;
}


void VVGLRenderQThread::start(QThread::Priority inPriority)	{
	qDebug() << __PRETTY_FUNCTION__;
	
	if (_ctx != nullptr)
		_ctx->moveToThread(this);
	if (_bp != nullptr)
		_bp->context()->moveToThread(this);
	if (_tc != nullptr)
		_tc->context()->moveToThread(this);
	
	QThread::start(inPriority);
}


void VVGLRenderQThread::run()	{
	qDebug() << __PRETTY_FUNCTION__;
	
	while (1)	{
		//qDebug() << "\tentering wait loop...";
		
		_condLock.lock();
		if (!_cond.wait(&_condLock, 17))	{
			//qDebug() << "\twait loop timed out...";
			
			_condLock.unlock();
			
			//	if i'm here, then the wait timed out- check to see if the thread should still be running, run another loop if it's clear
			if (isFinished() || isInterruptionRequested() || !isRunning())	{
				//qDebug() << "\tfinished or requested interrupt or not running- bailing";
				break;
			}
			else	{
				continue;
			}
		}
		//else
			//qDebug() << "\texited wait loop!";
		
		//	...if i'm here then the wait didn't time out- something signaled the condition
		
		//	check to see if the thread should still be running
		if (isFinished() || isInterruptionRequested() || !isRunning())	{
			//qDebug() << "\tfinished or requested interrupt or not running- bailing";
			break;
		}
		
		//	perform the render callback
		if (_renderCallback != nullptr)	{
			_ctx->makeCurrentIfNotCurrent();
			_renderCallback(this);
		}
		//	do housekeeping on the buffer pool
		if (_bp != nullptr)
			_bp->housekeeping();
		
		_condLock.unlock();
		
		//	process some events
		QAbstractEventDispatcher		*ed = eventDispatcher();
		if (ed != nullptr)	{
			ed->processEvents(QEventLoop::AllEvents);
		}
	}
	
	//qDebug() << "\t" <<  __PRETTY_FUNCTION__ << "- FINISHED";
}
