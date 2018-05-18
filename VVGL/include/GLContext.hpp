#ifndef VVGL_GLContext_hpp
#define VVGL_GLContext_hpp

#include "VVGL_Defines.hpp"

#include <iostream>
#if defined(VVGL_SDK_MAC)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-W#warnings"
	#import <OpenGL/OpenGL.h>
	#import <OpenGL/gl.h>
	#import <OpenGL/glext.h>
	#import <OpenGL/gl3.h>
	#import <OpenGL/gl3ext.h>
#pragma clang diagnostic pop
#elif defined(VVGL_SDK_IOS)
	//#ifndef __cplusplus
		//#import <OpenGLES/EAGL.h>
		#import <OpenGLES/ES3/glext.h>
		//#import <GLKit/GLKit.h>
	//#endif
#elif defined(VVGL_SDK_RPI)
	#include "bcm_host.h"
	//#include <GLES/gl.h>
	//#include <GLES/glext.h>
	#include <GLES2/gl2.h>
	#include <GLES2/gl2ext.h>
	#include <EGL/egl.h>
	#include <EGL/eglext.h>
#elif defined(VVGL_SDK_GLFW)
	//#include <glad/glad.h>
	#include <GL/glew.h>
	#include <GLFW/glfw3.h>
#elif defined(VVGL_SDK_QT)
	#include <GL/glew.h>
	#include <QPointer>
	#include "GLQtCtxWrapper.hpp"
	class QOpenGLContext;
#endif

#include "VVGL_Base.hpp"




