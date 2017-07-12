#ifndef VVGLContext_hpp
#define VVGLContext_hpp

#include <iostream>
#if ISF_TARGET_MAC
	#import <OpenGL/OpenGL.h>
	//#import <OpenGL/gl.h>
	//#import <OpenGL/glext.h>
	#import <OpenGL/gl3.h>
	#import <OpenGL/gl3ext.h>
#elif ISF_TARGET_RPI
	#include "bcm_host.h"
	//#include <GLES/gl.h>
	//#include <GLES/glext.h>
	#include <GLES2/gl2.h>
	#include <GLES2/gl2ext.h>
	#include <EGL/egl.h>
	#include <EGL/eglext.h>
#elif ISF_TARGET_GLFW
	#include <glad/glad.h>
	#include <GLFW/glfw3.h>
#endif

#include "VVBase.hpp"




namespace VVGL
{


using namespace std;




#if ISF_TARGET_MAC
uint32_t GLDisplayMaskForAllScreens();
CGLPixelFormatObj CreateDefaultPixelFormat();
CGLPixelFormatObj CreateCompatibilityGLPixelFormat();
CGLPixelFormatObj CreateGL3PixelFormat();
CGLPixelFormatObj CreateGL4PixelFormat();
#endif




//	use a shared_ptr to pass around refs to a VVGLContext w/o copying any GL resources
//class VVGLContext;
//using VVGLContextRef = shared_ptr<VVGLContext>;




/*		VVGLContext is an attempt to make a platform-agnostic representation of an OpenGL context.  
this is useful if you want to perform common functions on a GL context (setting the current context, 
making another context in the same sharegroup), but you don't want to have to write any platform-specific code.

if you're porting VVGL to another platform, one of the first things you need to do is to make a 
new ISF_TARGET_XXXX macro, and use that macro to add your platform's GL implementation to this class.  
you should be able to following along pretty well here using the other platforms as an example.			*/
class VVGLContext	{
	public:
		
#if ISF_TARGET_MAC
		CGLContextObj		ctx = nullptr;
		CGLContextObj		sharedCtx = nullptr;
		CGLPixelFormatObj	pxlFmt = nullptr;
#elif ISF_TARGET_IOS
		void				*ctx = nullptr;	//	really an EAGLContext under iOS
#elif ISF_TARGET_GLFW
		GLFWwindow			*win = nullptr;
#elif ISF_TARGET_RPI
		EGLDisplay			display = EGL_NO_DISPLAY;	//	weak ref, potentially unsafe
		EGLSurface			winSurface = EGL_NO_SURFACE;	//	weak ref, potentially unsafe
		EGLContext			sharedCtx = EGL_NO_CONTEXT;	//	weak ref, potentially unsafe
		bool				ownsTheCtx = false;	//	set to true when i "own" ctx and must destroy it on my release
		EGLContext			ctx = EGL_NO_CONTEXT;	//	owned by this object
#endif
		GLVersion			version = GLVersion_2;
		
		
	public:
#if ISF_TARGET_MAC
		//	this function doesn't create anything- it just retains the passed ctx/share ctx/pxl fmt, leaving them null if that's how they were passed in
		VVGLContext(const CGLContextObj & inCtx, const CGLContextObj & inShareCtx, const CGLPixelFormatObj & inPxlFm=CreateDefaultPixelFormat());
		//	this function creates a context using the passed pixel format and share context
		VVGLContext(const CGLContextObj & inShareCtx, const CGLPixelFormatObj & inPxlFmt=CreateDefaultPixelFormat());
#elif ISF_TARGET_IOS
		//	"inCtx" is an EAGLContext! this function doesn't create anything- it just retains the passed ctx
		VVGLContext(const void * inCtx);
		//	this function creates a GL context using the passed sharegroup
		//VVGLContext(const void * inSharegroup);
#elif ISF_TARGET_GLFW
		VVGLContext(GLFWwindow * inWindow);
#elif ISF_TARGET_RPI
		//	this function doesn't create anything- it just obtains a weak ref to the passed EGL vars
		VVGLContext(EGLDisplay inDisplay, EGLSurface inWinSurface, EGLContext inSharedCtx, EGLContext inCtx);
		//	this function creates a new GL context using the passed shared context
		VVGLContext(EGLDisplay inDisplay, EGLSurface inWinSurface, EGLContext inSharedCtx);
#endif
		//	this function creates a context using the default pixel format
		VVGLContext();
		
		
	public:
		void generalInit();
		void calculateVersion();
		
		//	returned variable MUST BE FREED
		//VVGLContext * allocNewContextSharingMe() const;
		//	creates a new GL context, but returned variable doesn't have to be freed
		//VVGLContext newContextSharingMe() const;
		VVGLContextRef newContextSharingMe() const;
		
		~VVGLContext();
		
		//	copy constructors- these do NOT create new GL contexts, they merely copy/retain the GL contexts from the passed var
		VVGLContext(const VVGLContext * n);
		VVGLContext(const VVGLContext & n);
		VVGLContext(const VVGLContextRef & n);
		
		void makeCurrent();
		void makeCurrentIfNotCurrent();
		void makeCurrentIfNull();
		
		bool sameShareGroupAs(const VVGLContextRef & inCtx);
#if ISF_TARGET_MAC
		bool sameShareGroupAs(const CGLContextObj & inCtx);
#endif
		
		VVGLContext & operator=(const VVGLContext & n);
		friend ostream & operator<<(ostream & os, const VVGLContext & n);
		friend ostream & operator<<(ostream & os, const VVGLContext * n);
};




}




#endif /* VVGLContext_hpp */
