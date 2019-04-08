#include "GLContextWindowBacking.hpp"

#if defined(VVGL_SDK_WIN)




namespace VVGL {




	const TCHAR*			GLCONTEXTWINDOWBACKING_WNDCLASSNAME = _T("GLCONTEXTWINDOWBACKING_WNDCLASSNAME");
	static const TCHAR*		GLCONTEXTWINDOWBACKING_PROPERTY = _T("GLCONTEXTWINDOWBACKING_PROPERTY");
	static bool				GLContextWindowBackingClassRegistered = false;

	LRESULT CALLBACK GLContextWindowBacking_wndProc(HWND Wnd, UINT Msg, WPARAM wParam, LPARAM lParam);



	GLContextWindowBacking::GLContextWindowBacking(const VVGL::Rect & inRect) {
		//cout << __PRETTY_FUNCTION__ << endl;
		//	register the window class if we haven't already
		if (!GLContextWindowBackingClassRegistered) {
			WNDCLASSEX		WndClass;
			memset(&WndClass, 0, sizeof(WNDCLASSEX));
			WndClass.cbSize = sizeof(WNDCLASSEX);
			//WndClass.style = CS_OWNDC;	// Class styles
			WndClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
			WndClass.lpfnWndProc = GLContextWindowBacking_wndProc;
			WndClass.cbClsExtra = 0;	// No extra class data
			WndClass.cbWndExtra = 0;	// No extra window data
			WndClass.hInstance = GetModuleHandle(NULL);
			WndClass.hIcon = LoadIcon(0, IDI_APPLICATION);
			WndClass.hCursor = LoadCursor(0, IDC_ARROW);
			WndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
			WndClass.lpszMenuName = NULL;
			WndClass.lpszClassName = GLCONTEXTWINDOWBACKING_WNDCLASSNAME;

			RegisterClassEx(&WndClass);
			GLContextWindowBackingClassRegistered = true;
		}

		//	create the window, get its device context
		//DWORD			tmpFlag = WS_OVERLAPPEDWINDOW | WS_CHILD;
		//DWORD			tmpFlag = WS_OVERLAPPEDWINDOW;
		//DWORD			tmpFlag = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
		//tmpFlag = tmpFlag | WS_VISIBLE;
		DWORD			tmpFlag = 0;
		tmpFlag |= WS_CLIPSIBLINGS;
		tmpFlag |= WS_CLIPCHILDREN;
		//tmpFlag |= WS_POPUP;
		//tmpFlag |= WS_EX_APPWINDOW;
		//tmpFlag |= WS_EX_TOPMOST;
		//tmpFlag |= WS_VISIBLE;
		//tmpFlag |= WS_MAXIMIZEBOX;
		//tmpFlag |= WS_THICKFRAME;
		//tmpFlag |= WS_EX_WINDOWEDGE;
		tmpFlag |= WS_OVERLAPPEDWINDOW;

		_wnd = CreateWindowEx(
			0,
			GLCONTEXTWINDOWBACKING_WNDCLASSNAME,
			_T("VVGL Test App"),
			tmpFlag,
			int(inRect.botLeft().x), int(inRect.botLeft().y), int(inRect.size.width), int(inRect.size.height),
			//int(inRect.topLeft().x), int(inRect.topLeft().y), int(inRect.botRight().x), int(inRect.botRight().y),
			0,
			0,
			NULL,
			NULL);
		SetProp(_wnd, GLCONTEXTWINDOWBACKING_PROPERTY, (HANDLE)this);
		_dc = GetDC(_wnd);

		//	configure the window's pixel format
		ConfigDeviceContextPixelFormat(_dc);
	}
	GLContextWindowBacking::~GLContextWindowBacking() {
		ReleaseDC(_wnd, _dc);
		DestroyWindow(_wnd);
	}




	LRESULT CALLBACK GLContextWindowBacking_wndProc(HWND Wnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
		//if (Msg == WM_CREATE)
		//	cout << "GLContextWindowBacking received WM_CREATE and is clear to create a GL ctx" << endl;
		return DefWindowProc(Wnd, Msg, wParam, lParam);
	};

}



#endif	//	VVGL_SDK_WIN