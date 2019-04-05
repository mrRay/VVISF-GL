#ifndef VVGL_VVGLBUFFERWINDOW_H
#define VVGL_VVGLBUFFERWINDOW_H

#include "VVGL.hpp"
#include "WindowGLObject.hpp"
#include <mutex>
#include <functional>




/*		this class displays GLBuffer instances in a window			*/




class GLBufferWindow : public WindowGLObject {
private:
	//	this _scene uses the super's _ctx ivar, which means it draws directly into the HWND
	VVGL::GLSceneRef				_scene = nullptr;	//	this scene uses OpenGL to draw the ivar _buffer in the window this class owns
	VVGL::GLBufferRef				_vao = nullptr;
	VVGL::Quad<VVGL::VertXYST>		_lastVBOCoords;
	VVGL::GLBufferRef				_buffer = nullptr;	//	the buffer that gets drawn
	std::function<void(const GLBufferWindow & window)>		_renderCallback;	//	this callback is executed *before* _buffer is drawn

public:
	GLBufferWindow(const VVGL::GLContextRef & inShareCtx = nullptr);
	
	~GLBufferWindow();

	//	draws the passed buffer
	void drawBuffer(const VVGL::GLBufferRef & inBuffer);

	inline VVGL::GLBufferRef getBuffer() { std::lock_guard<std::recursive_mutex> lock(_ctxLock); return _getBuffer(); }

	//	overrides the subclass' glDrawCallback: first executes _renderCallback, then tells my scene to draw (which draws my _buffer in the window)
	virtual void glDrawCallback();

	//	this window has an optional render callback, which is executed *before* this window does any GL drawing.
	//	it's convenient to put any code that renders a buffer you wish to draw in this callback, which will then draw the buffer immediately after
	void setRenderCallback(const std::function<void(const GLBufferWindow & inWindow)> & inCallback);

private:
	inline VVGL::GLBufferRef _getBuffer() { return _buffer; }
	void _renderNow();
	void stopRenderingImmediately();
	void _updateContext();
};




#endif	//	VVGL_VVGLBUFFERWINDOW_H

