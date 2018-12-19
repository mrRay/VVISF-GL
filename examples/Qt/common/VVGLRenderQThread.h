#ifndef VVGLRENDERQTHREAD_H
#define VVGLRENDERQTHREAD_H

#include <functional>

#include <QThread>
#include <QWaitCondition>
#include <QMutex>

#include <VVGL.hpp>




class VVGLRenderQThread : public QThread
{
	Q_OBJECT
	
	using RenderCallback = std::function<void(VVGLRenderQThread * inRenderThread)>;
	
public:
	//	moves the passed context to the thread (to self), creates a buffer pool & thread copier using the ctx, will use the ctx to perform all rendering
	explicit VVGLRenderQThread(const VVGL::GLContextRef & ctxToUse, QObject * parent = nullptr);
	
	void performRender();
	
	void setRenderCallback(const RenderCallback & n=nullptr);
	VVGL::GLContextRef renderContext();
	VVGL::GLBufferPoolRef bufferPool();
	VVGL::GLTexToTexCopierRef texCopier();
	
public slots:
	Q_SLOT void start(QThread::Priority inPriority=QThread::InheritPriority);
	
protected:
	virtual void run() Q_DECL_OVERRIDE;
	
private:
	QMutex				_condLock;
	QWaitCondition		_cond;
	
	VVGL::GLContextRef			_ctx;
	VVGL::GLBufferPoolRef		_bp;
	VVGL::GLTexToTexCopierRef	_tc;
	
	RenderCallback		_renderCallback = nullptr;
};




#endif // VVGLRENDERQTHREAD_H
