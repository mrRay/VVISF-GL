#ifndef VVGLQTCTXWRAPPER_H
#define VVGLQTCTXWRAPPER_H

#if ISF_TARGET_QT

#include <QSurfaceFormat>
#include <QThread>
#include <memory>
class QOpenGLContext;
class QSurface;


/*		this class was created to hold a shared_ptr to a QOpenGLContext, and prevent anything using
this class from knowing that the backend is a QOpenGLContext so Qt's OpenGL stuff doesn't conflict with GLEW

		this class exists because if you #include GLEW and Qt's OpenGL stuff there are conflicts and
GLEW's pointers get overwritten.  i want to use the GL environment provided by Qt's classes without
actually using those classes (i want to make raw GL calls), so i need to #include GLEW in my GL code
and make this PIMPL idiom to contain and manipulate the underlying Qt GL context.		*/


namespace VVGL	{


//	forward declaration for a struct that will hold the Qt GL stuff so their #includes don't conflict with GLEW
struct VVGLQtCtxHidden;


class VVGLQtCtxWrapper	{
private:
	//	this struct will contain the actual implementation of this class
	VVGLQtCtxHidden		*hidden = nullptr;

public:
	//	class functions that generate QSurfaceFormats
	static QSurfaceFormat CreateDefaultSurfaceFormat();
	static QSurfaceFormat CreateCompatibilityGLSurfaceFormat();
	static QSurfaceFormat CreateGL3SurfaceFormat();
	static QSurfaceFormat CreateGL4SurfaceFormat();
	static QOpenGLContext * GetCurrentContext();
	
	
	
	//	creates a new GL context and surface which it "owns", like calling VVGLQtCtxWrapper(nullptr,nullptr,true);
	VVGLQtCtxWrapper();
	//	if 'inTargetSurface' is null, a QOffscreenSurface will be created as a child of the QOpenGLContext.  if it's non-null (a widget or a window or etc), we just get a weak ref to it.
	//	if 'inCreateCtx' is YES, a new GL context will be created and it will share 'inCtx'.
	//	if 'inCreateCtx' is NO, no GL context will be created and instead a weak ref to 'inCtx' will be established.
	VVGLQtCtxWrapper(QSurface * inSurface, QOpenGLContext * inCtx, bool inCreateCtx=true, QSurfaceFormat inSfcFmt=VVGLQtCtxWrapper::CreateDefaultSurfaceFormat());
	~VVGLQtCtxWrapper();

	//	copy constructors- these do NOT create new GL contexts, they merely copy/retain the GL contexts from the passed var
	VVGLQtCtxWrapper(const VVGLQtCtxWrapper * n);
	VVGLQtCtxWrapper(const VVGLQtCtxWrapper & n);
	VVGLQtCtxWrapper(const std::shared_ptr<VVGLQtCtxWrapper> & n);

public:
	void setShareContext(QOpenGLContext * inCtx);
	QOpenGLContext * getContext();
	bool isSharingWith(QOpenGLContext * inCtx);
	
	void setSurface(const QSurface * inTargetSurface);
	void swap();
	void moveToThread(QThread * inThread);

	void makeCurrent();
	void makeCurrentIfNotCurrent();
	void makeCurrentIfNull();
};


}

#endif	//	ISF_TARGET_QT

#endif // VVGLQTCTXWRAPPER_H
