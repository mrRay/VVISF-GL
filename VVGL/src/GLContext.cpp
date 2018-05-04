#include "GLContext.hpp"

#if defined(VVGL_SDK_MAC)
	#import <CoreGraphics/CoreGraphics.h>
#endif

#if defined(VVGL_SDK_QT)
	#include <QDebug>
#endif

#include <iostream>
#include <cassert>
#include <regex>




namespace VVGL
{


using namespace std;




#if defined(VVGL_SDK_MAC)
#pragma mark ******************************************** VVGL_SDK_MAC




/*	========================================	*/
#pragma mark --------------------- non-class functions

uint32_t GLDisplayMaskForAllScreens()	{
	CGError					err = kCGErrorSuccess;
	CGDirectDisplayID		dspys[10];
	CGDisplayCount			count = 0;
	uint32_t				glDisplayMask = 0;
	err = CGGetActiveDisplayList(10,dspys,&count);
	if (err == kCGErrorSuccess)	{
		int					i;
		for (i=0;i<count;++i)
			glDisplayMask = glDisplayMask | CGDisplayIDToOpenGLDisplayMask(dspys[i]);
	}
	return glDisplayMask;
}
CGLPixelFormatObj CreateDefaultPixelFormat()	{
	return CreateCompatibilityGLPixelFormat();
	//std::cout << "defaulting to GL4 instead of GL 2 " << __PRETTY_FUNCTION__ << std::endl;
	//return CreateGL4PixelFormat();
}
CGLPixelFormatObj CreateCompatibilityGLPixelFormat()	{
	CGLPixelFormatAttribute		attribs[] = {
		kCGLPFAAcceleratedCompute,
		kCGLPFAOpenGLProfile, (CGLPixelFormatAttribute)kCGLOGLPVersion_Legacy,
		kCGLPFADisplayMask, (CGLPixelFormatAttribute)GLDisplayMaskForAllScreens(),
		kCGLPFANoRecovery,
		kCGLPFAAllowOfflineRenderers,
		(CGLPixelFormatAttribute)0
	};
	CGLError			cgErr = kCGLNoError;
	int32_t				screenCount = 0;
	CGLPixelFormatObj	returnMe = nullptr;
	cgErr = CGLChoosePixelFormat(attribs, &returnMe, &screenCount);
	if (cgErr != kCGLNoError)
		cout << "\terr: " << cgErr << ", " << __PRETTY_FUNCTION__ << endl;
	return returnMe;
}
CGLPixelFormatObj CreateGL3PixelFormat()	{
	CGLPixelFormatAttribute		attribs[] = {
		kCGLPFAAcceleratedCompute,
		kCGLPFAOpenGLProfile, (CGLPixelFormatAttribute)kCGLOGLPVersion_GL3_Core,
		kCGLPFADisplayMask, (CGLPixelFormatAttribute)GLDisplayMaskForAllScreens(),
		kCGLPFANoRecovery,
		kCGLPFAAllowOfflineRenderers,
		(CGLPixelFormatAttribute)0
	};
	CGLError			cgErr = kCGLNoError;
	int32_t				screenCount = 0;
	CGLPixelFormatObj	returnMe = nullptr;
	cgErr = CGLChoosePixelFormat(attribs, &returnMe, &screenCount);
	if (cgErr != kCGLNoError)
		cout << "\terr: " << cgErr << ", " << __PRETTY_FUNCTION__ << endl;
	return returnMe;
}
CGLPixelFormatObj CreateGL4PixelFormat()	{
	CGLPixelFormatAttribute		attribs[] = {
		kCGLPFAAcceleratedCompute,
		kCGLPFAOpenGLProfile, (CGLPixelFormatAttribute)kCGLOGLPVersion_GL4_Core,
		kCGLPFADisplayMask, (CGLPixelFormatAttribute)GLDisplayMaskForAllScreens(),
		kCGLPFANoRecovery,
		kCGLPFAAllowOfflineRenderers,
		(CGLPixelFormatAttribute)0
	};
	CGLError			cgErr = kCGLNoError;
	int32_t				screenCount = 0;
	CGLPixelFormatObj	returnMe = nullptr;
	cgErr = CGLChoosePixelFormat(attribs, &returnMe, &screenCount);
	if (cgErr != kCGLNoError)
		cout << "\terr: " << cgErr << ", " << __PRETTY_FUNCTION__ << endl;
	return returnMe;
}

/*	========================================	*/
#pragma mark --------------------- constructor/destructor

GLContext::GLContext(const CGLContextObj & inCtx, const CGLContextObj & inShareCtx, const CGLPixelFormatObj & inPxlFmt)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	
	sharedCtx = (inShareCtx==nullptr) ? nullptr : CGLRetainContext(inShareCtx);
	pxlFmt = (inPxlFmt==nullptr) ? nullptr : CGLRetainPixelFormat(inPxlFmt);
	ctx = (inCtx==nullptr) ? nullptr : CGLRetainContext(inCtx);
	
