#ifndef ISFGLBUFFERQWIDGET_H
#define ISFGLBUFFERQWIDGET_H

#include "VVGL.hpp"
#include "VVISF.hpp"
#include <mutex>
#include <QOpenGLWidget>

#ifdef Q_OS_MACOS
#include "DisplayLinkDriver.h"
#endif




class ISFGLBufferQWidget : public QOpenGLWidget	{
	Q_OBJECT
	
public:
	ISFGLBufferQWidget(QWidget * inParent=nullptr);
	~ISFGLBufferQWidget();
	
	//	calls the start/stop slot on the appropriate thread
	void startRendering();
	void stopRendering();
	
	//QThread * getRenderThread();
	
	inline VVGL::GLContextRef glContextRef() { std::lock_guard<std::recursive_mutex> lock(ctxLock); return ctx; }
	inline VVISF::ISFSceneRef getScene() { std::lock_guard<std::recursive_mutex> lock(ctxLock); return scene; }
	void drawBuffer(const VVGL::GLBufferRef & inBuffer);
	VVGL::GLBufferRef getBuffer();
	
public slots:
	Q_SLOT void startRenderingSlot();
	Q_SLOT void stopRenderingSlot();
	Q_SLOT void aboutToQuit();
	
signals:
	
protected:
	//bool event(QEvent * inEvent) Q_DECL_OVERRIDE;
	void paintGL() Q_DECL_OVERRIDE;
	void initializeGL() Q_DECL_OVERRIDE;
	void resizeGL(int w, int h) Q_DECL_OVERRIDE;
	
protected:
	void _renderNow();
	
protected:
#ifdef Q_OS_MACOS
	DisplayLinkDriver		displayLinkDriver;	//	calling 'QOpenGLWidget::update' on macs breaks Qt: https://bugreports.qt.io/browse/QTBUG-73209
#endif
	std::recursive_mutex	ctxLock;
	VVGL::GLContextRef		ctx = nullptr;
	VVISF::ISFSceneRef		scene = nullptr;
	QThread					*ctxThread = nullptr;
	VVGL::GLBufferRef		buffer = nullptr;
	
	void stopRenderingImmediately();
	inline VVGL::GLBufferRef _getBuffer() const { return buffer; }
};




#endif // ISFGLBUFFERQWIDGET_H