namespace VVGL
{


using namespace std;


#if defined(VVGL_SDK_MAC)
/*!
\relatesalso GLContext
\brief Returns an OpenGL display mask that covers all visible screens
*/
uint32_t GLDisplayMaskForAllScreens();
/*!
\relatesalso GLContext
\brief Creates a pixel format using the system's default OpenGL context settings (compatibility).
*/
CGLPixelFormatObj CreateDefaultPixelFormat();
/*!
\relatesalso GLContext
\brief Creates a pixel format using the system's OpenGL compatibility settings.
*/
CGLPixelFormatObj CreateCompatibilityGLPixelFormat();
/*!
\relatesalso GLContext
\brief Creates a pixel format using the OpenGL 3, if possible.
*/
CGLPixelFormatObj CreateGL3PixelFormat();
/*!
\relatesalso GLContext
\brief Creates a pixel format using the OpenGL 4 profile, if possible.
*/
CGLPixelFormatObj CreateGL4PixelFormat();
#elif defined(VVGL_SDK_QT)
/*!
\relatesalso GLContext
\brief Creates a surface format describing the default OpenGL surface settings for this platform.
*/
VVGL_EXPORT QSurfaceFormat CreateDefaultSurfaceFormat();
/*!
\relatesalso GLContext
\brief Creates a surface format describing the compatibility profile of OpenGL for this platform.
*/
VVGL_EXPORT QSurfaceFormat CreateCompatibilityGLSurfaceFormat();
/*!
\relatesalso GLContext
\brief Creates a surface format describing the OpenGL 3 profile for this platform.
*/
VVGL_EXPORT QSurfaceFormat CreateGL3SurfaceFormat();
/*!
\relatesalso GLContext
\brief Creates a surface format describing the OpenGL 4 profile for this platform.
*/
VVGL_EXPORT QSurfaceFormat CreateGL4SurfaceFormat();
#endif




/*!
\ingroup VVGL_BASIC

\brief GLContext is an attempt to make a platform/SDK-agnostic representation of an OpenGL context.

\detail GLContext is a OpenGL context- this class wraps up whatever the native object is for whatever platform/SDK you're compiling VVGL against, and presents a single standard interface across all platforms for the rest of VVGL and any libs that derive from it.  You can create a GLContext around an existing platform-specific OpenGL context- which makes integration with existing codebases easy- or you can also create new OpenGL contexts by making a new instance of GLContext or using the newContextSharingMe() member function of an existing context.  Has explicit support for sharing of multiple contexts so resources (textures, models, etc) can be shared between contexts.

Notes on use:
- You should strive whenever possible to work with #GLContextRef instead of GLContext.
- If you want to create a GLContext, you should try using one of the non-member creation functions listed on this page.  These functions return a #GLContextRef instead of just a GLContext, and their function names are slightly more verbose and descriptive than GLContext's constructors.
- The specific constructor (or create functions) for creating a GLContext is going to depend on the SDK you're working with, because they generally require some sort of platform- or SDK-specific object or pointer to a native GL context.
*/
class VVGL_EXPORT GLContext	{
	public:
		
#if defined(VVGL_SDK_MAC)
		CGLContextObj		ctx = nullptr;
		CGLContextObj		sharedCtx = nullptr;
		CGLPixelFormatObj	pxlFmt = nullptr;
#elif defined(VVGL_SDK_IOS)
		void				*ctx = nullptr;	//	really an EAGLContext under iOS
#elif defined(VVGL_SDK_GLFW)
		GLFWwindow			*win = nullptr;
		bool				initializedFuncs = false;	//	read some docs that say the GLEW funcs must be initialized once per-context per-thread
#elif defined(VVGL_SDK_RPI)
		EGLDisplay			display = EGL_NO_DISPLAY;	//	weak ref, potentially unsafe
		EGLSurface			winSurface = EGL_NO_SURFACE;	//	weak ref, potentially unsafe
		EGLContext			sharedCtx = EGL_NO_CONTEXT;	//	weak ref, potentially unsafe
		bool				ownsTheCtx = false;	//	set to true when i "own" ctx and must destroy it on my release
		EGLContext			ctx = EGL_NO_CONTEXT;	//	owned by this object
#elif defined(VVGL_SDK_QT)
		GLQtCtxWrapper		*ctx = nullptr;	//	we have to wrap all the QOpenGL* stuff because if its headers and GLEW's headers are in the same #include paths it breaks compilation
		QSurfaceFormat		sfcFmt = CreateDefaultSurfaceFormat();
		bool				initializedFuncs = false;	//	read some docs that say the GLEW funcs must be initialized once per-context per-thread
#endif
		//!	The version of OpenGL this context is using.
		GLVersion			version = GLVersion_Unknown;
		
		
	public:
		//	most uses of GLContext are through shared_ptrs, so you should avoid using these constructors wherever possible- there are creation functions defined in this header outside of this class.
#if defined(VVGL_SDK_MAC)
		//	this function doesn't create anything- it just retains the passed ctx/share ctx/pxl fmt, leaving them null if that's how they were passed in
		GLContext(const CGLContextObj & inCtx, const CGLContextObj & inShareCtx, const CGLPixelFormatObj & inPxlFm=CreateDefaultPixelFormat());
		//	this function creates a context using the passed pixel format and share context
		GLContext(const CGLContextObj & inShareCtx, const CGLPixelFormatObj & inPxlFmt=CreateDefaultPixelFormat());
#elif defined(VVGL_SDK_IOS)
		//	"inCtx" is an EAGLContext! this function doesn't create anything- it just retains the passed ctx
		GLContext(const void * inEAGLContext);
		//	this function doesn't create a new GL context- it "wraps" (and retains) the passed context
		//GLContext(const EAGLContext * inCtx);
		//	this function actually creates a new GL context using the passed sharegroup and rendering API
		//GLContext(const EAGLSharegroup * inSharegroup, const EAGLRenderingAPI & inRenderAPI);
#elif defined(VVGL_SDK_GLFW)
		GLContext(GLFWwindow * inWindow);
#elif defined(VVGL_SDK_RPI)
		//	this function doesn't create anything- it just obtains a weak ref to the passed EGL vars
		GLContext(EGLDisplay inDisplay, EGLSurface inWinSurface, EGLContext inSharedCtx, EGLContext inCtx);
		//	this function creates a new GL context using the passed shared context
		GLContext(EGLDisplay inDisplay, EGLSurface inWinSurface, EGLContext inSharedCtx);
#elif defined(VVGL_SDK_QT)
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
		
		//!	Creates and returns a new OpenGL context in the same sharegroup as the receiver.
		GLContextRef newContextSharingMe() const;
		
		~GLContext();
		
		//!	Makes this GL context current.
		void makeCurrent();
		//!	Makes this GL context current if it isn't already current.
		void makeCurrentIfNotCurrent();
		//!	Makes this GL context current if no context is current.
		void makeCurrentIfNull();
		
		//!	Returns a true if the passed context is in the same sharegroup as the receiver.
		bool sameShareGroupAs(const GLContextRef & inCtx);
#if defined(VVGL_SDK_MAC)
		bool sameShareGroupAs(const CGLContextObj & inCtx);
#endif
		
