#include "WindowGLObject.hpp"

#include <tchar.h>
#include <iostream>




using namespace std;
using namespace VVGL;




//	the name of our custom HWND class
const TCHAR*			WINGLOBJECT_WNDCLASSNAME = _T("WINGLOBJECT_WNDCLASSNAME");
//	we store a weak ref to the WindowGLObject instance that owns the HWND in the HWND itself for retrieval from HWND-facing calls
static const TCHAR*		WINGLOBJECT_PROPERTY = _T("WINGLOBJECT_PROPERTY");
//	we only want to register the class once
static bool				windowGLObjectClassRegistered = false;

//	forward decl of the callback function that our window class will use
LRESULT CALLBACK WindowGLObject_wndProc(HWND Wnd, UINT Msg, WPARAM wParam, LPARAM lParam);




WindowGLObject::WindowGLObject(const GLContextRef & inShareCtx) {
	//cout << __PRETTY_FUNCTION__ << endl;


	if (!windowGLObjectClassRegistered) {
		WNDCLASSEX		WndClass;
		memset(&WndClass, 0, sizeof(WNDCLASSEX));
		WndClass.cbSize = sizeof(WNDCLASSEX);
		//WndClass.style = CS_OWNDC;	// Class styles
		WndClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		WndClass.lpfnWndProc = WindowGLObject_wndProc;
		WndClass.cbClsExtra = 0;
		WndClass.cbWndExtra = 0;
		WndClass.hInstance = GetModuleHandle(NULL);
		WndClass.hIcon = LoadIcon(0, IDI_APPLICATION);
		WndClass.hCursor = LoadCursor(0, IDC_ARROW);
		WndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		WndClass.lpszMenuName = NULL;
		WndClass.lpszClassName = WINGLOBJECT_WNDCLASSNAME;

		RegisterClassEx(&WndClass);
		windowGLObjectClassRegistered = true;
	}

	DWORD			tmpFlag = 0;
	tmpFlag |= WS_CLIPSIBLINGS;
	tmpFlag |= WS_CLIPCHILDREN;
	//tmpFlag |= WS_POPUP;
	//tmpFlag |= WS_EX_APPWINDOW;
	//tmpFlag |= WS_EX_TOPMOST;
	tmpFlag |= WS_VISIBLE;
	//tmpFlag |= WS_MAXIMIZEBOX;
	//tmpFlag |= WS_THICKFRAME;
	//tmpFlag |= WS_EX_WINDOWEDGE;
	tmpFlag |= WS_OVERLAPPEDWINDOW;

	_wnd = CreateWindowEx(
		0,
		WINGLOBJECT_WNDCLASSNAME,
		_T("VVGL Test App"),
		tmpFlag,
		0, 0, 640, 480,
		//1920, 1080, 1920, 1080,
		0,
		0,
		NULL,
		NULL);
	SetProp(_wnd, WINGLOBJECT_PROPERTY, (HANDLE)this);
	_dc = GetDC(_wnd);

	//	configure the device context's pixel format
	ConfigDeviceContextPixelFormat(_dc);

	//	assemble attribs that describe the GL context we want
	unique_ptr<int[]>		contextAttribs = AllocGL4ContextAttribs();
	//unique_ptr<int[]>		contextAttribs = AllocCompatibilityContextAttribs();
	GLContextRef			tmpCtx = nullptr;

	//	if we were passed a valid share context, the GL context we create has to share it
	if (inShareCtx != nullptr)
		tmpCtx = CreateGLContextRef(inShareCtx->context(), _dc, contextAttribs.get());
	//	else we were passed a null share ctx- just make a new GL context, it doesn
	else
		tmpCtx = CreateGLContextRef(NULL, _dc, contextAttribs.get());
	
	//	apply the tmp ctx we just made
	_ctx = tmpCtx;

	//	make the context we just created current, so we can enable vsync
	if (tmpCtx != nullptr) {
		tmpCtx->makeCurrentIfNotCurrent();
		wglSwapIntervalEXT(1);
	}
}



WindowGLObject::~WindowGLObject() {
	stopRendering();
	wglMakeCurrent(NULL, NULL);
	ReleaseDC(_wnd, _dc);
	DestroyWindow(_wnd);
}