	generalInit();
}
GLContext::GLContext(const CGLContextObj & inShareCtx, const CGLPixelFormatObj & inPxlFmt)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	
	sharedCtx = (inShareCtx==nullptr) ? nullptr : CGLRetainContext(inShareCtx);
	
	pxlFmt = inPxlFmt;
	if (pxlFmt == nullptr)
		pxlFmt = CreateDefaultPixelFormat();
	if (pxlFmt != nullptr)
		pxlFmt = CGLRetainPixelFormat(pxlFmt);
	
	CGLError		cglErr = CGLCreateContext(pxlFmt, sharedCtx, &ctx);
	if (cglErr != kCGLNoError)	{
		cout << "ERR: " << cglErr << " in " << __PRETTY_FUNCTION__ << endl;
	}
	
	generalInit();
}
GLContext::GLContext()	{
	//cout << __PRETTY_FUNCTION__ << endl;
	
	sharedCtx = nullptr;
	pxlFmt = CGLRetainPixelFormat(CreateDefaultPixelFormat());
	//pxlFmt = CGLRetainPixelFormat(CreateGL4PixelFormat());
	
	CGLError		cglErr = CGLCreateContext(pxlFmt, sharedCtx, &ctx);
	if (cglErr != kCGLNoError)	{
		cout << "ERR: " << cglErr << " in " << __PRETTY_FUNCTION__ << endl;
	}
	
	generalInit();
}
GLContext::~GLContext()	{
	//cout << __PRETTY_FUNCTION__ << endl;
	
	if (sharedCtx != nullptr)	{
		CGLReleaseContext(sharedCtx);
		sharedCtx = nullptr;
	}
	if (ctx != nullptr)	{
		CGLReleaseContext(ctx);
		ctx = nullptr;
	}
	if (pxlFmt != nullptr)	{
		CGLReleasePixelFormat(pxlFmt);
		pxlFmt = nullptr;
	}
}

/*	========================================	*/
#pragma mark --------------------- factory method

/*
GLContext * GLContext::allocNewContextSharingMe() const	{
	if (sharedCtx == nullptr)
		return new GLContext(ctx, pxlFmt);
	else
		return new GLContext(sharedCtx, pxlFmt);
}
GLContext GLContext::newContextSharingMe() const	{
	if (sharedCtx == nullptr)
		return GLContext(ctx, pxlFmt);
	else
		return GLContext(sharedCtx, pxlFmt);
}
*/
GLContextRef GLContext::newContextSharingMe() const	{
	if (sharedCtx == nullptr)
		return make_shared<GLContext>(ctx, pxlFmt);
	else
		return make_shared<GLContext>(sharedCtx, pxlFmt);
}

/*	========================================	*/
#pragma mark --------------------- general init/delete

void GLContext::generalInit()	{
	if (ctx != nullptr)	{
		const int32_t		swap = 0;
		CGLSetParameter(ctx, kCGLCPSwapInterval, &swap);
	}
	
	//	figure out what version of GL we're working with
	calculateVersion();
}

/*	========================================	*/
#pragma mark --------------------- public methods

