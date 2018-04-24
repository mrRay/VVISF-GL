
#if defined(_MSC_VER)
 // Make MS math.h define M_PI
 #define _USE_MATH_DEFINES
#endif

#include <stdlib.h>
#include <stdio.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "VVGL.hpp"
#include "VVISF.h"




VVGL::GLScene			*displayScene = nullptr;
VVISF::ISFScene		*isfScene = nullptr;
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
		targetTex = isfScene->createAndRenderABuffer(displayScene->getSize());
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
	cout << __FUNCTION__ << ", " << width << " x " << height << endl;
	if (displayScene==nullptr || isfScene==nullptr)
		return;
	VVGL::Size			tmpSize(width,height);
	displayScene->setSize(tmpSize);
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
	//	load the various GL extensions
	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
	glfwSwapInterval( 1 );
	
	//	...at this point, GLFW has been set up and is ready to go.
	
	//	wrap the window's GL context with a GLContext, which we're going to use to screate a couple other resources
	GLContextRef	ctxRef = CreateRefUsing(window);
	//	first make the buffer pool
	CreateGlobalBufferPool(ctxRef);
	
	
	
	
	//	now create the display scene (this is what we're going to use to draw into the GLFWwindow)
	displayScene = new GLScene(ctxRef);
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
		tmpRect.size = s.getSize();
		//cout << "\tverts rect is " << tmpRect << endl;
		float			verts[] = {
			(float)MinX(tmpRect), (float)MinY(tmpRect), 0.0,
			(float)MinX(tmpRect), (float)MaxY(tmpRect), 0.0,
			(float)MaxX(tmpRect), (float)MaxY(tmpRect), 0.0,
			(float)MaxX(tmpRect), (float)MinY(tmpRect), 0.0
		};
		tmpRect = targetTex->glReadySrcRect();
		//cout << "\ttex coords are " << tmpRect << endl;
		float			texs[] = {
			(float)MinX(tmpRect), (float)MinY(tmpRect),
			(float)MinX(tmpRect), (float)MaxY(tmpRect),
			(float)MaxX(tmpRect), (float)MaxY(tmpRect),
			(float)MaxX(tmpRect), (float)MinY(tmpRect),
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
	isfScene = new ISFScene(ctxRef);
	isfScene->useFile(string("/Users/testAdmin/Documents/ISFSandbox/examples/apple/GLFWGears/CellMod.fs"));
	cout << "loaded ISF doc: " << *(isfScene->getDoc()) << endl;
	
	
	//	set the size of everything
	int				width, height;
	glfwGetFramebufferSize(window, &width, &height);
	reshape(window, width, height);
	
	
	// Main loop
	while(!glfwWindowShouldClose(window))	{
		targetTex = isfScene->createAndRenderABuffer(displayScene->getSize());
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

