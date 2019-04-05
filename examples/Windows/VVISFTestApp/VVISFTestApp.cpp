#include "header.h"
#include "VVISFTestApp.h"

//#define _WIN32_WINNT 0x0500
#include <windows.h>
#include <tchar.h>
#include <iostream>

#include "VVISF.hpp"
#include "WindowGLObject.hpp"
#include "GLBufferWindow.hpp"

#include <GL/wglext.h>

using namespace std;
using namespace VVGL;
using namespace VVISF;




//	this is necessary to get the app to use the discrete GPU (if present)
#if defined(VVGL_SDK_WIN)
extern "C"
{
	__declspec(dllexport) uint32_t NvOptimusEnablement = 0x00000001;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif	//	VVGL_SDK_WIN




//	application string constants
const TCHAR* AppClassName = _T("VVISF_TEST_APP");
//	whether or not the gl environment is initializing- set to false 
static bool			glInitializing = true;

//	GL's funny: the "modern" function to create a GL context must be retrieved from the GL driver, and the GL driver doesn't load until we create a GL context using the old-fashioned approach.  if you're using GLEW or something like that then you don't need these function pointers (which is why they're commented out here), but i'm leaving them in for exposition.
//PFNWGLCHOOSEPIXELFORMATARBPROC		wglChoosePixelFormatARB_local = NULL;
//PFNWGLCREATECONTEXTATTRIBSARBPROC		wglCreateContextAttribsARB_local = NULL;

//	this is the event callback proc for the window subclass we create
LRESULT CALLBACK TestAppWndProc(HWND Wnd, UINT Msg, WPARAM wParam, LPARAM lParam);
//	this function bootstraps the GL environment using a window- it creates a GL context using the "old-fashioned" approach and then calls glewInit(), which loads all the GL extensions and gets function pointers to the GL functions from the driver.
void BootstrapGLUsingWindow(HWND inWnd);




int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	//	allocate a console and bind cout/cin to it so we can view cout statements
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);

	cout << __PRETTY_FUNCTION__ << endl;

	//	create the window class we'll be using to bootstrap the GL environment (this is the "first window"- a GL context is created on it, and used to call glewInit())
	WNDCLASSEX		WndClass;
	memset(&WndClass, 0, sizeof(WNDCLASSEX));
	WndClass.cbSize = sizeof(WNDCLASSEX);
	//WndClass.style = CS_OWNDC;
	WndClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	WndClass.lpfnWndProc = TestAppWndProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = GetModuleHandle(NULL);
	WndClass.hIcon = LoadIcon(0, IDI_APPLICATION);
	WndClass.hCursor = LoadCursor(0, IDC_ARROW);
	WndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	WndClass.lpszMenuName = NULL;
	WndClass.lpszClassName = AppClassName;

	RegisterClassEx(&WndClass);


	//	in windows, you need to create a GL 1.1 ctx before you can access any of the more complex context creation functions, so make a throwaway window first to do that
	HWND			tmpWnd;
	tmpWnd = CreateWindowEx(
		0,
		AppClassName,
		_T("OpenGL Demo Program"),
		WS_VISIBLE | WS_OVERLAPPEDWINDOW,
		0, 0, 1, 1,
		0,
		0,
		0,
		NULL);
	//	now that we're done bootstrapping the GL environment, delete this window (a window can only have its pixel format declared once so we can't do anything further with it)
	DestroyWindow(tmpWnd);

	//	make the global share context!
	GLContextRef			shareCtx = CreateGLContextRef(NULL, NULL);

	//	make a GLBufferWindow, tell it to start rendering
	GLBufferWindow			displayWindow(shareCtx);

	//	make the global buffer pool!
	GLBufferPoolRef			bp = CreateGlobalBufferPool(shareCtx->newContextSharingMe());
	
	//	make the ISF scene which will render to texture
	ISFSceneRef				renderScene = CreateISFSceneRefUsing(shareCtx->newContextSharingMe());

	//	tell the ISF scene to load a file (a couple ISFs are included with this app, and copied to the build folder via a post-build event)
	wchar_t			tmpBuffer[MAX_PATH];
	GetModuleFileName(NULL, tmpBuffer, MAX_PATH);
	wstring			tmpWStr(tmpBuffer);
	string			tmpStr(tmpWStr.begin(), tmpWStr.end());
	string			parentDir = StringByDeletingLastPathComponent(tmpStr);
	//string			tmpPath = parentDir + string("\\Color Bars.fs");
	string			tmpPath = parentDir + string("\\CellMod.fs");
	try {
		renderScene->useFile(tmpPath);
	}
	catch (...) {
		cout << "ERR: caught exception trying to load ISF file!" << endl;
	}
	
	//	configure a render callback for the display window.  the display window displays GLBuffers (which are really GL textures)- its render
	//	callback is executed just before the window displays its texture, so it makes sense to have the render callback render the buffer/texture
	//	which will be displayed immediately after.
	displayWindow.setRenderCallback([=, &displayWindow](const GLBufferWindow & inWindow) {
		//std::cout << "display window driving render-to-texture here..." << endl;

		//	render the ISF to a buffer/texture
		GLBufferRef			tmpBuffer = CreateRGBATex(Size(1920, 1080), true, GetGlobalBufferPool());
		renderScene->renderToBuffer(tmpBuffer, tmpBuffer->size);

		//	pass the texture we just rendered to the display window (the window will draw its buffer immeidately after this render callback finishes)
		displayWindow.drawBuffer(tmpBuffer);
		
		//	tell the buffer pool to perform any housekeeping, which would remove any textures that have been lingering too long
		GLBufferPoolRef		bp = GetGlobalBufferPool();
		if (bp != nullptr)
			bp->housekeeping();
	});

	//	now that the display window has a render callback and is all set up and ready to go, tell it to begin rendering
	displayWindow.startRendering();

	//	enter the event loop
	MSG				Msg;
	while (GetMessage(&Msg, 0, 0, 0)) {								// Get messages
		TranslateMessage(&Msg);										// Translate each message
		DispatchMessage(&Msg);										// Dispatch each message
	};
	return (0);
}




