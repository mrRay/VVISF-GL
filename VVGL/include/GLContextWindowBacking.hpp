#ifndef VVGL_GLContextWindowBacking_hpp
#define VVGL_GLContextWindowBacking_hpp

#if defined(VVGL_SDK_WIN)

#include <Windows.h>
#include <tchar.h>
#include <iostream>
#include <functional>
#include "VVGL.hpp"




/*		This class exists to wrap an invisible/offscreen window from the Windows SDK- it exists because we need the window's "device context" 
to create an OpenGL context (GL contexts on windows appear to require actual windows as backings to access hardware acceleration).

you'll probably never have to create an instance of this class manually- GLContext will do so automatically			*/




namespace VVGL
{




class GLContextWindowBacking {
public:
	using RenderCallback = std::function<void (const GLContextWindowBacking & inWinBacking)>;

private:
	HWND			_wnd;
	HDC				_dc;

	//	it's not being used at the moment but this render callback could be used to perform drawing when the backing window receives a paint command
	RenderCallback		_renderCallback = nullptr;

public:
	GLContextWindowBacking(const Rect & inRect = Rect(0, 0, 640, 480));
	~GLContextWindowBacking();

	GLContextWindowBacking& operator=(const GLContextWindowBacking &) = delete;

	HWND wnd() { return _wnd; }
	HDC dc() { return _dc; }
	void performRenderCallback() { if (_renderCallback != nullptr) _renderCallback(*this); }

	static GLContextWindowBackingRef CreateWindowBackingRef(const Rect & inRect=Rect(0,0,640,480)){
		return make_shared<GLContextWindowBacking>(inRect);
	}
};




}	//	namespace VVGL




#endif	//	VVGL_SDK_WIN

#endif	//	VVGL_GLContextWindowBacking_hpp