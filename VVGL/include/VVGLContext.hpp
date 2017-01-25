#ifndef VVGLContext_hpp
#define VVGLContext_hpp

#include <iostream>
#if ISF_TARGET_MAC
	#include <OpenGL/OpenGL.h>
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




namespace VVGL
{


using namespace std;




#if ISF_TARGET_MAC
uint32_t GLDisplayMaskForAllScreens();
CGLPixelFormatObj CreateDefaultPixelFormat();
#endif




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
		EGLDisplay			*display = nullptr;
		EGLSurface			*winSurface = nullptr;
		EGLContext			*ctx = nullptr;	//	owned by this object
#endif
		
		
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
		//	this function doesn't create anything- it just obtains a weak ref to the passed context
		VVGLContext(EGLDisplay * inDisplay, EGLSurface * inWinSurface, EGLContext * inCtx);
#endif
		//	this function creates a context using the default pixel format
		VVGLContext();
		
		
	public:
		void generalInit();
		
		//	returned variable MUST BE FREED
		VVGLContext * allocNewContextSharingMe() const;
		//	creates a new GL context, but returned variable doesn't have to be freed
		VVGLContext newContextSharingMe() const;
		
		~VVGLContext();
		
		//	copy constructors- these do NOT create new GL contexts, they merely copy/retain the GL contexts from the passed var
		VVGLContext(const VVGLContext * n);
		VVGLContext(const VVGLContext & n);
		
		void makeCurrent();
		void makeCurrentIfNotCurrent();
		void makeCurrentIfNull();
		
		VVGLContext & operator=(const VVGLContext & n);
		friend ostream & operator<<(ostream & os, const VVGLContext & n);
		friend ostream & operator<<(ostream & os, const VVGLContext * n);
};




}




#endif /* VVGLContext_hpp */
