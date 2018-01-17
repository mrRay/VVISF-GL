
#include <iostream>
#include <cassert>
#include "VVGL.hpp"
#if ISF_TARGET_RPI
	#include "bcm_host.h"
	#include "VVRPIDispManager.hpp"
	//#include "VVRPICamManager.hpp"
#endif
#include "ISFKit.h"
#include <sstream>
#include <string>
#include <fstream>
#include <thread>
#include <atomic>




std::atomic<bool>		quitFlag(false);




int main(int argc, const char *argv[])	{
	using namespace std;
	using namespace VVGL;
	using namespace VVISF;
	cout << "****************************************\n";
	cout << __PRETTY_FUNCTION__ << endl;
	cout << "****************************************\n";
	
	
	/*			raspberry pi setup			*/
	//	make a display manager, which handles the EGL setup for the RPi
	VVRPIDispManager		dm;
	VVGL::Rect				displayRect = VVGL::Rect((double)dm.dstRect.x, (double)dm.dstRect.y, (double)dm.dstRect.width, (double)dm.dstRect.height);
	cout << "\tdisplayRect is " << displayRect << endl;
	
	
	/*			OpenGL setup				*/
	//	make a VVGLContext from the EGL properties owned by the display manager
	cout << "\tmaking base GL context...\n";
	VVGLContextRef			baseCtx = make_shared<VVGLContext>(dm.eglDisplay, dm.eglWinSurface, EGL_NO_CONTEXT, dm.eglCtx);
	//	make the global buffer pool
	cout << "\tmaking global buffer pool...\n";
	CreateGlobalBufferPool(baseCtx);
	
	
	/*			display scene setup			*/
	//	this is the buffer we're going to display.  we render content into this, and the display scene displays it.
	VVGLBufferRef			displayBuffer = CreateRGBATex({640.,480.});
	//	make the scene we're going to use to display content on the screen, set it up
	cout << "\tmaking display scene...\n";
	VVGLScene				displayScene(baseCtx->newContextSharingMe());
	displayScene.setAlwaysNeedsReshape(true);
	//displayScene.setPerformClear(false);
	//displayScene.setClearColor(GLColor(1,0,0,1));
	displayScene.setOrthoSize(displayRect.size);
	string			fsString("\n\
precision mediump		float;\n\
uniform sampler2D		inputImage;\n\
varying vec2			vertSTVar;\n\
\n\
void main()	{\n\
	gl_FragColor = texture2D(inputImage, vertSTVar);\n\
	//gl_FragColor = vec4(vertSTVar.x, vertSTVar.y, 0., 1.);\n\
	//gl_FragColor = vec4(1., 0., 0., 1.);\n\
}\n\
");
	string			vsString("\n\
attribute vec3			vertXYZ;\n\
attribute vec2			vertST;\n\
uniform mat4			vvglOrthoProj;\n\
varying vec2			vertSTVar;\n\
\n\
void main()	{\n\
	gl_Position = vec4(vertXYZ.x, vertXYZ.y, vertXYZ.z, 1.0) * vvglOrthoProj;\n\
	vertSTVar = vertST;\n\
}\n\
");
	displayScene.setFragmentShaderString(fsString);
	displayScene.setVertexShaderString(vsString);
	//	make some cached attrib/uniform refs that will refer to vars in the vert/frag shaders in the render prep and render callbacks
	VVGLCachedAttribRef		inputXYZ = make_shared<VVGLCachedAttrib>("vertXYZ");
	VVGLCachedAttribRef		inputST = make_shared<VVGLCachedAttrib>("vertST");
	VVGLCachedUniRef		inputTex = make_shared<VVGLCachedUni>("inputImage");
	//	set up the scene's render prep callback
	displayScene.setRenderPrepCallback([inputXYZ,inputST,inputTex](const VVGLScene & s, const bool & inReshaped, const bool & inPgmChanged)	{
		//	if the program's changed, we want to re-cache the locations of the attrib/uniforms
		if (inPgmChanged)	{
			cout << "\tpgm changed, caching GL vals.  pgm is " << s.getProgram() << endl;
			inputXYZ->cacheTheLoc(s.getProgram());
			inputST->cacheTheLoc(s.getProgram());
			inputTex->cacheTheLoc(s.getProgram());
			cout << "\tinputXYZ is now " << inputXYZ->loc << ", inputST is " << inputST->loc << ", inputTex is " << inputTex->loc << endl;
			cout << "\taddress of inputTex is " << &(*inputTex) << endl;
		}
		else	{
			if (inputXYZ->loc < 0)	{
				cout << "\tpgm didn't change, but loc invalid- needs to be cached.  pgm is " << s.getProgram() << endl;
				inputXYZ->cacheTheLoc(s.getProgram());
			}
			if (inputST->loc < 0)
				inputST->cacheTheLoc(s.getProgram());
			if (inputTex->loc < 0)
				inputTex->cacheTheLoc(s.getProgram());
		}
	});
	//	set up the scene's render callback
	displayScene.setRenderCallback([displayRect,inputXYZ,inputST,inputTex,&displayBuffer](const VVGLScene & s){
		using namespace VVGL;
		//cout << __FUNCTION__ << ", drawing buffer " << *displayBuffer << endl;
		
		//	set up some basic quad stuff
		VVGL::Rect			geoRect = displayRect;
		VVGL::Rect			texRect = (displayBuffer==nullptr) ? VVGL::Rect(0., 0., 1., 1.) : displayBuffer->glReadySrcRect();
		Quad<VertXYZST>		targetQuad;
		targetQuad.populateGeo(geoRect);
		targetQuad.populateTex(texRect, false);
		
		//	populate the vertex attribute
		if (inputXYZ->loc >= 0)	{
			inputXYZ->enable();
			glVertexAttribPointer(inputXYZ->loc, 3, GL_FLOAT, GL_FALSE, targetQuad.stride(), BUFFER_OFFSET(targetQuad.geoOffset()));
			GLERRLOG
		}
		else	{
			cout << "\tERR: cannot populate inputXYZ, location " << inputXYZ->loc << " not cached " << __PRETTY_FUNCTION__ << endl;
		}
		//	populate the tex coords attribute
		if (inputST->loc >= 0)	{
			inputST->enable();
			glVertexAttribPointer(inputST->loc, 2, GL_FLOAT, GL_FALSE, targetQuad.stride(), BUFFER_OFFSET(targetQuad.texOffset()));
			GLERRLOG
		}
		else	{
			cout << "\tERR: cannot populate inputST, location not cached (" << __PRETTY_FUNCTION__ << ")\n";
		}
		//	pass the texture to its uniform
		if (inputTex->loc >= 0 && displayBuffer!=nullptr)	{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(displayBuffer->desc.target, displayBuffer->name);
			GLERRLOG
			glUniform1i(inputTex->loc, 0);
			GLERRLOG
		}
		else	{
			cout << "\tERR: cannot populate tex, location or displayBuffer not cached " << __PRETTY_FUNCTION__ << endl;
			cout << "\taddress of inputTex is " << &(*inputTex) << endl;
		}
		//	draw
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		GLERRLOG
		//	un-bind and disable stuff
		if (displayBuffer != nullptr)
			glBindTexture(displayBuffer->desc.target, 0);
		GLERRLOG
		if (inputST->loc >= 0)
			inputST->disable();
		if (inputXYZ->loc >= 0)
			inputXYZ->disable();
		
	});
	//	tell the displayScene to render (this compiles the shaders so i'm ready to pass vars to them)
	cout << "\trendering display scene once to compile stuff...\n";
	displayScene.render();
	
	
	/*			source scene setup			*/
	
	//	make the src scene
	cout << "\tmaking src ISF scene...\n";
	ISFScene				srcScene(baseCtx->newContextSharingMe());
	bool					hasSrc = false;
	srcScene.setAlwaysNeedsReshape(true);
	//srcScene.setClearColor(0., 1., 0., 1.);
	//srcScene.setSize(displayRect.size);
	//srcScene.setSize({640., 480.});
	if (argc >= 2)	{
		hasSrc = true;
		string		path(argv[1]);
		cout << "\tloading src file: " << path << endl;
		srcScene.useFile(path);
	}
	else
		cout << "\terr: no src ISF file specified...\n";
	
	
	
	/*			fx scene setup				*/
	cout << "\tmaking fx ISF scene...\n";
	ISFScene				fxScene(baseCtx->newContextSharingMe());
	bool					hasFX = false;
	fxScene.setAlwaysNeedsReshape(true);
	if (argc >= 3)	{
		hasFX = true;
		string		path(argv[2]);
		cout << "\tloading fx file: " << path << endl;
		fxScene.useFile(path);
	}
	else
		cout << "\tno FX ISF file specified...\n";
	
	
	if (!hasSrc && !hasFX)	{
		cout << "\terr: no src or fx specified, quitting...\n";
		return 0;
	}
	
	
	/*			keystroke thread setup		*/
	//	split off a thread to watch cin for keystrokes
	thread			keyThread([](){
		string		line;
		while (getline(cin, line)) {
			if (line=="q" || line=="Q")	{
				quitFlag.store(true);
				break;
			}
			else
				cout << "\tunrecognized input...\n";
		}
	});
	
	
	/*			render stuff!				*/
	cout << "\tabout to enter render loop...\n";
	while (!quitFlag.load())
	{
		cout << "\tabout to render...\n";
		//if (hasFX)	{
		//	VVGLBufferRef		tmpBuffer = srcScene.createAndRenderABuffer(displayRect.size);
		//	fxScene.setFilterInputBuffer(tmpBuffer);
		//	displayBuffer = fxScene.createAndRenderABuffer(tmpBuffer->srcRect.size);
		//}
		//else	{
			displayBuffer = srcScene.createAndRenderABuffer(displayRect.size);
			if (displayBuffer == nullptr)
				cout << "\terr: displayBuffer NULL\n";
			else
				cout << "\tdisplayBuffer is " << *displayBuffer << endl;
		//}
		//displayScene.render();
		
		cout << "\tDM swapping buffers\n";
		dm.swapBuffers();
		//assert(glGetError()==0);
		GLERRLOG
		//cout << "\thousekeeping()\n";
		GetGlobalBufferPool()->housekeeping();
	}
	
	//	kill the key input thread
	keyThread.join();
	
	return 0;
}





