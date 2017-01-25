#include "VVRPIDispManager.hpp"


#if ISF_TARGET_RPI


#include <iostream>
#include <cassert>

#include "VVGL.hpp"




VVRPIDispManager::VVRPIDispManager()	{
	using namespace std;
	cout << __PRETTY_FUNCTION__ << endl;
	
	//	gotta run this first or nothing will work
	bcm_host_init();
	assert(eglGetError() == EGL_SUCCESS);
	//	get the display
	eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	assert(eglDisplay != EGL_NO_DISPLAY);
	assert(eglGetError() == EGL_SUCCESS);
	//	initialize the display
	EGLBoolean			result;
	result = eglInitialize(eglDisplay, NULL, NULL);
	assert(result != EGL_FALSE);
	assert(eglGetError() == EGL_SUCCESS);
	//	choose a display configuration
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
	result = eglChooseConfig(eglDisplay, targetDispAttribs, &eglConfig, 1, &numConfigs);
	assert(result != GL_FALSE);
	assert(eglGetError() == EGL_SUCCESS);
	//	pick an API to use
	result = eglBindAPI(EGL_OPENGL_ES_API);
	assert(result != GL_FALSE);
	assert(eglGetError() == EGL_SUCCESS);
	//	make a gl context
	const EGLint		targetCtxAttribs[] = {
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};
	eglCtx = eglCreateContext(eglDisplay, eglConfig, EGL_NO_CONTEXT, targetCtxAttribs);
	assert(eglCtx != EGL_NO_CONTEXT);
	assert(eglGetError() == EGL_SUCCESS);
	//	get the dimensions of the display
	uint32_t		displayWidth = 0;
	uint32_t		displayHeight = 0;
	int32_t			success = graphics_get_display_size(0, &displayWidth, &displayHeight);
	assert(success >= 0);
	dstRect.width = displayWidth;
	dstRect.height = displayHeight;
	srcRect.width = displayWidth << 16;
	srcRect.height = displayHeight << 16;
	//	create and set up a display element
	dispman_display = vc_dispmanx_display_open(0);
	dispman_update = vc_dispmanx_update_start(0);
	dispman_element = vc_dispmanx_element_add(
		dispman_update,
		dispman_display,
		0,
		&dstRect,
		0,
		&srcRect,
		DISPMANX_PROTECTION_NONE,
		0,
		0,
		(DISPMANX_TRANSFORM_T)0);
	nativeWindow.element = dispman_element;
	nativeWindow.width = displayWidth;
	nativeWindow.height = displayHeight;
	//	end the update, wait for it to complete
	vc_dispmanx_update_submit_sync(dispman_update);
	
	//	create a window surface for the display
	eglWinSurface = eglCreateWindowSurface(eglDisplay, eglConfig, &nativeWindow, NULL);
	assert(eglWinSurface != EGL_NO_SURFACE);
	
	result = eglMakeCurrent(eglDisplay, eglWinSurface, eglWinSurface, eglCtx);
	assert(result != EGL_FALSE);
	assert(eglGetError() == EGL_SUCCESS);
	
}
VVRPIDispManager::~VVRPIDispManager()	{
	using namespace std;
	cout << __PRETTY_FUNCTION__ << endl;
	
	eglDestroySurface(eglDisplay, eglWinSurface);
	
	dispman_update = vc_dispmanx_update_start(0);
	int			s = vc_dispmanx_element_remove(dispman_update, dispman_element);
	assert(s==0);
	vc_dispmanx_update_submit_sync(dispman_update);
	s = vc_dispmanx_display_close(dispman_display);
	assert(s==0);
	
	if (eglDisplay != EGL_NO_DISPLAY)	{
		eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		eglDestroyContext(eglDisplay, eglCtx);
		eglTerminate(eglDisplay);
	}
	
}
void VVRPIDispManager::testFunc()	{
	using namespace std;
	cout << __PRETTY_FUNCTION__ << endl;
	
	glClearColor(1., 0., 0., 1.);
	GLERRLOG
	glClear( GL_COLOR_BUFFER_BIT );
	GLERRLOG
	glFlush();
	GLERRLOG
	eglSwapBuffers(eglDisplay, eglWinSurface);
}
void VVRPIDispManager::swapBuffers()	{
	eglSwapBuffers(eglDisplay, eglWinSurface);
}


#endif /*	ISF_TARGET_RPI	*/