		GLContext & operator=(const GLContext & n);
		friend ostream & operator<<(ostream & os, const GLContext & n);
		friend ostream & operator<<(ostream & os, const GLContext * n);
};




//	these creation functions are the preferred way of making GLContext instances.  they're just more human-readable than make_shared<GLContext>(constructor args).
#if defined(VVGL_SDK_MAC)
/*!
\relatesalso GLContext
\brief Doesn't create any GL resources, just makes a new GLContext instnace that retains the passed objects.
*/
inline GLContextRef CreateGLContextRefUsing(const CGLContextObj & inCtx, const CGLContextObj & inShareCtx, const CGLPixelFormatObj & inPxlFmt=CreateDefaultPixelFormat()) { return make_shared<GLContext>(inCtx, inShareCtx, inPxlFmt); }
/*!
\relatesalso GLContext
\brief Creates a new GL context using the passed pixel format and share context.
*/
inline GLContextRef CreateNewGLContextRef(const CGLContextObj & inShareCtx, const CGLPixelFormatObj & inPxlFmt=CreateDefaultPixelFormat()) { return make_shared<GLContext>(inShareCtx, inPxlFmt); }
#elif defined(VVGL_SDK_IOS)
/*!
\relatesalso GLContext
\brief Doesn't create any GL resources, just makew a new GLContext instance that retains the passed objects.
*/
inline GLContextRef CreateGLContextRefUsing(const void * inEAGLContext) { return make_shared<GLContext>(inEAGLContext); }
#elif defined(VVGL_SDK_GLFW)
/*!
\relatesalso GLContext
\brief Doesn't create any GL resources, just makes a new GLContext instance around the OpenGL context in the passed window.
*/
inline GLContextRef CreateGLContextRefUsing(GLFWwindow * inWindow) { return make_shared<GLContext>(inWindow); }
#elif defined(VVGL_SDK_RPI)
/*!
\relatesalso GLContext
\brief Doesn't create any GL resources, just makes a new GLContext instance around the passed EGLContext.
*/
inline GLContextRef CreateGLContextRefUsing(EGLDisplay inDisplay, EGLSurface inWinSurface, EGLContext inSharedCtx, EGLContext inCtx) { return make_shared<GLContext>(inDisplay, inWinSurface, inSharedCtx, inCtx); }
/*!
\relatesalso GLContext
\brief Makes a new OpenGL context and GLContext instance.
*/
inline GLContextRef CreateNewGLContextRef(EGLDisplay inDisplay, EGLSurface inWinSurface, EGLContext inSharedCtx) { return make_shared<GLContext>(inDisplay, inWinSurface, inSharedCtx); }
#elif defined(VVGL_SDK_QT)
/*!
\relatesalso GLContext
\brief Doesn't create any GL resources, just makes a new GLContext instance using the passed resources
\param inTargetSurface If null, a QOffscreenSurface will be created.  If non-null (a widget or window or etc), we just store a weak ref to the passed surface.
\param inCtx Must be non-null.  A weak ref is made to this context- the GLContext instance that is created is basically just a wrapper around this context.
\param inSfcFmt The surface format describes what kind of OpenGL environment you want to work with.  The QSurfaceFormat can be created using one of the Create****SurfaceFormat() functions listed in this document.
*/
inline GLContextRef CreateGLContextRefUsing(QSurface * inTargetSurface, QOpenGLContext * inCtx, QSurfaceFormat inSfcFmt=CreateDefaultSurfaceFormat()) { return make_shared<GLContext>(inTargetSurface, inCtx, false, inSfcFmt); }
/*!
\relatesalso GLContext
\brief Creates a new OpenGL context and GLContext instance.
\param inTargetSurface If null, a QOffscreenSurface will be created.  If non-null (a widget or window or etc), we just store a weak ref to the passed surface.
\param inShareCtx The OpenGL context that gets created will be in the same sharegroup as this context.
\param inSfcFmt The surface format describes what kind of OpenGL environment you want to work with.  The QSurfaceFormat can be created using one of the Create****SurfaceFormat() functions listed in this document.
*/
inline GLContextRef CreateNewGLContextRef(QSurface * inTargetSurface, QOpenGLContext * inShareCtx, QSurfaceFormat inSfcFmt=CreateDefaultSurfaceFormat()) { return make_shared<GLContext>(inTargetSurface, inShareCtx, true, inSfcFmt); }
#endif
/*!
\relatesalso GLContext
\brief Creates a generic OpenGL context and GLContext instance using whatever the default settings are for this platform and SDK.
*/
inline GLContextRef CreateNewGLContextRef() { return make_shared<GLContext>(); }




}




#endif /* VVGL_GLContext_hpp */
