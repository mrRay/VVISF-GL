#ifndef VVGL_GLContext_hpp
#define VVGL_GLContext_hpp

#include "VVGL_Defines.hpp"

#include <iostream>
#if ISF_SDK_MAC
	#import <OpenGL/OpenGL.h>
	#import <OpenGL/gl.h>
	#import <OpenGL/glext.h>
	#import <OpenGL/gl3.h>
	#import <OpenGL/gl3ext.h>
#elif ISF_SDK_IOS
	//#ifndef __cplusplus
		//#import <OpenGLES/EAGL.h>
		#import <OpenGLES/ES3/glext.h>
		//#import <GLKit/GLKit.h>
	//#endif
#elif ISF_SDK_RPI
	#include "bcm_host.h"
	//#include <GLES/gl.h>
	//#include <GLES/glext.h>
	#include <GLES2/gl2.h>
	#include <GLES2/gl2ext.h>
	#include <EGL/egl.h>
	#include <EGL/eglext.h>
#elif ISF_SDK_GLFW
	#include <glad/glad.h>
	#include <GLFW/glfw3.h>
#elif ISF_SDK_QT
	//#define GLEW_STATIC 1
	#include <GL/glew.h>
	#include <QPointer>
	#include "GLQtCtxWrapper.hpp"
	class QOpenGLContext;
#endif

#include "Base.hpp"




namespace VVGL
{


using namespace std;




#if ISF_SDK_MAC
uint32_t GLDisplayMaskForAllScreens();
CGLPixelFormatObj CreateDefaultPixelFormat();
CGLPixelFormatObj CreateCompatibilityGLPixelFormat();
CGLPixelFormatObj CreateGL3PixelFormat();
CGLPixelFormatObj CreateGL4PixelFormat();
#elif ISF_SDK_QT
QSurfaceFormat CreateDefaultSurfaceFormat();
QSurfaceFormat CreateCompatibilityGLSurfaceFormat();
QSurfaceFormat CreateGL3SurfaceFormat();
QSurfaceFormat CreateGL4SurfaceFormat();
#endif




//	use a shared_ptr to pass around refs to a GLContext w/o copying any GL resources
//class GLContext;
//using GLContextRef = shared_ptr<GLContext>;




/*		GLContext is an attempt to make a platform/SDK-agnostic representation of an OpenGL context.  
this is useful if you want to perform common functions on a GL context (setting the current context, 
making another context in the same sharegroup), but you don't want to have to write any platform-specific code.

if you're porting VVGL to another platform, one of the first things you need to do is to make a 
new ISF_TARGETENV_XXXX macro, and use that macro to add your platform's GL implementation to this class.  
you should be able to following along pretty well here using the other platforms as an example.			*/
class GLContext	{
	public:
		
#if ISF_SDK_MAC
		CGLContextObj		ctx = nullptr;
		CGLContextObj		sharedCtx = nullptr;
		CGLPixelFormatObj	pxlFmt = nullptr;
#elif ISF_SDK_IOS
		void				*ctx = nullptr;	//	really an EAGLContext under iOS
#elif ISF_SDK_GLFW
		GLFWwindow			*win = nullptr;
#elif ISF_SDK_RPI
		EGLDisplay			display = EGL_NO_DISPLAY;	//	weak ref, potentially unsafe
		EGLSurface			winSurface = EGL_NO_SURFACE;	//	weak ref, potentially unsafe
		EGLContext			sharedCtx = EGL_NO_CONTEXT;	//	weak ref, potentially unsafe
		bool				ownsTheCtx = false;	//	set to true when i "own" ctx and must destroy it on my release
		EGLContext			ctx = EGL_NO_CONTEXT;	//	owned by this object
#elif ISF_SDK_QT
		GLQtCtxWrapper	*ctx = nullptr;	//	we have to wrap all the QOpenGL* stuff because if its headers and GLEW's headers are in the same #include paths it breaks compilation
		QSurfaceFormat		sfcFmt = CreateDefaultSurfaceFormat();
		bool				initializedFuncs = false;	//	read some docs that say the GLEW funcs must be initialized once per-context per-thread
#endif
		GLVersion			version = GLVersion_Unknown;
		
		
	public:
#if ISF_SDK_MAC
		//	this function doesn't create anything- it just retains the passed ctx/share ctx/pxl fmt, leaving them null if that's how they were passed in
		GLContext(const CGLContextObj & inCtx, const CGLContextObj & inShareCtx, const CGLPixelFormatObj & inPxlFm=CreateDefaultPixelFormat());
		//	this function creates a context using the passed pixel format and share context
		GLContext(const CGLContextObj & inShareCtx, const CGLPixelFormatObj & inPxlFmt=CreateDefaultPixelFormat());
#elif ISF_SDK_IOS
		//	"inCtx" is an EAGLContext! this function doesn't create anything- it just retains the passed ctx
		GLContext(const void * inEAGLContext);
		//	this function doesn't create a new GL context- it "wraps" (and retains) the passed context
		//GLContext(const EAGLContext * inCtx);
		//	this function actually creates a new GL context using the passed sharegroup and rendering API
		//GLContext(const EAGLSharegroup * inSharegroup, const EAGLRenderingAPI & inRenderAPI);
#elif ISF_SDK_GLFW
		GLContext(GLFWwindow * inWindow);
#elif ISF_SDK_RPI
		//	this function doesn't create anything- it just obtains a weak ref to the passed EGL vars
		GLContext(EGLDisplay inDisplay, EGLSurface inWinSurface, EGLContext inSharedCtx, EGLContext inCtx);
		//	this function creates a new GL context using the passed shared context
		GLContext(EGLDisplay inDisplay, EGLSurface inWinSurface, EGLContext inSharedCtx);
#elif ISF_SDK_QT
		//	if 'inTargetSurface' is null, a QOffscreenSurface will be created.  if it's non-null (a widget or a window or etc), we just get a weak ref to it.
		//	if 'inCreateCtx' is YES, a new GL context will be created and it will share 'inCtx'.
		//	if 'inCreateCtx' is NO, no GL context will be created and instead a weak ref to 'inCtx' will be established.
		GLContext(QSurface * inTargetSurface, QOpenGLContext * inCtx, bool inCreateCtx=true, QSurfaceFormat inSfcFmt=CreateDefaultSurfaceFormat());
		
