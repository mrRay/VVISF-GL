#ifndef VVGL_WINDOWGLOBJECT_H
#define VVGL_WINDOWGLOBJECT_H

#include <windows.h>
#include <mutex>
#include "VVGL.hpp"




/*		this is a small class that contains a visible window, and a GLContext that will render 
into that window.  it is meant to be subclassed for further functionality.			*/




class WindowGLObject {
protected:
	HWND					_wnd;
	HDC						_dc;
	std::recursive_mutex	_ctxLock;
	VVGL::GLContextRef		_ctx = nullptr;
	int						_timer = NULL;
	bool					_rendering = false;

public:
	
	//	creates a window from scratch- the GL context will share the passed context
	WindowGLObject(const VVGL::GLContextRef & inShareCtx = nullptr);

	~WindowGLObject();

	//	this method is called from the windows API callback proc for paint events, 
	//	subclasses can override here to implement custom drawing
	virtual void glDrawCallback();

	//	rendering isn't terribly efficient with this class- i'm not sure how to use vblanks to "drive" rendering, 
	//	so instead we have a timer that invalidates the window- (rendering occurs when the window receives a paint event)
	void startRendering();
	void stopRendering();
	bool rendering();

	//	returns the size of the window
	VVGL::Size windowSize();

	//	returns the window's device context
	HDC dc();
	//	returns the GL context that was created on this window
	VVGL::GLContextRef context();
};




#endif	//	VVGL_WINDOWGLOBJECT_H