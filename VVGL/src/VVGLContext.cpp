#include "VVGLContext.hpp"

#if ISF_SDK_MAC
	#import <CoreGraphics/CoreGraphics.h>
#endif

#if ISF_SDK_QT
	#include <QDebug>
#endif

#include <iostream>
#include <cassert>
#include <regex>




namespace VVGL
{


using namespace std;




#if ISF_SDK_MAC
#pragma mark ******************************************** ISF_SDK_MAC




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

VVGLContext::VVGLContext(const CGLContextObj & inCtx, const CGLContextObj & inShareCtx, const CGLPixelFormatObj & inPxlFmt)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	
	sharedCtx = (inShareCtx==nullptr) ? nullptr : CGLRetainContext(inShareCtx);
	pxlFmt = (inPxlFmt==nullptr) ? nullptr : CGLRetainPixelFormat(inPxlFmt);
	ctx = (inCtx==nullptr) ? nullptr : CGLRetainContext(inCtx);
	
	generalInit();
}
VVGLContext::VVGLContext(const CGLContextObj & inShareCtx, const CGLPixelFormatObj & inPxlFmt)	{
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
VVGLContext::VVGLContext()	{
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
VVGLContext::~VVGLContext()	{
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
#pragma mark --------------------- copy constructor

//	copy constructors DO NOT CREATE NEW GL CONTEXTS.  they only retain the contexts they were passed.
VVGLContext::VVGLContext(const VVGLContext * n)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	
	sharedCtx = (n->sharedCtx==nullptr) ? nullptr : CGLRetainContext(n->sharedCtx);
	pxlFmt = (n->pxlFmt==nullptr) ? nullptr : CGLRetainPixelFormat(n->pxlFmt);
	ctx = (n->ctx==nullptr) ? nullptr : CGLRetainContext(n->ctx);
	
	generalInit();
}
VVGLContext::VVGLContext(const VVGLContext & n)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	
	sharedCtx = (n.sharedCtx==nullptr) ? nullptr : CGLRetainContext(n.sharedCtx);
	pxlFmt = (n.pxlFmt==nullptr) ? nullptr : CGLRetainPixelFormat(n.pxlFmt);
	ctx = (n.ctx==nullptr) ? nullptr : CGLRetainContext(n.ctx);
	
	generalInit();
}
VVGLContext::VVGLContext(const VVGLContextRef & n)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	
	sharedCtx = (n->sharedCtx==nullptr) ? nullptr : CGLRetainContext(n->sharedCtx);
	pxlFmt = (n->pxlFmt==nullptr) ? nullptr : CGLRetainPixelFormat(n->pxlFmt);
	ctx = (n->ctx==nullptr) ? nullptr : CGLRetainContext(n->ctx);
	
	generalInit();
}

/*	========================================	*/
#pragma mark --------------------- factory method

/*
VVGLContext * VVGLContext::allocNewContextSharingMe() const	{
	if (sharedCtx == nullptr)
		return new VVGLContext(ctx, pxlFmt);
	else
		return new VVGLContext(sharedCtx, pxlFmt);
}
VVGLContext VVGLContext::newContextSharingMe() const	{
	if (sharedCtx == nullptr)
		return VVGLContext(ctx, pxlFmt);
	else
		return VVGLContext(sharedCtx, pxlFmt);
}
*/
VVGLContextRef VVGLContext::newContextSharingMe() const	{
	if (sharedCtx == nullptr)
		return make_shared<VVGLContext>(ctx, pxlFmt);
	else
		return make_shared<VVGLContext>(sharedCtx, pxlFmt);
}

/*	========================================	*/
#pragma mark --------------------- general init/delete

void VVGLContext::generalInit()	{
	if (ctx != nullptr)	{
		const int32_t		swap = 0;
		CGLSetParameter(ctx, kCGLCPSwapInterval, &swap);
	}
	
	//	figure out what version of GL we're working with
	calculateVersion();
}

/*	========================================	*/
#pragma mark --------------------- public methods

