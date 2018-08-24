//	this source file is included programmatically in GLContext.cpp, so we don't need to include a header for GLContext here

//	makes absolutely sure that the following is only compiled if we're using the given SDK
#if defined(VVGL_SDK_QT)

#include <QDebug>

#include <iostream>
//#include <cassert>
#include <regex>




namespace VVGL
{


using namespace std;



/*	========================================	*/
#pragma mark --------------------- non-class functions


QSurfaceFormat CreateDefaultSurfaceFormat()	{
	return GLQtCtxWrapper::CreateDefaultSurfaceFormat();
}
QSurfaceFormat CreateCompatibilityGLSurfaceFormat()	{
	return GLQtCtxWrapper::CreateCompatibilityGLSurfaceFormat();
}
QSurfaceFormat CreateGL3SurfaceFormat()	{
	return GLQtCtxWrapper::CreateGL3SurfaceFormat();
}
QSurfaceFormat CreateGL4SurfaceFormat()	{
	return GLQtCtxWrapper::CreateGL4SurfaceFormat();
}


/*	========================================	*/
#pragma mark --------------------- static class functions


QOpenGLContext * GLContext::GetCurrentContext()	{
	return GLQtCtxWrapper::GetCurrentContext();
}


/*	========================================	*/
#pragma mark --------------------- constructor/destructor


//	if 'inTargetSurface' is null, a QOffscreenSurface will be created.  if it's non-null (a widget or a window or etc), we just get a weak ref to it.
//	if 'inCreateCtx' is YES, a new GL context will be created and it will share 'inCtx'.
//	if 'inCreateCtx' is NO, no GL context will be created and instead a weak ref to 'inCtx' will be established.
GLContext::GLContext(QSurface * inTargetSurface, QOpenGLContext * inCtx, bool inCreateCtx, QSurfaceFormat inSfcFmt)	{
	//qDebug() << __PRETTY_FUNCTION__;
	if (ctx != nullptr)
		delete ctx;
	ctx = new GLQtCtxWrapper(inTargetSurface, inCtx, inCreateCtx, inSfcFmt);
	initializedFuncs = false;
	sfcFmt = inSfcFmt;
	generalInit();
}
GLContext::GLContext(QOpenGLContext * inCtx)	{
	if (ctx != nullptr)
		delete ctx;
	ctx = new GLQtCtxWrapper(inCtx);
	initializedFuncs = false;
	sfcFmt = ctx->format();
	generalInit();
}
GLContext::GLContext()	{
	//qDebug() << __PRETTY_FUNCTION__;
	if (ctx != nullptr)
		delete ctx;
	ctx = new GLQtCtxWrapper(nullptr,nullptr,true,CreateDefaultSurfaceFormat());
	initializedFuncs = false;
	sfcFmt = CreateDefaultSurfaceFormat();
	generalInit();
}
GLContext::~GLContext()	{
	//cout << __PRETTY_FUNCTION__ << endl;
	if (ctx != nullptr)	{
		delete ctx;
		ctx = nullptr;
		initializedFuncs = false;
	}
}


/*	========================================	*/
#pragma mark --------------------- factory method

/*
GLContext * GLContext::allocNewContextSharingMe() const	{
	return new GLContext(display, winSurface, ctx);
}
GLContext GLContext::newContextSharingMe() const	{
	return GLContext(display, winSurface, ctx);
}
*/
GLContextRef GLContext::newContextSharingMe() const	{
	/*
	return make_shared<GLContext>(display, winSurface, ctx);
	*/
	if (ctx == nullptr)
		return nullptr;
	return make_shared<GLContext>(nullptr, ctx->getContext(), true, sfcFmt);
}

/*	========================================	*/
#pragma mark --------------------- general init/delete

void GLContext::generalInit()	{
	//cout << __PRETTY_FUNCTION__ << endl;
	//qDebug() << __PRETTY_FUNCTION__ << ctx->getContext();
	/*
	if (display != nullptr)	{
		eglSwapInterval(display, 0);
	}
	*/
	//	figure out what version of GL we're working with
	if (ctx != nullptr)	{
		ctx->makeCurrentIfNotCurrent();
		calculateVersion();
	}
}

/*	========================================	*/
#pragma mark --------------------- public methods

void GLContext::setSurface(const QSurface * inTargetSurface)	{
	if (ctx != nullptr)	{
		ctx->setSurface(inTargetSurface);
	}
}
void GLContext::swap()	{
	if (ctx != nullptr)	{
		ctx->swap();
	}
}
void GLContext::moveToThread(QThread * inThread)	{
	if (ctx != nullptr)	{
		ctx->moveToThread(inThread);
		initializedFuncs = false;
	}
}
QOpenGLContext * GLContext::getContext()	{
	QOpenGLContext		*returnMe = nullptr;
	if (ctx != nullptr)	{
		returnMe = ctx->getContext();
	}
	return returnMe;
}
void GLContext::makeCurrent()	{
	//cout << __PRETTY_FUNCTION__ << ", ctx is " << ctx << endl;
	if (ctx != nullptr)	{
		ctx->makeCurrent();
		if (!initializedFuncs)	{
			GLenum			err = glewInit();
			if (err != GLEW_OK)
				cout << "\tERR: failed to initialize GLEW\n";
			initializedFuncs = true;
		}
	}
}
void GLContext::makeCurrentIfNotCurrent()	{
	//cout << __PRETTY_FUNCTION__ << ", ctx is " << ctx << endl;
	if (ctx != nullptr)	{
		ctx->makeCurrentIfNotCurrent();
		if (!initializedFuncs)	{
			GLenum			err = glewInit();
			if (err != GLEW_OK)
				cout << "\tERR: failed to initialize GLEW\n";
			initializedFuncs = true;
		}
	}
}
void GLContext::makeCurrentIfNull()	{
	//cout << __PRETTY_FUNCTION__ << ", ctx is " << ctx << endl;
	if (ctx != nullptr)	{
		ctx->makeCurrentIfNull();
		if (!initializedFuncs)	{
			GLenum			err = glewInit();
			if (err != GLEW_OK)
				cout << "\tERR: failed to initialize GLEW\n";
			initializedFuncs = true;
		}
	}
}
bool GLContext::sameShareGroupAs(const GLContextRef & inCtx)	{
	//cout << "ERR: undefined behavior, " << __PRETTY_FUNCTION__ << endl;
	if (ctx == nullptr)
		return false;
	if (inCtx == nullptr)
		return false;
	GLQtCtxWrapper		*otherCtx = inCtx->ctx;
	if (otherCtx == nullptr)
		return false;
	return ctx->isSharingWith(otherCtx->getContext());
}
GLContext & GLContext::operator=(const GLContext & /*n*/)	{
	/*
	display = n.display;
	winSurface = n.winSurface;
	ctx = n.ctx;
	*/
	return *this;
}
ostream & operator<<(ostream & os, const GLContext & n)	{
	os << &n;
	return os;
}
ostream & operator<<(ostream & os, const GLContext * n)	{
	//os << "<GLContext " << (void *)n << ">";
	os << "<GLContext " << static_cast<void*>(const_cast<GLContext*>(n)) << ">";
	return os;
}




}




#endif	//	VVGL_SDK_QT
