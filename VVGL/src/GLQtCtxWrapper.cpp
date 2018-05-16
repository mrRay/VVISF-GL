#include "GLQtCtxWrapper.hpp"

#if defined(VVGL_SDK_QT)

#include <QOpenGLContext>
#include <QSurface>
#include <QPointer>
#include <QOffscreenSurface>
#include <QDebug>

#include <memory>
#include <iostream>

namespace VVGL
{
using namespace std;



/*	========================================	*/
#pragma mark --------------------- "hidden" struct




struct GLQtCtxHidden	{
public:
	//	only one of these vars should be non-nil at a time!
	QOffscreenSurface		*strongSurface = nullptr;
	QSurface				*weakSurface = nullptr;	//	can't be a QPointer, QSurface is not a subclass of QObject.

	QSharedPointer<QOpenGLContext>		strongCtx = nullptr;	//	if we have a strong ref to our context then this is non-nil
	QPointer<QOpenGLContext>	weakCtx = nullptr;	//	if we only have a weak ref to our context (because the context is owned by a Qt-based widget/window/etc) then this is non-nil

public:
	//	creates a new GL context and surface which it "owns", like calling GLQtCtxWrapper(nullptr,nullptr,true);
	GLQtCtxHidden();
	//	if 'inTargetSurface' is null, a QOffscreenSurface will be created as a child of the QOpenGLContext.  if it's non-null (a widget or a window or etc), we just get a weak ref to it.
	//	if 'inCreateCtx' is YES, a new GL context will be created and it will share 'inCtx'.
	//	if 'inCreateCtx' is NO, no GL context will be created and instead a weak ref to 'inCtx' will be established.
	GLQtCtxHidden(QSurface * inSurface, QOpenGLContext * inCtx, bool inCreateCtx=true, QSurfaceFormat inSfcFmt=GLQtCtxWrapper::CreateDefaultSurfaceFormat());

	~GLQtCtxHidden();

	//	copy constructors- these do NOT create new GL contexts, they merely copy/retain the GL contexts from the passed var
	GLQtCtxHidden(const GLQtCtxHidden * n);
	GLQtCtxHidden(const GLQtCtxHidden & n);
	GLQtCtxHidden(const std::shared_ptr<GLQtCtxHidden> & n);