void VVGLContext::makeCurrent()	{
	//cout << __PRETTY_FUNCTION__ << ", ctx is " << ctx << endl;
	if (ctx != nullptr)
		CGLSetCurrentContext(ctx);
}
void VVGLContext::makeCurrentIfNotCurrent()	{
	//cout << __PRETTY_FUNCTION__ << ", ctx is " << ctx << endl;
	if (ctx != nullptr)	{
		CGLContextObj		orig_ctx = CGLGetCurrentContext();
		if (orig_ctx != ctx)	{
			//cout << "\tchanging current context!\n";
			CGLSetCurrentContext(ctx);
		}
	}
}
void VVGLContext::makeCurrentIfNull()	{
	//cout << __PRETTY_FUNCTION__ << ", ctx is " << ctx << endl;
	if (ctx != nullptr)	{
		CGLContextObj		orig_ctx = CGLGetCurrentContext();
		if (orig_ctx == nullptr)	{
			//cout << "\tsetting current context\n";
			CGLSetCurrentContext(ctx);
		}
	}
}
bool VVGLContext::sameShareGroupAs(const VVGLContextRef & inCtx)	{
	CGLContextObj		inCtxCtx = (inCtx==nullptr) ? nullptr : inCtx->ctx;
	CGLShareGroupObj	inShareGroup = (inCtxCtx==nullptr) ? NULL : CGLGetShareGroup(inCtxCtx);
	CGLShareGroupObj	myShareGroup = (ctx==nullptr) ? NULL : CGLGetShareGroup(ctx);
	if (inShareGroup!=NULL && myShareGroup!=NULL && inShareGroup==myShareGroup)
		return true;
	return false;
}
bool VVGLContext::sameShareGroupAs(const CGLContextObj & inCtx)	{
	CGLShareGroupObj	inShareGroup = (inCtx==nullptr) ? NULL : CGLGetShareGroup(inCtx);
	CGLShareGroupObj	myShareGroup = (ctx==nullptr) ? NULL : CGLGetShareGroup(ctx);
	if (inShareGroup!=NULL && myShareGroup!=NULL && inShareGroup==myShareGroup)
		return true;
	return false;
}
VVGLContext & VVGLContext::operator=(const VVGLContext & n)	{
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
ostream & operator<<(ostream & os, const VVGLContext & n)	{
	os << &n;
	return os;
}
ostream & operator<<(ostream & os, const VVGLContext * n)	{
	//os << "<VVGLContext " << (void *)n << ">";
	//os << "<VVGLContext " << n->ctx << "/" << n->sharedCtx << ">";
	os << "<VVGLContext " << n->ctx << ">";
	return os;
}




#pragma mark ******************************************** ISF_SDK_GLFW
#elif ISF_SDK_GLFW




/*	========================================	*/
#pragma mark --------------------- constructor/destructor

VVGLContext::VVGLContext(GLFWwindow * inWindow)	{
	win = inWindow;
	generalInit();
}
VVGLContext::VVGLContext()	{
	generalInit();
}
VVGLContext::~VVGLContext()	{
	win = nullptr;
}

/*	========================================	*/
#pragma mark --------------------- copy constructor

//	copy constructors DO NOT CREATE NEW GL CONTEXTS.  they only retain the contexts they were passed.
VVGLContext::VVGLContext(const VVGLContext * n)	{
	qDebug() << __PRETTY_FUNCTION__;
	win = n->win;
	generalInit();
}
VVGLContext::VVGLContext(const VVGLContext & n)	{
	qDebug() << __PRETTY_FUNCTION__;
	win = n.win;
	generalInit();
}
VVGLContext::VVGLContext(const VVGLContextRef & n)	{
	qDebug() << __PRETTY_FUNCTION__;
	win = n->win;
	generalInit();
}

/*	========================================	*/
#pragma mark --------------------- factory method

/*
VVGLContext * VVGLContext::allocNewContextSharingMe() const	{
	return new VVGLContext(win);
}
VVGLContext VVGLContext::newContextSharingMe() const	{
	return VVGLContext(win);
}
*/
VVGLContextRef VVGLContext::newContextSharingMe() const	{
	return make_shared<VVGLContext>(win);
}

/*	========================================	*/
#pragma mark --------------------- general init/delete

void VVGLContext::generalInit()	{
	if (win != nullptr)	{
	}
	
	//	figure out what version of GL we're working with
	calculateVersion();
}

/*	========================================	*/
#pragma mark --------------------- public methods

void VVGLContext::makeCurrent()	{
	//cout << __PRETTY_FUNCTION__ << ", ctx is " << ctx << endl;
	if (win != nullptr)
		glfwMakeContextCurrent(win);
}
void VVGLContext::makeCurrentIfNotCurrent()	{
	//cout << __PRETTY_FUNCTION__ << ", ctx is " << ctx << endl;
	GLFWwindow		*currentCtx = glfwGetCurrentContext();
	if (currentCtx != win)
		glfwMakeContextCurrent(win);
}
void VVGLContext::makeCurrentIfNull()	{
	//cout << __PRETTY_FUNCTION__ << ", ctx is " << ctx << endl;
	GLFWwindow		*currentCtx = glfwGetCurrentContext();
	if (currentCtx == nullptr)
		glfwMakeContextCurrent(win);
}
bool VVGLContext::sameShareGroupAs(const VVGLContextRef & inCtx)	{
	cout << "ERR: undefined behavior, " << __PRETTY_FUNCTION__ << endl;
	return false;
}
VVGLContext & VVGLContext::operator=(const VVGLContext & n)	{
	win = n.win;
	return *this;
}
ostream & operator<<(ostream & os, const VVGLContext & n)	{
	os << &n;
	return os;
}
ostream & operator<<(ostream & os, const VVGLContext * n)	{
	//os << "<VVGLContext " << (void *)n << ">";
	os << "<VVGLContext " << n->win << ">";
	return os;
}




#pragma mark ******************************************** ISF_SDK_RPI
#elif ISF_SDK_RPI




/*	========================================	*/
#pragma mark --------------------- constructor/destructor


VVGLContext::VVGLContext(EGLDisplay inDisplay, EGLSurface inWinSurface, EGLContext inSharedCtx, EGLContext inCtx)	{
	display = inDisplay;
	winSurface = inWinSurface;
	sharedCtx = inSharedCtx;
	ctx = inCtx;
	generalInit();
}
VVGLContext::VVGLContext(EGLDisplay inDisplay, EGLSurface inWinSurface, EGLContext inSharedCtx)	{
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
VVGLContext::VVGLContext()	{
	generalInit();
}
VVGLContext::~VVGLContext()	{
	if (ownsTheCtx && ctx != EGL_NO_CONTEXT)	{
		eglDestroyContext(display, ctx);
	}
	display = EGL_NO_DISPLAY;
	winSurface = EGL_NO_SURFACE;
	ctx = EGL_NO_CONTEXT;
}

/*	========================================	*/
#pragma mark --------------------- copy constructor

//	copy constructors DO NOT CREATE NEW GL CONTEXTS.  they only retain the contexts they were passed.
VVGLContext::VVGLContext(const VVGLContext * n)	{
	display = n->display;
	winSurface = n->winSurface;
	ctx = n->ctx;
	generalInit();
}
VVGLContext::VVGLContext(const VVGLContext & n)	{
	display = n.display;
	winSurface = n.winSurface;
	ctx = n.ctx;
	generalInit();
}
VVGLContext::VVGLContext(const VVGLContextRef & n)	{
	display = n->display;
	winSurface = n->winSurface;
	ctx = n->ctx;
	generalInit();
}

/*	========================================	*/
#pragma mark --------------------- factory method

/*
VVGLContext * VVGLContext::allocNewContextSharingMe() const	{
	return new VVGLContext(display, winSurface, ctx);
}
VVGLContext VVGLContext::newContextSharingMe() const	{
	return VVGLContext(display, winSurface, ctx);
}
*/
VVGLContextRef VVGLContext::newContextSharingMe() const	{
	return make_shared<VVGLContext>(display, winSurface, ctx);
}

/*	========================================	*/
#pragma mark --------------------- general init/delete

void VVGLContext::generalInit()	{
	if (display != nullptr)	{
		eglSwapInterval(display, 0);
	}
	
	//	figure out what version of GL we're working with
	calculateVersion();
}

/*	========================================	*/
#pragma mark --------------------- public methods

void VVGLContext::makeCurrent()	{
	//cout << __PRETTY_FUNCTION__ << ", ctx is " << ctx << endl;
	if (display != nullptr && winSurface != nullptr && ctx != nullptr)
		eglMakeCurrent(display, winSurface, winSurface, ctx);
}
void VVGLContext::makeCurrentIfNotCurrent()	{
	//cout << __PRETTY_FUNCTION__ << ", ctx is " << ctx << endl;
	EGLContext		currentCtx = eglGetCurrentContext();
	if (currentCtx != ctx)	{
		if (display != nullptr && winSurface != nullptr && ctx != nullptr)	{
			eglMakeCurrent(display, winSurface, winSurface, ctx);
		}
	}
}
void VVGLContext::makeCurrentIfNull()	{
	//cout << __PRETTY_FUNCTION__ << ", ctx is " << ctx << endl;
	makeCurrent();
}
bool VVGLContext::sameShareGroupAs(const VVGLContextRef & inCtx)	{
	cout << "ERR: undefined behavior, " << __PRETTY_FUNCTION__ << endl;
	return false;
}
VVGLContext & VVGLContext::operator=(const VVGLContext & n)	{
	display = n.display;
	winSurface = n.winSurface;
	ctx = n.ctx;
	return *this;
}
ostream & operator<<(ostream & os, const VVGLContext & n)	{
	os << &n;
	return os;
}
ostream & operator<<(ostream & os, const VVGLContext * n)	{
	//os << "<VVGLContext " << (void *)n << ">";
	os << "<VVGLContext " << n << ">";
	return os;
}



#pragma mark ******************************************** ISF_SDK_QT
#elif ISF_SDK_QT



/*	========================================	*/
#pragma mark --------------------- non-class functions


QSurfaceFormat CreateDefaultSurfaceFormat()	{
	return VVGLQtCtxWrapper::CreateDefaultSurfaceFormat();
}
QSurfaceFormat CreateCompatibilityGLSurfaceFormat()	{
	return VVGLQtCtxWrapper::CreateCompatibilityGLSurfaceFormat();
}
QSurfaceFormat CreateGL3SurfaceFormat()	{
	return VVGLQtCtxWrapper::CreateGL3SurfaceFormat();
}
QSurfaceFormat CreateGL4SurfaceFormat()	{
	return VVGLQtCtxWrapper::CreateGL4SurfaceFormat();
}


/*	========================================	*/
#pragma mark --------------------- static class functions


QOpenGLContext * VVGLContext::GetCurrentContext()	{
	return VVGLQtCtxWrapper::GetCurrentContext();
}


/*	========================================	*/
#pragma mark --------------------- constructor/destructor


//	if 'inTargetSurface' is null, a QOffscreenSurface will be created.  if it's non-null (a widget or a window or etc), we just get a weak ref to it.
//	if 'inCreateCtx' is YES, a new GL context will be created and it will share 'inCtx'.
//	if 'inCreateCtx' is NO, no GL context will be created and instead a weak ref to 'inCtx' will be established.
VVGLContext::VVGLContext(QSurface * inTargetSurface, QOpenGLContext * inCtx, bool inCreateCtx, QSurfaceFormat inSfcFmt)	{
	//qDebug() << __PRETTY_FUNCTION__;
	if (ctx != nullptr)
		delete ctx;
	ctx = new VVGLQtCtxWrapper(inTargetSurface, inCtx, inCreateCtx, inSfcFmt);
	initializedFuncs = false;
	sfcFmt = inSfcFmt;
	generalInit();
}
VVGLContext::VVGLContext()	{
	//qDebug() << __PRETTY_FUNCTION__;
	if (ctx != nullptr)
		delete ctx;
	ctx = new VVGLQtCtxWrapper(nullptr,nullptr,true,CreateDefaultSurfaceFormat());
	initializedFuncs = false;
	sfcFmt = CreateDefaultSurfaceFormat();
	generalInit();
}
VVGLContext::~VVGLContext()	{
	//cout << __PRETTY_FUNCTION__ << endl;
	if (ctx != nullptr)	{
		delete ctx;
		ctx = nullptr;
		initializedFuncs = false;
	}
}


/*	========================================	*/
#pragma mark --------------------- copy constructor

//	copy constructors DO NOT CREATE NEW GL CONTEXTS.  they only retain the contexts they were passed.
VVGLContext::VVGLContext(const VVGLContext * n)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	if (ctx != nullptr)
		delete ctx;
	if (n == nullptr)
		ctx = new VVGLQtCtxWrapper();
	else
		ctx = new VVGLQtCtxWrapper(n->ctx);
	initializedFuncs = false;
	generalInit();
}
VVGLContext::VVGLContext(const VVGLContext & n)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	if (ctx != nullptr)
		delete ctx;
	ctx = new VVGLQtCtxWrapper(n.ctx);
	initializedFuncs = false;
	generalInit();
}
VVGLContext::VVGLContext(const VVGLContextRef & n)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	if (ctx != nullptr)
		delete ctx;
	if (n == nullptr)
		ctx = new VVGLQtCtxWrapper();
	else
		ctx = new VVGLQtCtxWrapper(n->ctx);
	initializedFuncs = false;
	generalInit();
}