void GLContext::makeCurrent()	{
	//cout << __PRETTY_FUNCTION__ << ", ctx is " << ctx << endl;
	if (ctx != nullptr)
		CGLSetCurrentContext(ctx);
}
void GLContext::makeCurrentIfNotCurrent()	{
	//cout << __PRETTY_FUNCTION__ << ", ctx is " << ctx << endl;
	if (ctx != nullptr)	{
		CGLContextObj		orig_ctx = CGLGetCurrentContext();
		if (orig_ctx != ctx)	{
			//cout << "\tchanging current context!\n";
			CGLSetCurrentContext(ctx);
		}
	}
}
void GLContext::makeCurrentIfNull()	{
	//cout << __PRETTY_FUNCTION__ << ", ctx is " << ctx << endl;
	if (ctx != nullptr)	{
		CGLContextObj		orig_ctx = CGLGetCurrentContext();
		if (orig_ctx == nullptr)	{
			//cout << "\tsetting current context\n";
			CGLSetCurrentContext(ctx);
		}
	}
}
bool GLContext::sameShareGroupAs(const GLContextRef & inCtx)	{
	CGLContextObj		inCtxCtx = (inCtx==nullptr) ? nullptr : inCtx->ctx;
	CGLShareGroupObj	inShareGroup = (inCtxCtx==nullptr) ? NULL : CGLGetShareGroup(inCtxCtx);
	CGLShareGroupObj	myShareGroup = (ctx==nullptr) ? NULL : CGLGetShareGroup(ctx);
	if (inShareGroup!=NULL && myShareGroup!=NULL && inShareGroup==myShareGroup)
		return true;
	return false;
}
bool GLContext::sameShareGroupAs(const CGLContextObj & inCtx)	{
	CGLShareGroupObj	inShareGroup = (inCtx==nullptr) ? NULL : CGLGetShareGroup(inCtx);
	CGLShareGroupObj	myShareGroup = (ctx==nullptr) ? NULL : CGLGetShareGroup(ctx);
	if (inShareGroup!=NULL && myShareGroup!=NULL && inShareGroup==myShareGroup)
		return true;
	return false;
}
GLContext & GLContext::operator=(const GLContext & n)	{
	if (ctx != nullptr)	{
		CGLReleaseContext(ctx);
		ctx = nullptr;
	}
	if (sharedCtx != nullptr)	{
		CGLReleaseContext(sharedCtx);
		sharedCtx = nullptr;
	}
	if (pxlFmt != nullptr)	{
		CGLReleasePixelFormat(pxlFmt);
		pxlFmt = nullptr;
	}
	
	if (n.ctx != nullptr)
		ctx = CGLRetainContext(n.ctx);
	if (n.sharedCtx != nullptr)
		sharedCtx = CGLRetainContext(n.sharedCtx);
	if (n.pxlFmt != nullptr)
		pxlFmt = CGLRetainPixelFormat(n.pxlFmt);
	
	return *this;
}
ostream & operator<<(ostream & os, const GLContext & n)	{
	os << &n;
	return os;
}
ostream & operator<<(ostream & os, const GLContext * n)	{
	//os << "<GLContext " << (void *)n << ">";
	//os << "<GLContext " << n->ctx << "/" << n->sharedCtx << ">";
	os << "<GLContext " << n->ctx << ">";
	return os;
}




#pragma mark ******************************************** VVGL_SDK_GLFW
#elif defined(VVGL_SDK_GLFW)




/*	========================================	*/
#pragma mark --------------------- constructor/destructor

GLContext::GLContext(GLFWwindow * inWindow)	{
	win = inWindow;
	initializedFuncs = false;
	generalInit();
}
GLContext::GLContext()	{
	initializedFuncs = false;
	generalInit();
}
GLContext::~GLContext()	{
	win = nullptr;
	initializedFuncs = false;
}

/*	========================================	*/
#pragma mark --------------------- factory method

/*
GLContext * GLContext::allocNewContextSharingMe() const	{
	return new GLContext(win);
}
GLContext GLContext::newContextSharingMe() const	{
	return GLContext(win);
}
*/
GLContextRef GLContext::newContextSharingMe() const	{
	return make_shared<GLContext>(win);
}

/*	========================================	*/
#pragma mark --------------------- general init/delete

void GLContext::generalInit()	{
	if (win != nullptr)	{
	}
	
	//	figure out what version of GL we're working with
	calculateVersion();
}

/*	========================================	*/
#pragma mark --------------------- public methods

