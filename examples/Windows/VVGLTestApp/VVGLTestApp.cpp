#include "header.h"
#include "VVGLTestApp.h"

//#define _WIN32_WINNT 0x0500
//#include <windows.h>
//#include <tchar.h>
#include <iostream>

#include "WindowGLObject.hpp"
#include "GLBufferWindow.hpp"

#include <GL/wglext.h>

using namespace std;
using namespace VVGL;




//	this is necessary to get the app to use the discrete GPU (if present)
#if defined(VVGL_SDK_WIN)
extern "C"
{
	__declspec(dllexport) uint32_t NvOptimusEnablement = 0x00000001;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif	//	VVGL_SDK_WIN




//	the main event!
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
	
	//	make a GLBufferWindow, tell it to start rendering.  the buffer window will share the shared context (so it can share textures/etc)
	GLBufferWindow			displayWindow(shareCtx);

	//	make the global buffer pool!
	GLBufferPoolRef			bp = CreateGlobalBufferPool(shareCtx->newContextSharingMe());
	
	//	make a scene that will render-to-texture
	GLSceneRef				renderScene = CreateGLSceneRefUsing(shareCtx->newContextSharingMe());
	renderScene->setPerformClear(true);
	
	//	configure a render callback for the display window.  the display window displays GLBuffers (which are really GL textures)- its render
	//	callback is executed just before the window displays its texture, so it makes sense to have the render callback render the buffer/texture
	//	which will be displayed immediately after.
	displayWindow.setRenderCallback([=,&displayWindow](const GLBufferWindow & inWindow) {
		//std::cout << "display window driving render-to-texture here..." << endl;

		Size				tmpSize = Size(1920, 1080);
		GLBufferRef			tmpBuffer = CreateRGBATex(tmpSize, true, GetGlobalBufferPool());
		//	the rendered texture will appear to "flicker"- every frame has a different (randomly chosen) luma value.  i'm making it flicker so i can check for tearing.
		float				tmpLuma = float(rand()) / 32767.0f * 0.8f + 0.1f;
		renderScene->setClearColor(tmpLuma, tmpLuma, tmpLuma, 1.0);
		renderScene->renderToBuffer(tmpBuffer);
		//std::cout << "rendered to texture " << tmpBuffer->getDescriptionString() << endl;

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