		static QOpenGLContext * GetCurrentContext();
		
		void setSurface(const QSurface * inTargetSurface);
		void swap();
		void moveToThread(QThread * inThread);
		QOpenGLContext * getContext();
		
		/*
		//	this function doesn't create anything- it just obtains a weak ref to the passed var
		GLContext(const QOpenGLContext * inCtx, const QOpenGLContext * inShareCtx);
		//	this function creates a new QOpenGLContext in the backend sharing the passed context
		GLContext(const QOpenGLContext * inShareCtx);
		*/
#endif
		//	this function creates a context using the default pixel format
		GLContext();
		
		
	public:
		void generalInit();
		void calculateVersion();
		
		//	returned variable MUST BE FREED
		//GLContext * allocNewContextSharingMe() const;
		//	creates a new GL context, but returned variable doesn't have to be freed
		//GLContext newContextSharingMe() const;
		GLContextRef newContextSharingMe() const;
		
		~GLContext();
		
		//	copy constructors- these do NOT create new GL contexts, they merely copy/retain the GL contexts from the passed var
		GLContext(const GLContext * n);
		GLContext(const GLContext & n);
		GLContext(const GLContextRef & n);
		
		void makeCurrent();
		void makeCurrentIfNotCurrent();
		void makeCurrentIfNull();
		
		bool sameShareGroupAs(const GLContextRef & inCtx);
#if ISF_SDK_MAC
		bool sameShareGroupAs(const CGLContextObj & inCtx);
#endif
		
		GLContext & operator=(const GLContext & n);
		friend ostream & operator<<(ostream & os, const GLContext & n);
		friend ostream & operator<<(ostream & os, const GLContext * n);
};




}




#endif /* VVGL_GLContext_hpp */