	//	accessors
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




//	creates a new GL context and surface which it "owns", like calling GLQtCtxWrapper(nullptr,nullptr,true);
GLQtCtxHidden::GLQtCtxHidden()	{
	//cout << __PRETTY_FUNCTION__ << ", " << this << endl;
	//qDebug()<<__PRETTY_FUNCTION__<<endl;
	
	strongSurface = new QOffscreenSurface;
	strongSurface->create();

	strongCtx = QSharedPointer<QOpenGLContext>(new QOpenGLContext);
	strongCtx->setFormat(GLQtCtxWrapper::CreateDefaultSurfaceFormat());
	strongCtx->create();
}
//	if 'inTargetSurface' is null, a QOffscreenSurface will be created as a child of the QOpenGLContext.  if it's non-null (a widget or a window or etc), we just get a weak ref to it.
//	if 'inCreateCtx' is YES, a new GL context will be created and it will share 'inCtx'.
//	if 'inCreateCtx' is NO, no GL context will be created and instead a weak ref to 'inCtx' will be established.
GLQtCtxHidden::GLQtCtxHidden(QSurface * inSurface, QOpenGLContext * inCtx, bool inCreateCtx, QSurfaceFormat inSfcFmt)	{
	//cout << __PRETTY_FUNCTION__ << ", " << this << endl;
	//qDebug() << "\tcurrent context is " << QOpenGLContext::currentContext();
	
	if (inSurface == nullptr)	{
		strongSurface = new QOffscreenSurface;
		strongSurface->create();
	}
	else	{
		weakSurface = inSurface;
	}
	
	if (inCreateCtx)	{
		strongCtx = QSharedPointer<QOpenGLContext>(new QOpenGLContext);
		//qDebug() << "\tstrongCtx is " << strongCtx.data();
		if (inCtx != nullptr)
			strongCtx->setShareContext(inCtx);
		strongCtx->setFormat(inSfcFmt);
		strongCtx->create();
	}
	else	{
		weakCtx = inCtx;
	}
}
GLQtCtxHidden::~GLQtCtxHidden()	{
	//cout << __PRETTY_FUNCTION__ << ", " << this << endl;
	
	if (strongSurface!=nullptr)	{
		delete strongSurface;
		strongSurface = nullptr;
	}
	if (weakSurface!=nullptr)	{
		weakSurface = nullptr;
	}
	strongCtx.clear();
	if (weakCtx!=nullptr)	{
		weakCtx = nullptr;
	}
}

//	copy constructors- these do NOT create new GL contexts, they merely copy/retain the GL contexts from the passed var
GLQtCtxHidden::GLQtCtxHidden(const GLQtCtxHidden * n)	{
	strongSurface = nullptr;
	weakSurface = nullptr;
	strongCtx.clear();
	weakCtx = nullptr;
	if (n != nullptr)	{
		//	if the passed object has a weak surface (window/widget/etc), get a weak ref to it
		if (n->weakSurface != nullptr)
			weakSurface = n->weakSurface;
		//	else the passed object either has a strong surface (offscreen, i need my own) or no surface (i need an offscreen)
		else	{
			strongSurface = new QOffscreenSurface;
			strongSurface->create();
		}
		//	if the passed object has a weak ctx, get a weak ref to it
		if (n->weakCtx != nullptr)
			weakCtx = n->weakCtx;
		//	else if the passed object has a strong ctx, i want to get a strong ref to it
		else if (!n->strongCtx.isNull())
			strongCtx = n->strongCtx;
	}
}
GLQtCtxHidden::GLQtCtxHidden(const GLQtCtxHidden & n)	{
	strongSurface = nullptr;
	weakSurface = nullptr;
	strongCtx.clear();
	weakCtx = nullptr;
	//if (n != nullptr)	{
		//	if the passed object has a weak surface (window/widget/etc), get a weak ref to it
		if (n.weakSurface != nullptr)
			weakSurface = n.weakSurface;
		//	else the passed object either has a strong surface (offscreen, i can't share & need my own) or no surface (i need an offscreen)
		else	{
			strongSurface = new QOffscreenSurface;
			strongSurface->create();
		}
		//	if the passed object has a weak ctx, get a weak ref to it
		if (n.weakCtx != nullptr)
			weakCtx = n.weakCtx;
		//	else if the passed object has a strong ctx, i want to get a strong ref to it
		else if (!n.strongCtx.isNull())
			strongCtx = n.strongCtx;
	//}
}
GLQtCtxHidden::GLQtCtxHidden(const std::shared_ptr<GLQtCtxHidden> & n)	{
	strongSurface = nullptr;
	weakSurface = nullptr;
	strongCtx.clear();
	weakCtx = nullptr;
	if (n != nullptr)	{
		//	if the passed object has a weak surface (window/widget/etc), get a weak ref to it
		if (n->weakSurface != nullptr)
			weakSurface = n->weakSurface;
		//	else the passed object either has a strong surface (offscreen, i need my own) or no surface (i need an offscreen)
		else	{
			strongSurface = new QOffscreenSurface;
			strongSurface->create();
		}
		//	if the passed object has a weak ctx, get a weak ref to it
		if (n->weakCtx != nullptr)
			weakCtx = n->weakCtx;
		//	else if the passed object has a strong ctx, i want to get a strong ref to it
		else if (!n->strongCtx.isNull())
			strongCtx = n->strongCtx;
	}
}
//	accessors
void GLQtCtxHidden::setShareContext(QOpenGLContext * inCtx)	{
	if (weakCtx != nullptr)	{
		weakCtx->setShareContext(inCtx);
		weakCtx->create();
	}
	else if (strongCtx != nullptr)	{
		strongCtx->setShareContext(inCtx);
		strongCtx->create();
	}
}
QOpenGLContext * GLQtCtxHidden::getContext()	{
	if (weakCtx != nullptr)
		return weakCtx;
	if (strongCtx != nullptr)
		return &(*strongCtx);
	return nullptr;
}
bool GLQtCtxHidden::isSharingWith(QOpenGLContext * inCtx)	{
	if (inCtx == nullptr)
		return false;
	QOpenGLContext		*tmpCtx = getContext();
	if (tmpCtx == nullptr)
		return false;
	return QOpenGLContext::areSharing(tmpCtx, inCtx);
}
void GLQtCtxHidden::setSurface(const QSurface * inTargetSurface)	{
	//qDebug() << __PRETTY_FUNCTION__ << ", " << inTargetSurface;
	if (strongSurface != nullptr)	{
		delete strongSurface;
		strongSurface = nullptr;
	}
	weakSurface = const_cast<QSurface*>(inTargetSurface);
}
void GLQtCtxHidden::swap()	{
	//qDebug() << __PRETTY_FUNCTION__;
	
	QOpenGLContext		*ctxToUse = nullptr;
	if (weakCtx != nullptr)
		ctxToUse = weakCtx;
	else
		ctxToUse = strongCtx.data();
	QSurface			*sfcToUse = nullptr;
	if (weakSurface != nullptr)
		sfcToUse = weakSurface;
	else
		sfcToUse = strongSurface;

	if (ctxToUse!=nullptr && sfcToUse!=nullptr)	{
		ctxToUse->swapBuffers(sfcToUse);
	}
}
void GLQtCtxHidden::moveToThread(QThread * inThread)	{
	QOpenGLContext		*ctxToUse = nullptr;
	if (weakCtx != nullptr)
		ctxToUse = weakCtx;
	else
		ctxToUse = strongCtx.data();
	
	if (ctxToUse != nullptr)	{
		ctxToUse->moveToThread(inThread);
	}
	//qDebug()<<"ctx "<<this<<" is now on thread "<<inThread;
}
void GLQtCtxHidden::makeCurrent()	{
	//qDebug() << __PRETTY_FUNCTION__;
	
	QOpenGLContext		*ctxToUse = nullptr;
	if (weakCtx != nullptr)
		ctxToUse = weakCtx;
	else
		ctxToUse = strongCtx.data();
	QSurface			*sfcToUse = nullptr;
	if (weakSurface != nullptr)
		sfcToUse = weakSurface;
	else
		sfcToUse = strongSurface;

	if (ctxToUse!=nullptr && sfcToUse!=nullptr)
		ctxToUse->makeCurrent(sfcToUse);
}
void GLQtCtxHidden::makeCurrentIfNotCurrent()	{
	//cout << __PRETTY_FUNCTION__ << endl;
	
	QOpenGLContext		*current = QOpenGLContext::currentContext();
	QOpenGLContext		*ctxToUse = nullptr;
	if (weakCtx != nullptr)
		ctxToUse = weakCtx;
	else
		ctxToUse = strongCtx.data();
	QSurface			*sfcToUse = nullptr;
	if (weakSurface != nullptr)
		sfcToUse = weakSurface;
	else
		sfcToUse = strongSurface;

	if (ctxToUse!=nullptr && sfcToUse!=nullptr)	{
		if (ctxToUse != current)	{
			//cout << "\tshould be making current, ctx thread is " << ctxToUse->thread() << ", current thread is " << QThread::currentThread() << endl;
			ctxToUse->makeCurrent(sfcToUse);
			//qDebug() << "\tcurrent context is " << QOpenGLContext::currentContext();
		}
	}
	else	{
		qDebug() << "ERR: " << __PRETTY_FUNCTION__ << " ctx (" << ctxToUse << ") or surface (" << sfcToUse << ") was nil";
	}
}
void GLQtCtxHidden::makeCurrentIfNull()	{
	//cout << __PRETTY_FUNCTION__ << endl;
	
	QOpenGLContext		*current = QOpenGLContext::currentContext();
	if (current != nullptr)
		return;

	QOpenGLContext		*ctxToUse = nullptr;
	if (weakCtx != nullptr)
		ctxToUse = weakCtx;
	else
		ctxToUse = strongCtx.data();
	QSurface			*sfcToUse = nullptr;
	if (weakSurface != nullptr)
		sfcToUse = weakSurface;
	else
		sfcToUse = strongSurface;

	if (ctxToUse!=nullptr && sfcToUse!=nullptr)	{
		if (ctxToUse != current)	{
			ctxToUse->makeCurrent(sfcToUse);
		}
	}
	else	{
		qDebug() << "ERR: " << __PRETTY_FUNCTION__ << " ctx (" << ctxToUse << ") or surface (" << sfcToUse << ") was nil";
	}
}






/*	========================================	*/
#pragma mark --------------------- wrapper class




QSurfaceFormat GLQtCtxWrapper::CreateDefaultSurfaceFormat()	{
	QSurfaceFormat		fmt = QSurfaceFormat::defaultFormat();
	
	//fmt.setAlphaBufferSize(8);
	//fmt.setDepthBufferSize(16);
	//fmt.setRedBufferSize(8);
	//fmt.setGreenBufferSize(8);
	//fmt.setBlueBufferSize(8);
	//fmt.setOptions(0);
	//fmt.setProfile(QSurfaceFormat::CoreProfile);
	fmt.setRenderableType(QSurfaceFormat::OpenGL);
	fmt.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
	fmt.setSwapInterval(1);
	//fmt.setVersion(4,1);
	
	return fmt;
}
QSurfaceFormat GLQtCtxWrapper::CreateCompatibilityGLSurfaceFormat()	{
	QSurfaceFormat		fmt = QSurfaceFormat::defaultFormat();
	
	//fmt.setAlphaBufferSize(8);
	//fmt.setDepthBufferSize(16);
	//fmt.setRedBufferSize(8);
	//fmt.setGreenBufferSize(8);
	//fmt.setBlueBufferSize(8);
	fmt.setOptions(0);
	fmt.setProfile(QSurfaceFormat::CompatibilityProfile);
	fmt.setRenderableType(QSurfaceFormat::OpenGL);
	fmt.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
	fmt.setSwapInterval(1);
	fmt.setVersion(4,1);
	
	return fmt;
}
QSurfaceFormat GLQtCtxWrapper::CreateGL3SurfaceFormat()	{
	QSurfaceFormat		fmt = QSurfaceFormat::defaultFormat();
	
	//fmt.setAlphaBufferSize(8);
	//fmt.setDepthBufferSize(16);
	//fmt.setRedBufferSize(8);
	//fmt.setGreenBufferSize(8);
	//fmt.setBlueBufferSize(8);
	fmt.setOptions(0);
	fmt.setProfile(QSurfaceFormat::CoreProfile);
	fmt.setRenderableType(QSurfaceFormat::OpenGL);
	fmt.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
	fmt.setSwapInterval(1);
	fmt.setVersion(3,0);
	
	return fmt;
}
QSurfaceFormat GLQtCtxWrapper::CreateGL4SurfaceFormat()	{
	QSurfaceFormat		fmt = QSurfaceFormat::defaultFormat();
	
	//fmt.setAlphaBufferSize(8);
	//fmt.setDepthBufferSize(16);
	//fmt.setRedBufferSize(8);
	//fmt.setGreenBufferSize(8);
	//fmt.setBlueBufferSize(8);
	fmt.setOptions(0);
	fmt.setProfile(QSurfaceFormat::CoreProfile);
	fmt.setRenderableType(QSurfaceFormat::OpenGL);
	fmt.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
	fmt.setSwapInterval(1);
	fmt.setVersion(4,0);
	
	return fmt;
}
QOpenGLContext * GLQtCtxWrapper::GetCurrentContext()	{
	return QOpenGLContext::currentContext();
}


//	creates a new GL context and surface which it "owns", like calling GLQtCtxWrapper(nullptr,nullptr,true);
GLQtCtxWrapper::GLQtCtxWrapper()	{
	hidden = new GLQtCtxHidden();
}
//	if 'inTargetSurface' is null, a QOffscreenSurface will be created as a child of the QOpenGLContext.  if it's non-null (a widget or a window or etc), we just get a weak ref to it.
//	if 'inCreateCtx' is YES, a new GL context will be created and it will share 'inCtx'.
//	if 'inCreateCtx' is NO, no GL context will be created and instead a weak ref to 'inCtx' will be established.
GLQtCtxWrapper::GLQtCtxWrapper(QSurface * inSurface, QOpenGLContext * inCtx, bool inCreateCtx, QSurfaceFormat inSfcFmt)	{
	hidden = new GLQtCtxHidden(inSurface, inCtx, inCreateCtx, inSfcFmt);
}


GLQtCtxWrapper::~GLQtCtxWrapper()	{
	if (hidden != nullptr)	{
		delete hidden;
		hidden = nullptr;
	}
}


GLQtCtxWrapper::GLQtCtxWrapper(const GLQtCtxWrapper * n)	{
	if (hidden != nullptr)
		delete hidden;
	if (n == nullptr)
		hidden = new GLQtCtxHidden();
	else
		hidden = new GLQtCtxHidden(n->hidden);
}
GLQtCtxWrapper::GLQtCtxWrapper(const GLQtCtxWrapper & n)	{
	if (hidden != nullptr)
		delete hidden;
	hidden = new GLQtCtxHidden(n.hidden);
}
GLQtCtxWrapper::GLQtCtxWrapper(const GLQtCtxWrapperRef & n)	{
	if (hidden != nullptr)
		delete hidden;
	if (n == nullptr)
		hidden = new GLQtCtxHidden();
	else
		hidden = new GLQtCtxHidden(n->hidden);
}


void GLQtCtxWrapper::setShareContext(QOpenGLContext * inCtx)	{
	if (hidden == nullptr)
		return;
	hidden->setShareContext(inCtx);
}
QOpenGLContext * GLQtCtxWrapper::getContext()	{
	if (hidden==nullptr)
		return nullptr;
	return hidden->getContext();
}
bool GLQtCtxWrapper::isSharingWith(QOpenGLContext * inCtx)	{
	if (hidden==nullptr)
		return false;
	return hidden->isSharingWith(inCtx);
}

void GLQtCtxWrapper::setSurface(const QSurface * inTargetSurface)	{
	if (hidden == nullptr)
		return;
	hidden->setSurface(inTargetSurface);
}
void GLQtCtxWrapper::swap()	{
	if (hidden == nullptr)
		return;
	hidden->swap();
}
void GLQtCtxWrapper::moveToThread(QThread * inThread)	{
	if (hidden == nullptr)
		return;
	hidden->moveToThread(inThread);
}

void GLQtCtxWrapper::makeCurrent()	{
	if (hidden == nullptr)
		return;
	hidden->makeCurrent();
}
void GLQtCtxWrapper::makeCurrentIfNotCurrent()	{
	if (hidden == nullptr)
		return;
	hidden->makeCurrentIfNotCurrent();
}
void GLQtCtxWrapper::makeCurrentIfNull()	{
	if (hidden == nullptr)
		return;
	hidden->makeCurrentIfNull();
}




}

#endif	//	VVGL_SDK_QT
