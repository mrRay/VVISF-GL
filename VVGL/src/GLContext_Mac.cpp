//	this source file is included programmatically in GLContext.cpp, so we don't need to include a header for GLContext here

//	makes absolutely sure that the following is only compiled if we're using the given SDK
#if defined(VVGL_SDK_MAC)

#import <CoreGraphics/CoreGraphics.h>

#include <iostream>
//#include <cassert>
#include <regex>




namespace VVGL
{


using namespace std;




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
		kCGLPFAAccelerated,
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
		kCGLPFAAccelerated,
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
		kCGLPFAAccelerated,
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




}




#endif	//	VVGL_SDK_MAC
