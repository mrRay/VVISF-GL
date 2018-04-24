#include <QGuiApplication>
#include <VVGL.hpp>
#include <QDebug>
#include <QImage>
#include "VVBufferGLWindow.h"
#include <QCoreApplication>
#include <QTime>

int main(int argc, char *argv[])
{
	QGuiApplication a(argc, argv);
	qDebug()<<"on launch, current thread is "<<QThread::currentThread()<<", main thread is "<<QCoreApplication::instance()->thread();

	using namespace VVGL;

	//	figure out what version GL we're going to use
	QSurfaceFormat		sfcFmt = CreateDefaultSurfaceFormat();
	//QSurfaceFormat		sfcFmt = CreateGL4SurfaceFormat();
	
	//	make the shared context using the vsn of GL you need to target.  all GL contexts are going to share this so they can share textures/etc with one another
	GLContextRef		sharedContext = CreateNewGLContext(nullptr, nullptr, sfcFmt);
	
	//	make the global buffer pool.  if there's a global buffer pool, GL resources can be recycled and runtime performance is much better.
	CreateGlobalBufferPool(sharedContext);
	
	
	//	load the image file we include with the sample app, convert it to a VVGLBufferRef
	QImage		tmpImg(":/files/SampleImg.png");
	GLBufferRef		imgBuffer = CreateBufferForQImage(&tmpImg);
	
	//	make the window, open it, give it an initial buffer to draw
	VVBufferGLWindow			window(sharedContext);
	window.setFormat(sfcFmt);
	window.resize(QSize(800,600));
	window.show();
	window.startRendering();
	window.drawBuffer(imgBuffer);
	
	//	move the buffer pool's context to the window's render thread
	GetGlobalBufferPool()->getContext()->moveToThread(window.getRenderThread());
	
	
	
	//	set up the render-to-texture scene.  this is big because we're settup up different draw methods for different GL flavors; you probably only need one.
	GLSceneRef		renderScene = nullptr;
	GLBufferRef		vao = nullptr;
	Quad<VertXYST>		lastVBOCoords;	//	the last coords used in the VBO associated with 'vao' (the VAO implicitly retains the VBO, so we only need to update it when the coords change- which we track with this)
	{
		QTime				myTimer;
		myTimer.start();
		//	make a new scene.  we're going to use this scene to render-to-texture, and we're going to display the texture.
		renderScene = CreateGLScene(sharedContext->newContextSharingMe());
		//	move the render scene to the window's render thread!
		renderScene->getContext()->moveToThread(window.getRenderThread());
		//	set up the render scene's draw callback, depending on the version of GL in use
		//	if the shared context (from which all other contexts derive) is using GL 2...
		if (sharedContext->version == GLVersion_2)	{
			renderScene->setRenderCallback([imgBuffer,&myTimer](const GLScene & n)	{
				//	populate a tex quad with the geometry & tex coords
				Quad<VertXYST>			texQuad;
				VVGL::Size				sceneSize = n.getOrthoSize();
				//VVGL::Rect				geoRect(0, 0, sceneSize.width, sceneSize.height);
				VVGL::Rect				geoRect = ResizeRect(imgBuffer->srcRect, VVGL::Rect(0,0,sceneSize.width,sceneSize.height), SizingMode_Fit);
				VVGL::Rect				texRect = imgBuffer->glReadySrcRect();
				texQuad.populateGeo(geoRect);
				texQuad.populateTex(texRect, imgBuffer->flipped);
		
				//	draw the VVGLBufferRef we created from the PNG, using the tex quad
				glEnable(imgBuffer->desc.target);
				glEnableClientState(GL_VERTEX_ARRAY);
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				glVertexPointer(2, GL_FLOAT, texQuad.stride(), &texQuad);
				glTexCoordPointer(2, GL_FLOAT, texQuad.stride(), &texQuad.bl.tex[0]);
				glBindTexture(imgBuffer->desc.target, imgBuffer->name);
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
				glBindTexture(imgBuffer->desc.target, 0);
		
				//	we're going to draw a quad "over" the image at a varying opacity
				int					msSinceStart = myTimer.elapsed();
				double				timeSinceStart = (double)msSinceStart/1000.;
				//double					timeSinceStart = [[(id)selfPtr date] timeIntervalSinceNow] * -1.;
				GLfloat					opacity = fmod(timeSinceStart, 1.);
				//cout << "\topacity is " << opacity << endl;
				Quad<VertXY>			colorQuad;
				colorQuad.populateGeo(geoRect);
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);
				glVertexPointer(2, GL_FLOAT, texQuad.stride(), &texQuad);
				glColor4f(0., 0., 0., opacity);
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			});
		}
		//	else if the shared context is using something newer than GL2
		else if (sharedContext->version >= GLVersion_ES2)	{
#if defined(ISF_TARGETENV_GL3PLUS) || defined(ISF_TARGETENV_GLES3)
			string			vsString("\r\
#version 330 core\r\
in vec3		inXYZ;\r\
in vec2		inST;\r\
uniform mat4	vvglOrthoProj;\r\
out vec2		programST;\r\
void main()	{\r\
	gl_Position = vec4(inXYZ.x, inXYZ.y, inXYZ.z, 1.0) * vvglOrthoProj;\r\
	programST = inST;\r\
}\r\
");
			string			fsString("\r\
#version 330 core\r\
in vec2		programST;\r\
uniform sampler2D		inputImage;\r\
uniform sampler2DRect	inputImageRect;\r\
uniform int		isRectTex;\r\
uniform float	fadeVal;\r\
out vec4		FragColor;\r\
void main()	{\r\
if (isRectTex==0)\r\
	FragColor = vec4(0,0,0,1);\r\
else if (isRectTex==1)\r\
	FragColor = texture(inputImage,programST);\r\
else\r\
	FragColor = texture(inputImageRect,programST);\r\
FragColor *= (1.-fadeVal);\r\
}\r\
");
			renderScene->setVertexShaderString(vsString);
			renderScene->setFragmentShaderString(fsString);
			
			//	we're going to create a couple vars on the stack here- the vars themselves are shared 
			//	ptrs, so when they're copied by value in the callback blocks the copies will refer to 
			//	the same underlying vars, which will be retained until these callback blocks are 
			//	destroyed and shared between the callback lambdas...
			GLCachedAttribRef		xyzAttr = make_shared<GLCachedAttrib>("inXYZ");
			GLCachedAttribRef		stAttr = make_shared<GLCachedAttrib>("inST");
			GLCachedUniRef		inputImageUni = make_shared<GLCachedUni>("inputImage");
			GLCachedUniRef		inputImageRectUni = make_shared<GLCachedUni>("inputImageRect");
			GLCachedUniRef		isRectTexUni = make_shared<GLCachedUni>("isRectTex");
			GLCachedUniRef		fadeValUni = make_shared<GLCachedUni>("fadeVal");
		
			//	the render prep callback needs to cache the location of the vertex attributes and uniforms
			renderScene->setRenderPrepCallback([=,&vao](const GLScene & n, const bool & inReshaped, const bool & inPgmChanged)	{
				//cout << __PRETTY_FUNCTION__ << endl;
				if (inPgmChanged)	{
					//	cache all the locations for the vertex attributes & uniform locations
					GLint				myProgram = n.getProgram();
					xyzAttr->cacheTheLoc(myProgram);
					stAttr->cacheTheLoc(myProgram);
					inputImageUni->cacheTheLoc(myProgram);
					inputImageRectUni->cacheTheLoc(myProgram);
					isRectTexUni->cacheTheLoc(myProgram);
					fadeValUni->cacheTheLoc(myProgram);
	
					//	make a quad struct that describes XYST geometry.  we don't have to populate it now (we'll update it during the render pass)
					//Quad<VertXYST>	targetQuad;
			
					//	make a new VAO
					vao = CreateVAO(true);
				}
			});
	
			//	the render callback passes all the data to the GL program
			renderScene->setRenderCallback([=,&vao,&lastVBOCoords](const GLScene & n)	{
				//cout << __PRETTY_FUNCTION__ << endl;
				if (imgBuffer == nullptr)
					return;

				//	clear
				glClearColor(0., 0., 0., 1.);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				VVGL::Size			orthoSize = n.getOrthoSize();
				VVGL::Rect			boundsRect(0, 0, orthoSize.width, orthoSize.height);
				VVGL::Rect			geometryRect = ResizeRect(imgBuffer->srcRect, boundsRect, SizingMode_Fit);
				Quad<VertXYST>		targetQuad;
				targetQuad.populateGeo(geometryRect);
				targetQuad.populateTex((imgBuffer==nullptr) ? geometryRect : imgBuffer->glReadySrcRect(), (imgBuffer==nullptr) ? false : imgBuffer->flipped);

				//	pass the 2D texture to the program (if there's a 2D texture)
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GLBuffer::Target_2D, (imgBuffer!=nullptr && imgBuffer->desc.target==GLBuffer::Target_2D) ? imgBuffer->name : 0);
				glBindTexture(GLBuffer::Target_Rect, 0);
				if (inputImageUni->loc >= 0)	{
					glUniform1i(inputImageUni->loc, 0);
				}
				//	pass the RECT texture to the program (if there's a RECT texture)
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GLBuffer::Target_2D, 0);
				glBindTexture(GLBuffer::Target_Rect, (imgBuffer!=nullptr && imgBuffer->desc.target==GLBuffer::Target_Rect) ? imgBuffer->name : 0);
				if (inputImageRectUni->loc >= 0)	{
					glUniform1i(inputImageRectUni->loc, 1);
				}
				//	pass an int to the program that indicates whether we're passing no texture (0), a 2D texture (1) or a RECT texture (2)
				if (isRectTexUni->loc >= 0)	{
					if (imgBuffer == nullptr)
						glUniform1i(isRectTexUni->loc, 0);
					else	{
						switch (imgBuffer->desc.target)	{
						case GLBuffer::Target_2D:
							glUniform1i(isRectTexUni->loc, 1);
							break;
						case GLBuffer::Target_Rect:
							glUniform1i(isRectTexUni->loc, 2);
							break;
						default:
							glUniform1i(isRectTexUni->loc, 0);
							break;
						}
					}
				}
				//	pass the fade val to the program
				if (fadeValUni->loc >= 0)	{
					int					msSinceStart = myTimer.elapsed();
					double				timeSinceStart = (double)msSinceStart/1000.;
					GLfloat					opacity = fmod(timeSinceStart, 1.);
					//double					timeSinceStart = [[(id)selfPtr date] timeIntervalSinceNow] * -1.;
					//GLfloat					opacity = fmod(timeSinceStart, 1.);
					glUniform1f(fadeValUni->loc, opacity);
				}
		
				//	bind the VAO
				glBindVertexArray(vao->name);
		
				uint32_t			vbo = 0;
				if (lastVBOCoords != targetQuad)	{
					//	make a new VBO to contain vertex + texture coord data
					glGenBuffers(1, &vbo);
					glBindBuffer(GL_ARRAY_BUFFER, vbo);
					glBufferData(GL_ARRAY_BUFFER, sizeof(targetQuad), (void*)&targetQuad, GL_STATIC_DRAW);
					//	configure the attribute pointers to use the VBO
					if (xyzAttr->loc >= 0)	{
						glVertexAttribPointer(xyzAttr->loc, 2, GL_FLOAT, GL_FALSE, targetQuad.stride(), BUFFER_OFFSET(targetQuad.geoOffset()));
						xyzAttr->enable();
					}
					if (stAttr->loc >= 0)	{
						glVertexAttribPointer(stAttr->loc, 2, GL_FLOAT, GL_FALSE, targetQuad.stride(), BUFFER_OFFSET(targetQuad.texOffset()));
						stAttr->enable();
					}
				}
		
				//	draw
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
				//	un-bind the VAO
				glBindVertexArray(0);
		
				if (lastVBOCoords != targetQuad)	{
					//	delete the VBO we made earlier...
					glDeleteBuffers(1, &vbo);
					//	update the vbo coords ivar (we don't want to update the VBO contents every pass)
					lastVBOCoords = targetQuad;
				}
		
			});
#endif	//	ISF_TARGETENV_GL3PLUS || ISF_TARGETENV_GLES3
		}
	}
	
	
	
	//	the window has its own thread on which it drives rendering- it emits a signal after each frame, which we're going to use to drive rendering with this lambda.
	QObject::connect(&window, &VVBufferGLWindow::renderedAFrame, [&window,renderScene](){
		//	size the target texture so it's the same size as the window
		double				ltbbm = window.devicePixelRatio();
		VVGL::Size			windowSize = VVGL::Size(window.width()*ltbbm, window.height()*ltbbm);
		GLBufferRef		newBuffer = CreateRGBATex(windowSize,true);
		//	tell the scene to render to the target texture
		renderScene->renderToBuffer(newBuffer);
		//	tell the window to draw the texture we just rendered!
		window.drawBuffer(newBuffer);
	});
	
	return a.exec();
}
