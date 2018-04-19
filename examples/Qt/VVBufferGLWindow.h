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


class VVBufferGLWindow : public QWindow	{
	Q_OBJECT

public:
	explicit VVBufferGLWindow(GLContextRef & inSharedContext, QWindow * inParent=nullptr);
	~VVBufferGLWindow();
	
	void startRendering();
	void stopRendering();
	
	
	void setContext(const GLContextRef & inCtx);
	
	QThread * getRenderThread();
	
	inline void drawBuffer(GLBufferRef & inBuffer) { lock_guard<mutex> lock(ctxLock); buffer = inBuffer; }
	inline GLBufferRef getBuffer() { lock_guard<mutex> lock(ctxLock); return _getBuffer(); }
	
signals:
	Q_SIGNAL void renderedAFrame();
public slots:
	Q_SLOT void startRenderingSlot();
	Q_SLOT void stopRenderingSlot();
	//Q_SLOT void aboutToQuit();
protected:
	bool event(QEvent * inEvent) Q_DECL_OVERRIDE;
	//void exposeEvent(QExposeEvent * inEvent) Q_DECL_OVERRIDE;
private:
	void renderNow();

private:
	mutex				ctxLock;
	GLContextRef		ctx = nullptr;
	GLSceneRef		scene = nullptr;
	QThread				*ctxThread = nullptr;
	GLBufferRef		vao = nullptr;
	Quad<VertXYST>		lastVBOCoords;	//	the last coords used in the VBO associated with 'vao' (the VAO implicitly retains the VBO, so we only need to update it when the coords change- which we track with this)
	GLBufferRef		buffer = nullptr;
	
	//bool		lastFill = false;
	
	void stopRenderingImmediately();
	inline GLBufferRef _getBuffer() const { return buffer; }
};



/*
// 		this template establishes a function for asynchronously performing a lambda on the passed 
// 		object's thread (or the main thread if there's no passed object).
// 	
// 	usage:
// 		perform_async([]() { qDebug() << __PRETTY_FUNCTION__; });
// 	
// 		perform_async([&]{ o.setObjectName("hello"); }, &o);
// 		perform_async(std::bind(&QObject::setObjectName, &o, "hello"), &o);
template <typename FTYPE>
static void perform_async(FTYPE && inFunc, QObject * inObj=qApp)
{
	struct Event : public QEvent
	{
		using Fun = typename std::decay<FTYPE>::type;
		Fun			varFunc;
		Event(Fun && declInFunc) : QEvent(QEvent::None), varFunc(std::move(declInFunc)) {}
		Event(const Fun & declInFunc) : QEvent(QEvent::None), varFunc(declInFunc) {}
		~Event() { this->varFunc(); }
	};
	QCoreApplication::postEvent(inObj, new Event(std::forward<FTYPE>(inFunc)));
}
*/



#endif // VVBUFFERGLWINDOW_H