void GLContext::makeCurrent()	{
	//cout << __PRETTY_FUNCTION__ << ", ctx is " << ctx << endl;
	if (win != nullptr)	{
		glfwMakeContextCurrent(win);
		if (!initializedFuncs)	{
			glewInit();
			initializedFuncs = true;
		}
	}
}
void GLContext::makeCurrentIfNotCurrent()	{
	//cout << __PRETTY_FUNCTION__ << ", ctx is " << ctx << endl;
	GLFWwindow		*currentCtx = glfwGetCurrentContext();
	if (currentCtx != win)	{
		glfwMakeContextCurrent(win);
		if (!initializedFuncs)	{
			glewInit();
			initializedFuncs = true;
		}
	}
}
void GLContext::makeCurrentIfNull()	{
	//cout << __PRETTY_FUNCTION__ << ", ctx is " << ctx << endl;
	GLFWwindow		*currentCtx = glfwGetCurrentContext();
	if (currentCtx == nullptr)	{
		glfwMakeContextCurrent(win);
		if (!initializedFuncs)	{
			glewInit();
			initializedFuncs = true;
		}
	}
}
bool GLContext::sameShareGroupAs(const GLContextRef & inCtx)	{
	cout << "ERR: undefined behavior, " << __PRETTY_FUNCTION__ << endl;
	return false;
}
GLContext & GLContext::operator=(const GLContext & n)	{
	win = n.win;
	return *this;
}
ostream & operator<<(ostream & os, const GLContext & n)	{
	os << &n;
	return os;
}
ostream & operator<<(ostream & os, const GLContext * n)	{
	//os << "<GLContext " << (void *)n << ">";
	os << "<GLContext " << n->win << ">";
	return os;
}




#pragma mark ******************************************** VVGL_SDK_RPI
#elif defined(VVGL_SDK_RPI)




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



#pragma mark ******************************************** VVGL_SDK_QT
#elif defined(VVGL_SDK_QT)



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
			glewInit();
			initializedFuncs = true;
		}
	}
}
void GLContext::makeCurrentIfNotCurrent()	{
	//cout << __PRETTY_FUNCTION__ << ", ctx is " << ctx << endl;
	if (ctx != nullptr)	{
		ctx->makeCurrentIfNotCurrent();
		if (!initializedFuncs)	{
			glewInit();
			initializedFuncs = true;
		}
	}
}
void GLContext::makeCurrentIfNull()	{
	//cout << __PRETTY_FUNCTION__ << ", ctx is " << ctx << endl;
	if (ctx != nullptr)	{
		ctx->makeCurrentIfNull();
		if (!initializedFuncs)	{
			glewInit();
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




#endif	//	VVGL_SDK_QT




#pragma mark ******************************************** COMMON



void GLContext::calculateVersion()	{
	//cout << __PRETTY_FUNCTION__ << endl;
	version = GLVersion_Unknown;
#if defined(VVGL_SDK_MAC) || defined(VVGL_SDK_IOS) || defined(VVGL_SDK_RPI) || defined(VVGL_SDK_QT)
	if (ctx == nullptr)
		return;
#elif defined(VVGL_SDK_GLFW)
	if (win == nullptr)
		return;
#endif
	makeCurrentIfNotCurrent();
	const unsigned char			*versString = glGetString(GL_VERSION);
	//cout << "\tversion string is " << versString << endl;
	switch (*versString)	{
	case '2': version = GLVersion_2; break;
	case '3': version = GLVersion_33; break;
	case '4': version = GLVersion_4; break;
	default:
		{
			string			baseString = string((const char *)versString);
			regex			regexJustES("[gG][lL][\\s]*[eE][sS]");
			//	if the base string looks like an OpenGL ES string...
			if (regex_search(baseString, regexJustES))	{
				regex			regexESVsn("[gG][lL][\\s]*[eE][sS][\\s]*([0-9]+)");
				smatch			matches;
				//	if we were able to extract some kind of vsn for GL ES
				if (regex_search(baseString, matches, regexESVsn) && matches.size()>=2)	{
					int				majorVsn = atoi(matches[1].str().c_str());
					switch (majorVsn)	{
					case 2:
						version = GLVersion_ES2;
						break;
					case 3:
						version = GLVersion_ES3;
						break;
					default:
						version = GLVersion_ES2;
						break;
					}
				}
				//	else we weren't able to extract any kind of vsn for GL ES- something's wrong, the full regex isn't matched
				else	{
					//cout << "\terr: matched base string (" << baseString << ") didn't match expanded\n";
					version = GLVersion_ES2;
				}
			}
			//	else the base string doesn't look like a GL ES string
			else	{
				//cout << "\terr: base string (" << baseString << ") not recognized\n";
				//	...i have no idea what goes here, need to test more hardware.  for now, fall back to GL 2.
				version = GLVersion_2;
			}
		}
		break;
	}
}




}
