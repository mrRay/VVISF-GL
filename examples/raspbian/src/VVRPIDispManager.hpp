#ifndef VVRPIDispManager_hpp
#define VVRPIDispManager_hpp


#if ISF_TARGET_RPI


#include "bcm_host.h"
//#include <GLES/gl.h>
//#include <GLES/glext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>




struct VVRPIDispManager	{
	EGLDisplay				eglDisplay = EGL_NO_DISPLAY;	//	the abstract display on which stuff will be drawn
	EGLConfig				eglConfig;
	EGLContext				eglCtx = EGL_NO_CONTEXT;	//	 the state machine defined by the client API
	EGLSurface				eglWinSurface = EGL_NO_SURFACE;
	EGLNativeWindowType		eglWin;
	EGL_DISPMANX_WINDOW_T	nativeWindow;
	
	DISPMANX_ELEMENT_HANDLE_T	dispman_element;
	DISPMANX_DISPLAY_HANDLE_T	dispman_display;
	DISPMANX_UPDATE_HANDLE_T	dispman_update;
	
	VC_RECT_T		srcRect = {0,0,0,0};
	VC_RECT_T		dstRect = {0,0,0,0};
	
	VVRPIDispManager();
	~VVRPIDispManager();
	
	void testFunc();
	void swapBuffers();
};


#endif	/*	ISF_TARGET_RPI	*/


#endif /* VVRPIDispManager_hpp */
