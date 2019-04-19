#include "header.h"
#include "VVISFTestApp.h"

//#define _WIN32_WINNT 0x0500
//#include <windows.h>
//#include <tchar.h>
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




int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	//	allocate a console and bind cout/cin to it so we can view cout statements
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);

	cout << __PRETTY_FUNCTION__ << endl;

	//	boostratp the GL environment- this creates a window, loads a basic GL context, then uses GLEW to load the more advanced GL extensions before deleting everything and cleaning up after itself
	GLContext::bootstrapGLEnvironmentIfNecessary();

	//	make the global share context!
	GLContextRef			shareCtx = CreateNewGLContextRef(NULL, NULL);

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