//	this method is called from the windows API callback proc for paint events, subclasses can override here to implement custom drawing
void WindowGLObject::glDrawCallback() {
	//cout << __PRETTY_FUNCTION__ << endl;
	lock_guard<recursive_mutex>		lock(_ctxLock);

	if (_ctx == nullptr)
		return;

	_ctx->makeCurrentIfNotCurrent();
	glClearColor(1., 0., 0., 1.);
	GLERRLOG
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	GLERRLOG
}


void WindowGLObject::startRendering() {
	//cout << __PRETTY_FUNCTION__ << endl;
	lock_guard<recursive_mutex>		lock(_ctxLock);
	if (_timer == NULL) {
		_timer = SetTimer(_wnd, 1, 16, 0);
	}
	if (!_rendering)
		InvalidateRect(_wnd, 0, TRUE);
	_rendering = true;
}
void WindowGLObject::stopRendering() {
	//cout << __PRETTY_FUNCTION__ << endl;
	lock_guard<recursive_mutex>		lock(_ctxLock);
	_rendering = false;
	if (_timer != NULL) {
		KillTimer(_wnd, _timer);
		_timer = NULL;
	}
}
bool WindowGLObject::rendering() {
	lock_guard<recursive_mutex>		lock(_ctxLock);
	return _rendering;
}
Size WindowGLObject::windowSize() {
	Size			returnMe;
	RECT			tmpRect;
	if (_wnd==NULL || !GetWindowRect(_wnd, &tmpRect))
		cout << "ERR: unable to get window dims in " << __PRETTY_FUNCTION__ << endl;
	else
		returnMe = Size(abs(tmpRect.right - tmpRect.left), abs(tmpRect.top - tmpRect.bottom));
	return returnMe;
}
HDC WindowGLObject::dc() {
	lock_guard<recursive_mutex>		lock(_ctxLock);
	return _dc;
}
GLContextRef WindowGLObject::context() {
	lock_guard<recursive_mutex>		lock(_ctxLock);
	return _ctx;
}





LRESULT CALLBACK WindowGLObject_wndProc(HWND Wnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
	//cout << __PRETTY_FUNCTION__ << ", " << Msg << endl;
	switch (Msg) {
	case WM_CREATE:
	{
		//cout << "WM_CREATE proc" << endl;
	}
	break;
	case WM_DESTROY:
		//cout << "destroying a window" << endl;
		break;
	case WM_COMMAND:
		break;
	case WM_PAINT:
	{
		//cout << "paint event " << Msg << endl;
		
		//PAINTSTRUCT			Ps;
		WindowGLObject		*tmpObj = static_cast<WindowGLObject*>(GetProp(Wnd, WINGLOBJECT_PROPERTY));
		bool				tmpObjIsRendering = (tmpObj == NULL) ? false : tmpObj->rendering();
		//BeginPaint(Wnd, &Ps);
		tmpObj->glDrawCallback();
		SwapBuffers(tmpObj->dc());
		//SwapBuffers(Ps.hdc);
		//EndPaint(Wnd, &Ps);

		LRESULT			returnMe = DefWindowProc(Wnd, Msg, wParam, lParam);
		//if (tmpObjIsRendering) {
		//	InvalidateRect(Wnd, 0, TRUE);	//	this would cause the window to paint itself/render as rapidly as possible
		//}
		return returnMe;
	}
	break;
	case WM_TIMER:
	{
		//cout << "timer event " << Msg << endl;
		//	just invalidate the rect, we'll render on paint event
		LRESULT			returnMe = DefWindowProc(Wnd, Msg, wParam, lParam);
		InvalidateRect(Wnd, 0, TRUE);
		return returnMe;
	}
		break;
	case WM_WINDOWPOSCHANGED:
		//	check to see if the window size has changed (movement doesn't require resize)
		if (lParam == 0 || (((PWINDOWPOS)lParam)->flags & SWP_NOSIZE) == 0) {
			//	resize & redraw
			InvalidateRect(Wnd, 0, TRUE);
		}
		break;
	case WM_ERASEBKGND:
		//return (FALSE);
		return (TRUE);
	default:
		return DefWindowProc(Wnd, Msg, wParam, lParam);
	};
	return 0;
};

