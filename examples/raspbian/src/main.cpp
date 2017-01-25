
#include <iostream>
#include <cassert>
//#include "ClassA.hpp"
#include "VVGL.hpp"
#if ISF_TARGET_RPI
	#include "bcm_host.h"
	#include "VVRPIDispManager.hpp"
#endif
#include "ISFKit.h"
#include "VVBase.hpp"
#include <sstream>
#include <string>
#include <fstream>
#include <thread>
#include <atomic>




std::atomic<bool>		quitFlag(false);




int main(int argc, const char *argv[])	{
	using namespace std;
	using namespace VVGL;
	using namespace VVISFKit;
	cout << "****************************************\n";
	cout << __PRETTY_FUNCTION__ << endl;
	cout << "****************************************\n";
#if ISF_TARGET_RPI
	//	make a display manager, which handles the EGL setup for the RPi
	VVRPIDispManager		dm;
	//	make a VVGLContext from the EGL properties owned by the display manager
	VVGLContext				ctx(&(dm.eglDisplay), &(dm.eglWinSurface), &(dm.eglCtx));
	//	make the global buffer pool
	CreateGlobalBufferPool(&ctx);
	//	this is the buffer we're going to render into
	VVGLBufferRef				displayBuffer = nullptr;
	//	make the scene we're going to use to display content on the screen
	VVGLShaderScene				displayScene(&ctx);
	
	VVGL::Rect			displayRect = {(double)dm.dstRect.x, (double)dm.dstRect.y, (double)dm.dstRect.width, (double)dm.dstRect.height};
	cout << "\tdisplayRect is " << displayRect << endl;
	//	set up the display scene
	displayScene.setAlwaysNeedsReshape(true);
	//displayScene.setPerformClear(false);
	displayScene.setSize(displayRect.size);
	string			fsString("\
precision mediump		float;\
uniform sampler2D		inputImage;\
varying vec2			vertSTVar;\
\
void main()	{\
	gl_FragColor = texture2D(inputImage, vertSTVar);\
}\
");
	string			vsString("\
attribute vec4			vertXYZ;\
attribute vec2			vertST;\
uniform vec4			orthoRect;\
varying vec2			vertSTVar;\
\
void main()	{\
	gl_Position = vertXYZ;\
	mat4			projectionMatrix = mat4(2./orthoRect[2], 0., 0., -1.,\
		0., 2./orthoRect[3], 0., -1.,\
		0., 0., -1., 0.,\
		0., 0., 0., 1.);\
	gl_Position *= projectionMatrix;\
	vertSTVar = vertST;\
}\
");
	displayScene.setFragmentShaderString(fsString);
	displayScene.setVertexShaderString(vsString);
	displayScene.setRenderCallback([&](const VVGLScene & s){
		if (displayBuffer == nullptr)
			return;
		using namespace VVGL;
		//cout << __FUNCTION__ << ", drawing buffer " << *displayBuffer << endl;
		VVGLShaderScene		*recast = (VVGLShaderScene *)(&s);
		//	set up some basic data- dst rect, tex coords
		Rect			inDstRect = {0., 0., 1920., 1080.};
		GLfloat			geoCoords[] = {
			(GLfloat)MinX(inDstRect), (GLfloat)MinY(inDstRect), 0.,
			(GLfloat)MaxX(inDstRect), (GLfloat)MinY(inDstRect), 0.,
			(GLfloat)MinX(inDstRect), (GLfloat)MaxY(inDstRect), 0.,
			(GLfloat)MaxX(inDstRect), (GLfloat)MaxY(inDstRect), 0.
		};
		Rect			glTexCoordsRect = displayBuffer->glReadySrcRect();
		GLfloat			texCoords[] = {
			(GLfloat)MinX(glTexCoordsRect), (GLfloat)MinY(glTexCoordsRect),
			(GLfloat)MaxX(glTexCoordsRect), (GLfloat)MinY(glTexCoordsRect),
			(GLfloat)MinX(glTexCoordsRect), (GLfloat)MaxY(glTexCoordsRect),
			(GLfloat)MaxX(glTexCoordsRect), (GLfloat)MaxY(glTexCoordsRect)
		};
		GLint			tmpIndex;
		//	populate the vertex attribute
		tmpIndex = glGetAttribLocation(recast->getProgram(), "vertXYZ");
		glEnableVertexAttribArray(tmpIndex);
		glVertexAttribPointer(tmpIndex, 3, GL_FLOAT, GL_FALSE, 0, geoCoords);
		//	populate the tex coords attribute
		tmpIndex = glGetAttribLocation(recast->getProgram(), "vertST");
		glEnableVertexAttribArray(tmpIndex);
		glVertexAttribPointer(tmpIndex, 2, GL_FLOAT, GL_FALSE, 0, texCoords);
		//	pass the texture to its uniform
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(displayBuffer->desc.target, displayBuffer->name);
		tmpIndex = glGetUniformLocation(recast->getProgram(), "inputImage");
		glUniform1i(tmpIndex, 0);
		//	draw
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		//	unbind stuff
		glBindTexture(displayBuffer->desc.target, 0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(0);
		
	});
	//	tell the displayScene to render (this compiles the shaders so i'm ready to pass vars to them)
	displayScene.render();
	//	pass the orthogonal rect to the shader (we only need to pass it once, on creation)
	VVGL::Rect			dstRect = displayRect;
	GLint				progLoc = displayScene.getProgram();
	if (progLoc > 0)	{
		glUseProgram(progLoc);
		GLERRLOG
		GLint				samplerLoc = glGetUniformLocation(progLoc, "orthoRect");
		//cout << "\tstart, progLoc is " << progLoc << ", samplerLoc is " << samplerLoc << endl;
		GLERRLOG
		if (samplerLoc >= 0)	{
			glUniform4f(samplerLoc, dstRect.origin.x, dstRect.origin.y, dstRect.size.width, dstRect.size.height);
			GLERRLOG
		}
		glUseProgram(0);
		GLERRLOG
	}
	
	
	
	
	//string			extString = string((char *)glGetString(GL_EXTENSIONS));
	//cout << "\textensions are " << extString << endl;
	
	
	
	
	//	make the src scene
	ISFScene				srcScene(&ctx);
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
	
	
	
	
	//	make the fx scene
	ISFScene				fxScene(&ctx);
	bool					hasFX = false;
	fxScene.setAlwaysNeedsReshape(true);
	if (argc >= 3)	{
		hasFX = true;
		string		path(argv[2]);
		cout << "\tload fx file: " << path << endl;
		fxScene.useFile(path);
	}
	else
		cout << "\tno FX ISF file specified...\n";
	
	
	if (!hasSrc && !hasFX)	{
		cout << "\terr: no src or fx specified, quitting...\n";
		return 0;
	}
	
	
	
	
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
	
	
	
	
	//	render stuff!
	Timestamper				ts;
	ts.reset();
	//double					tmpDur = 5.;
	//cout << "\tdisplaying stuff for " << tmpDur << " seconds\n";
	//while (ts.nowTime().getTimeInSeconds() < tmpDur)
	while (!quitFlag.load())
	{
		if (hasFX)	{
			VVGLBufferRef		tmpBuffer = srcScene.createAndRenderABuffer(displayRect.size);
			fxScene.setFilterInputBuffer(tmpBuffer);
			displayBuffer = fxScene.createAndRenderABuffer(tmpBuffer->srcRect.size);
		}
		else	{
			displayBuffer = srcScene.createAndRenderABuffer(displayRect.size);
		}
		displayScene.render();
		
		
		dm.swapBuffers();
		//assert(glGetError()==0);
		GLERRLOG
		GetGlobalBufferPool()->housekeeping();
	}
	
	//	kill the key input thread
	keyThread.join();
	
#endif
	return 0;
}





