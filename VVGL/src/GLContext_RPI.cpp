//	this source file is included programmatically in GLContext.cpp, so we don't need to include a header for GLContext here

//	makes absolutely sure that the following is only compiled if we're using the given SDK
#if defined(VVGL_SDK_RPI)

#include <iostream>
#include <cassert>
#include <regex>




namespace VVGL
{


using namespace std;




/*	========================================	*/
#pragma mark --------------------- constructor/destructor


GLContext::GLContext(EGLDisplay inDisplay, EGLSurface inWinSurface, EGLContext inSharedCtx, EGLContext inCtx)	{
	display = inDisplay;
	winSurface = inWinSurface;
	sharedCtx = inSharedCtx;
	ctx = inCtx;
	generalInit();
}
GLContext::GLContext(EGLDisplay inDisplay, EGLSurface inWinSurface, EGLContext inSharedCtx)	{
	display = inDisplay;
	winSurface = inWinSurface;
	sharedCtx = inSharedCtx;
	
	//	choose a display configuration
	EGLBoolean			result;
	EGLConfig			eglConfig;
	const EGLint		targetDispAttribs[] = {
		EGL_RED_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_BLUE_SIZE, 8,
		EGL_ALPHA_SIZE, 8,
		EGL_DEPTH_SIZE, 16,
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_NONE
	};
	EGLint				numConfigs = 0;
	result = eglChooseConfig(display, targetDispAttribs, &eglConfig, 1, &numConfigs);
	assert(result != GL_FALSE);
	assert(eglGetError() == EGL_SUCCESS);
	
	//	create a new context
	const EGLint		targetCtxAttribs[] = {
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};
	ctx = eglCreateContext(display, eglConfig, sharedCtx, targetCtxAttribs);
	assert(ctx != EGL_NO_CONTEXT);
	assert(eglGetError() == EGL_SUCCESS);
	
	//	set this so we know to destroy the context when we get released!
	ownsTheCtx = true;
	
	generalInit();
}
GLContext::GLContext()	{
	generalInit();
}
GLContext::~GLContext()	{
	if (ownsTheCtx && ctx != EGL_NO_CONTEXT)	{
		eglDestroyContext(display, ctx);
	}
	display = EGL_NO_DISPLAY;
	winSurface = EGL_NO_SURFACE;
	ctx = EGL_NO_CONTEXT;
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
	return make_shared<GLContext>(display, winSurface, ctx);
}

/*	========================================	*/
#pragma mark --------------------- general init/delete

void GLContext::generalInit()	{
	if (display != nullptr)	{
		eglSwapInterval(display, 0);
	}
	
	//	figure out what version of GL we're working with
	calculateVersion();
}

/*	========================================	*/
#pragma mark --------------------- public methods

void GLContext::makeCurrent()	{
	//cout << __PRETTY_FUNCTION__ << ", ctx is " << ctx << endl;
	if (display != nullptr && winSurface != nullptr && ctx != nullptr)
		eglMakeCurrent(display, winSurface, winSurface, ctx);
}
void GLContext::makeCurrentIfNotCurrent()	{
	//cout << __PRETTY_FUNCTION__ << ", ctx is " << ctx << endl;
	EGLContext		currentCtx = eglGetCurrentContext();
	if (currentCtx != ctx)	{
		if (display != nullptr && winSurface != nullptr && ctx != nullptr)	{
			eglMakeCurrent(display, winSurface, winSurface, ctx);
		}
	}
}
void GLContext::makeCurrentIfNull()	{
	//cout << __PRETTY_FUNCTION__ << ", ctx is " << ctx << endl;
	makeCurrent();
}
bool GLContext::sameShareGroupAs(const GLContextRef & inCtx)	{
	cout << "ERR: undefined behavior, " << __PRETTY_FUNCTION__ << endl;
	return false;
}
GLContext & GLContext::operator=(const GLContext & n)	{
	display = n.display;
	winSurface = n.winSurface;
	ctx = n.ctx;
	return *this;
}
ostream & operator<<(ostream & os, const GLContext & n)	{
	os << &n;
	return os;
}
ostream & operator<<(ostream & os, const GLContext * n)	{
	//os << "<GLContext " << (void *)n << ">";
	os << "<GLContext " << n << ">";
	return os;
}




}




#endif	//	VVGL_SDK_RPI