/*	========================================	*/
#pragma mark --------------------- factory method

/*
VVGLContext * VVGLContext::allocNewContextSharingMe() const	{
	return new VVGLContext(display, winSurface, ctx);
}
VVGLContext VVGLContext::newContextSharingMe() const	{
	return VVGLContext(display, winSurface, ctx);
}
*/
VVGLContextRef VVGLContext::newContextSharingMe() const	{
	/*
	return make_shared<VVGLContext>(display, winSurface, ctx);
	*/
	if (ctx == nullptr)
		return nullptr;
	return make_shared<VVGLContext>(nullptr, ctx->getContext(), true, sfcFmt);
}

/*	========================================	*/
#pragma mark --------------------- general init/delete

void VVGLContext::generalInit()	{
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

void VVGLContext::setSurface(const QSurface * inTargetSurface)	{
	if (ctx != nullptr)	{
		ctx->setSurface(inTargetSurface);
	}
}
void VVGLContext::swap()	{
	if (ctx != nullptr)	{
		ctx->swap();
	}
}
void VVGLContext::moveToThread(QThread * inThread)	{
	if (ctx != nullptr)	{
		ctx->moveToThread(inThread);
		initializedFuncs = false;
	}
}
QOpenGLContext * VVGLContext::getContext()	{
	QOpenGLContext		*returnMe = nullptr;
	if (ctx != nullptr)	{
		returnMe = ctx->getContext();
	}
	return returnMe;
}
void VVGLContext::makeCurrent()	{
	//cout << __PRETTY_FUNCTION__ << ", ctx is " << ctx << endl;
	if (ctx != nullptr)	{
		ctx->makeCurrent();
		if (!initializedFuncs)	{
			glewInit();
			initializedFuncs = true;
		}
	}
}
void VVGLContext::makeCurrentIfNotCurrent()	{
	//cout << __PRETTY_FUNCTION__ << ", ctx is " << ctx << endl;
	if (ctx != nullptr)	{
		ctx->makeCurrentIfNotCurrent();
		if (!initializedFuncs)	{
			glewInit();
			initializedFuncs = true;
		}
	}
}
void VVGLContext::makeCurrentIfNull()	{
	//cout << __PRETTY_FUNCTION__ << ", ctx is " << ctx << endl;
	if (ctx != nullptr)	{
		ctx->makeCurrentIfNull();
		if (!initializedFuncs)	{
			glewInit();
			initializedFuncs = true;
		}
	}
}
bool VVGLContext::sameShareGroupAs(const VVGLContextRef & inCtx)	{
	//cout << "ERR: undefined behavior, " << __PRETTY_FUNCTION__ << endl;
	if (ctx == nullptr)
		return false;
	if (inCtx == nullptr)
		return false;
	VVGLQtCtxWrapper		*otherCtx = inCtx->ctx;
	if (otherCtx == nullptr)
		return false;
	return ctx->isSharingWith(otherCtx->getContext());
}
VVGLContext & VVGLContext::operator=(const VVGLContext & n)	{
	/*
	display = n.display;
	winSurface = n.winSurface;
	ctx = n.ctx;
	*/
	return *this;
}
ostream & operator<<(ostream & os, const VVGLContext & n)	{
	os << &n;
	return os;
}
ostream & operator<<(ostream & os, const VVGLContext * n)	{
	//os << "<VVGLContext " << (void *)n << ">";
	os << "<VVGLContext " << static_cast<void*>(const_cast<VVGLContext*>(n)) << ">";
	return os;
}




#endif	//	ISF_SDK_QT




#pragma mark ******************************************** COMMON



void VVGLContext::calculateVersion()	{
	//cout << __PRETTY_FUNCTION__ << endl;
	version = GLVersion_Unknown;
	if (ctx == nullptr)
		return;
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