LRESULT CALLBACK TestAppWndProc(HWND Wnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
	switch (Msg) {
	case WM_CREATE:
		//	my understanding is you shouldn't initialize GL before this proc, as the device context may not be complete
		if (glInitializing) {
			BootstrapGLUsingWindow(Wnd);
		}
	break;
	case WM_DESTROY:
		if (glInitializing) {
			//	we're done initializing GL now!
			glInitializing = false;
		}
		else {
			//	if we're not initializing GL then the user is closing the window and we should quit
			PostQuitMessage(0);
		}
		break;
	default:
		return DefWindowProc(Wnd, Msg, wParam, lParam);
	};
	return 0;
};




//	this function initializes GL environment by creating that first GL context needed to bootstrap loading more advanced contexts
void BootstrapGLUsingWindow(HWND inWnd) {
	//std::cout << __FUNCSIG__ << endl;

	//	get the window's device context
	HDC			tmpDC = GetDC(inWnd);
	//	pick a hardware accelerated pixel format for the device context
	PIXELFORMATDESCRIPTOR		tmpPFD;
	ZeroMemory(&tmpPFD, sizeof(tmpPFD));
	tmpPFD.nSize = sizeof(tmpPFD);
	tmpPFD.nVersion = 1;
	tmpPFD.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	tmpPFD.iPixelType = PFD_TYPE_RGBA;
	tmpPFD.cColorBits = 32;
	tmpPFD.cAlphaBits = 8;
	tmpPFD.cDepthBits = 24;

	int			tmpPFDID = ChoosePixelFormat(tmpDC, &tmpPFD);
	if (tmpPFDID == 0) {
		std::cout << "ERR: unable to choose pixel format in " << __FUNCSIG__ << endl;
		return;
	}

	if (SetPixelFormat(tmpDC, tmpPFDID, &tmpPFD) == false) {
		std::cout << "ERR: unable to SetPixelFormat() in " << __FUNCSIG__ << endl;
		return;
	}

	//	create a rendering context, make it current
	HGLRC		tmpRC = wglCreateContext(tmpDC);
	if (tmpRC == 0) {
		std::cout << "ERR: wglCreateContext() failed in " << __FUNCSIG__ << endl;
		return;
	}
	//GLERRLOG	//	if there's no ctx then this will throw a 1282
	if (wglMakeCurrent(tmpDC, tmpRC) == false) {
		std::cout << "ERR: wglMakeCurrent() failed in " << __FUNCSIG__ << endl;
		return;
	}
	GLERRLOG

		//	if you wanted to manually load GL functions, this is how you'd go about doing it.  instead we use GLEW below, but this is illustrative of what we're skipping...
		/*
		wglChoosePixelFormatARB_local = reinterpret_cast<PFNWGLCHOOSEPIXELFORMATARBPROC>(wglGetProcAddress("wglChoosePixelFormatARB"));
		if (wglChoosePixelFormatARB_local == nullptr) {
			std::cout << "ERR: wglGetProcAdddress A failed in " << __FUNCSIG__ << endl;
			return;
		}
		wglCreateContextAttribsARB_local = reinterpret_cast<PFNWGLCREATECONTEXTATTRIBSARBPROC>(wglGetProcAddress("wglCreateContextAttribsARB"));
		if (wglCreateContextAttribsARB_local == nullptr) {
			std::cout << "ERR: wglGetProcAddress B failed in " << __FUNCSIG__ << endl;
			return;
		}
		*/

		//	we need to load the correct GL function pointers from the driver- do this by calling GLContext::bootstrapGLEnvironmentIfNecessary(), which basicaly calls through to glewInit() (VVGL uses GLEW)
		GLenum			glErr = GLContext::bootstrapGLEnvironmentIfNecessary();
	if (glErr != GLEW_OK)
		std::cout << "ERR: " << glErr << " at glewInit() in " << __PRETTY_FUNCTION__ << endl;
	//	...now we can get ptrs from modern GL funcs!

	//	delete the rendering context and device context
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(tmpRC);
	ReleaseDC(inWnd, tmpDC);
}

