#ifndef VVBUFFERGLWINDOW_H
#define VVBUFFERGLWINDOW_H

#include "VVGL.hpp"
#include <QWindow>
//class QPainter;
//class QOpenGLContext;
//class QOpenGLPaintDevice;
#include <mutex>
#include <QThread>
#include <QCoreApplication>


using namespace std;
using namespace VVGL;


class GLBufferQWindow : public QWindow	{
	Q_OBJECT

public:
	explicit GLBufferQWindow(GLContextRef & inSharedContext, QWindow * inParent=nullptr);
	explicit GLBufferQWindow(QWindow * inParent = nullptr);
	~GLBufferQWindow();
	
	void startRendering();
	void stopRendering();
	
	
	void setContext(const GLContextRef & inCtx);
	
	QThread * getRenderThread();
	
	inline void drawBuffer(GLBufferRef & inBuffer) { lock_guard<recursive_mutex> lock(ctxLock); buffer = inBuffer; }
	inline GLBufferRef getBuffer() { lock_guard<recursive_mutex> lock(ctxLock); return _getBuffer(); }
	
signals:
	Q_SIGNAL void renderedAFrame();
public slots:
	Q_SLOT void startRenderingSlot();
	Q_SLOT void stopRenderingSlot();
	Q_SLOT void aboutToQuit();


protected:
	bool event(QEvent * inEvent) Q_DECL_OVERRIDE;
	//void exposeEvent(QExposeEvent * inEvent) Q_DECL_OVERRIDE;
private:
	void renderNow();

private:
	recursive_mutex		ctxLock;
	GLContextRef		ctx = nullptr;
	GLSceneRef			scene = nullptr;
	QThread				*ctxThread = nullptr;
	GLBufferRef			vao = nullptr;
	Quad<VertXYST>		lastVBOCoords;	//	the last coords used in the VBO associated with 'vao' (the VAO implicitly retains the VBO, so we only need to update it when the coords change- which we track with this)
	GLBufferRef			buffer = nullptr;
	
	void stopRenderingImmediately();
	inline GLBufferRef _getBuffer() const { return buffer; }
};



#endif // VVBUFFERGLWINDOW_H
