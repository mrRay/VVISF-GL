
#if defined(_MSC_VER)
 // Make MS math.h define M_PI
 #define _USE_MATH_DEFINES
#endif

#include <stdlib.h>
#include <stdio.h>

#import <Foundation/Foundation.h>

#include "VVGL.hpp"
#include "VVISF.hpp"

//#include <glad/glad.h>
#include <GLFW/glfw3.h>

using namespace std;
using namespace VVGL;
using namespace VVISF;






//	this GLScene is going to be a shim over the GL context created by GLFW (VVGL won't create a new context, it'll just create a GLContext and populate it with the context from GLFW)
VVGL::GLSceneRef		displayScene = nullptr;
VVISF::ISFSceneRef		isfScene = nullptr;
//	create the texture variable we're eventually going to render into
VVGL::GLBufferRef		targetTex = nullptr;


void key( GLFWwindow* window, int k, int s, int action, int mods )	{
	if( action != GLFW_PRESS ) return;

	switch (k) {
	case GLFW_KEY_ESCAPE:
		glfwSetWindowShouldClose(window, GLFW_TRUE);
		break;
	case GLFW_KEY_1:
		cout << "\tabout to render ISF to texture...\n";
		targetTex = isfScene->createAndRenderABuffer(displayScene->orthoSize());
		cout << "\trendered ISF to texture " << *targetTex << endl;
		break;
	case GLFW_KEY_2:
		cout << "\tdrawing in output...\n";
		displayScene->render();
		break;
	case GLFW_KEY_3:
		cout << "\tswapping the buffer...\n";
		glfwSwapBuffers(window);
		GetGlobalBufferPool()->housekeeping();
		break;
	case GLFW_KEY_ENTER:
		cout << "\tflushing...\n";
		glFlush();
	default:
		return;
	}
}
void reshape( GLFWwindow* window, int width, int height )	{
	using namespace std;
	cout << __PRETTY_FUNCTION__ << ", " << width << " x " << height << endl;
	if (displayScene==nullptr || isfScene==nullptr)
		return;
	VVGL::Size			tmpSize(width,height);
	displayScene->setOrthoSize(tmpSize);
	//isfScene->setRenderSize(tmpSize);
}




/* program entry */
int main(int argc, char *argv[])	{
	using namespace std;
	using namespace VVGL;
	using namespace VVISF;
	
	
	if (!glfwInit())	{
		cout << "ERR: failed to initialize GLFW\n";
		exit( EXIT_FAILURE );
	}
	glfwWindowHint(GLFW_DEPTH_BITS, 16);
	GLFWwindow *	window = glfwCreateWindow( 640, 480, "Scion TC", NULL, NULL );
	if (!window)	{
		cout << "ERR: failed to create GLFW window\n";
		glfwTerminate();
		exit( EXIT_FAILURE );
	}
	
	// Set callback functions
	glfwSetFramebufferSizeCallback(window, reshape);
	glfwSetKeyCallback(window, key);
	//	make the context current...
	glfwMakeContextCurrent(window);
	glfwSwapInterval( 1 );
	//	load the various GL extensions
	//gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
	glewInit();
	
	//	...at this point, GLFW has been set up and is ready to go.
	
	//	wrap the window's GL context with a GLContext, which we're going to use to create a couple other resources
	GLContextRef	ctxRef = CreateGLContextRefUsing(window);
	//	the global buffer pool will use the shared context to create any GL resources
	CreateGlobalBufferPool(ctxRef);
	
	//	now create the display scene (this is what we're going to use to draw into the GLFWwindow)
	displayScene = CreateGLSceneRefUsing(ctxRef);
	displayScene->setAlwaysNeedsReshape(true);
	displayScene->setClearColor(1., 0., 0., 1.);
	//	set the display scene's render callback, which will draw 'targetTex' in the scene
	displayScene->setRenderCallback([&](const GLScene & s){
		if (targetTex == nullptr)
			return;
		//cout << "\tshould be drawing buffer " << *targetTex << endl;
		glColor4f(1., 1., 1., 1.);
		glActiveTexture(GL_TEXTURE0);
		glEnable(targetTex->desc.target);
		
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
		VVGL::Rect			tmpRect(0.,0.,0.,0.);
		tmpRect.size = s.orthoSize();
		//cout << "\tverts rect is " << tmpRect << endl;
		float			verts[] = {
			(float)tmpRect.minX(), (float)tmpRect.minY(), 0.0,
			(float)tmpRect.minX(), (float)tmpRect.maxY(), 0.0,
			(float)tmpRect.maxX(), (float)tmpRect.maxY(), 0.0,
			(float)tmpRect.maxX(), (float)tmpRect.minY(), 0.0
		};
		tmpRect = targetTex->glReadySrcRect();
		//cout << "\ttex coords are " << tmpRect << endl;
		float			texs[] = {
			(float)tmpRect.minX(), (float)tmpRect.minY(),
			(float)tmpRect.minX(), (float)tmpRect.maxY(),
			(float)tmpRect.maxX(), (float)tmpRect.maxY(),
			(float)tmpRect.maxX(), (float)tmpRect.minY(),
		};
		glVertexPointer(3, GL_FLOAT, 0, verts);
		glTexCoordPointer(2, GL_FLOAT, 0, texs);
		
		//glActiveTexture(GL_TEXTURE0);
		glBindTexture(targetTex->desc.target, targetTex->name);
		
		glDrawArrays(GL_QUADS, 0, 4);
		
		glBindTexture(targetTex->desc.target, 0);
		glDisable(targetTex->desc.target);
	});
	
	
	//	create the ISF scene, which is going to render-to-texture
	isfScene = CreateISFSceneRefUsing(ctxRef);
	NSString		*includedISFPath = [[NSBundle mainBundle] pathForResource:@"CellMod" ofType:@"fs"];
	isfScene->useFile(string([includedISFPath UTF8String]));
	//cout << "loaded ISF doc: " << *(isfScene->doc()) << endl;
	
	
	//	set the size of everything
	int				width, height;
	glfwGetFramebufferSize(window, &width, &height);
	reshape(window, width, height);
	
	
	// Main loop
	while(!glfwWindowShouldClose(window))	{
		targetTex = isfScene->createAndRenderABuffer(displayScene->orthoSize());
		displayScene->render();
		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
		//	tell the buffer pool to do its housekeeping
		//GetGlobalBufferPool()->housekeeping();
	}

	// Terminate GLFW
	glfwTerminate();

	// Exit program
	exit( EXIT_SUCCESS );
}

