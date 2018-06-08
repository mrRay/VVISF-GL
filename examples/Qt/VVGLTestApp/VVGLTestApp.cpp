#include <QGuiApplication>
#include <VVGL.hpp>
#include <QDebug>
#include <QImage>
#include "GLBufferQWindow.h"
#include <QCoreApplication>
#include <QTime>
#include <QTimer>


int main(int argc, char *argv[])
{
	QGuiApplication a(argc, argv);

	using namespace VVGL;

	//	figure out what version GL we're going to use
	QSurfaceFormat		sfcFmt = CreateDefaultSurfaceFormat();
	//QSurfaceFormat		sfcFmt = CreateGL4SurfaceFormat();
	
	//	make the shared context using the vsn of GL you need to target.  all GL contexts are going to share this so they can share textures/etc with one another
	GLContextRef		sharedContext = CreateNewGLContextRef(nullptr, nullptr, sfcFmt);
	
	//	make the global buffer pool.  buffer pools create GL resources, so you need one- they also recycle these resources, improving runtime performance.  this global buffer pool will use the shared context to create any GL resources
	CreateGlobalBufferPool(sharedContext);
	
	
	//	load the image file we include with the sample app, convert it to a VVGLBufferRef
	QImage				tmpImg(":/files/SampleImg.png");
	GLBufferRef			imgBuffer = CreateBufferForQImage(&tmpImg);
	
	//	make the window, open it, give it an initial buffer to draw
	GLBufferQWindow			window(sharedContext);
	window.setFormat(sfcFmt);
	window.resize(QSize(800,600));
	window.show();
	window.startRendering();
	window.drawBuffer(imgBuffer);
	
	//	set up the render-to-texture scene.  this is big because we're settup up different draw methods for different GL flavors; you probably only need one.
	GLSceneRef		renderScene = nullptr;
	GLBufferRef		vao = nullptr;
	Quad<VertXYZSTRGBA>		lastVBOCoords;	//	the last coords used in the VBO associated with 'vao' (the VAO implicitly retains the VBO, so we only need to update it when the coords change- which we track with this)
	{
		QTime				myTimer;
		myTimer.start();
		//	make a new scene.  we're going to use this scene to render-to-texture, and we're going to display the texture.
		renderScene = CreateGLSceneRefUsing(sharedContext->newContextSharingMe());
		//	set up the render scene's draw callback, depending on the version of GL in use
		//	if the shared context (from which all other contexts derive) is using GL 2...
		if (sharedContext->version == GLVersion_2)	{
#if defined(VVGL_TARGETENV_GL2)
			renderScene->setRenderPrepCallback([&](const GLScene & n, const bool inReshaped, const bool inPgmChanged)	{
				/*
				//	intentionally blank
				*/
				
				glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
				
			});
			renderScene->setRenderCallback([imgBuffer,&myTimer](const GLScene & n)	{
				/*
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
				*/
				
				
				//	figure out how long we've been rendering, modulate it at one second
				//double					timeSinceStart = [[(id)selfPtr date] timeIntervalSinceNow] * -1.;
				int						msSinceStart = myTimer.elapsed();
				double					timeSinceStart = (double)msSinceStart/1000.;
				double					fillerColorVal = fmod(timeSinceStart, 1.);
				//	populate a tex quad with the geometry, tex, and color vals
				Quad<VertXYZSTRGBA>		texQuad;
				VVGL::Size				sceneSize = n.getOrthoSize();
				VVGL::Rect				geoRect = ResizeRect(imgBuffer->srcRect, VVGL::Rect(0,0,sceneSize.width,sceneSize.height), SizingMode_Fit);
				VVGL::Rect				texRect = imgBuffer->glReadySrcRect();
				texQuad.populateGeo(geoRect);
				texQuad.populateTex(texRect, imgBuffer->flipped);
				texQuad.bl.color = GLColor(1., 1., 1., 1.);
				texQuad.tl.color = GLColor(1., fillerColorVal, fillerColorVal, 1.);
				texQuad.tr.color = GLColor(fillerColorVal, 1., fillerColorVal, 1.);
				texQuad.br.color = GLColor(fillerColorVal, fillerColorVal, 1., 1.);
		
				//	draw the GLBufferRef we created from the PNG, using the tex quad
				glEnable(imgBuffer->desc.target);
				glEnableClientState(GL_VERTEX_ARRAY);
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				glEnableClientState(GL_COLOR_ARRAY);
		
				glVertexPointer(texQuad.bl.geo.numComponents(), GL_FLOAT, texQuad.stride(), &texQuad.bl.geo[0]);
				glTexCoordPointer(texQuad.bl.tex.numComponents(), GL_FLOAT, texQuad.stride(), &texQuad.bl.tex[0]);
				glColorPointer(texQuad.bl.color.numComponents(), GL_FLOAT, texQuad.stride(), &texQuad.bl.color[0]);
		
				glBindTexture(imgBuffer->desc.target, imgBuffer->name);
		
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		
				glBindTexture(imgBuffer->desc.target, 0);
				
			});
#endif	// defined(VVGL_TARGETENV_GL2)
		}
		//	else if the shared context is using something newer than GL2
		else if (sharedContext->version >= GLVersion_ES2)	{

#if defined(VVGL_TARGETENV_GL3PLUS) || defined(VVGL_TARGETENV_GLES3)
			/*
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
			renderScene->setRenderPrepCallback([=,&vao](const GLScene & n, const bool &, const bool & inPgmChanged)	{
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
			*/
			
			string			vsString("\r\
#version 330 core\r\
in vec3		inXYZ;\r\
in vec2		inST;\r\
in vec4		inRGBA;\r\
uniform mat4	vvglOrthoProj;\r\
out vec2		programST;\r\
out vec4		programRGBA;\r\
void main()	{\r\
	gl_Position = vec4(inXYZ.x, inXYZ.y, inXYZ.z, 1.0) * vvglOrthoProj;\r\
	programST = inST;\r\
	programRGBA = inRGBA;\r\
}\r\
");
			string			fsString("\r\
#version 330 core\r\
in vec2		programST;\r\
in vec4		programRGBA;\r\
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
FragColor *= programRGBA;\r\
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
			GLCachedAttribRef		rgbaAttr = make_shared<GLCachedAttrib>("inRGBA");
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
					rgbaAttr->cacheTheLoc(myProgram);
					inputImageUni->cacheTheLoc(myProgram);
					inputImageRectUni->cacheTheLoc(myProgram);
					isRectTexUni->cacheTheLoc(myProgram);
					fadeValUni->cacheTheLoc(myProgram);
			
					//	make a new VAO
					vao = CreateVAO(true);
				}
			});
	
			//	the render callback passes all the data to the GL program
			renderScene->setRenderCallback([=,&vao,&lastVBOCoords](const GLScene & n)	{
				//cout << __PRETTY_FUNCTION__ << endl;
				if (imgBuffer == nullptr)
					return;
		
				//	figure out how long we've been rendering, modulate it at one second
				//double					timeSinceStart = [[(id)selfPtr date] timeIntervalSinceNow] * -1.;
				int						msSinceStart = myTimer.elapsed();
				double					timeSinceStart = (double)msSinceStart/1000.;
				double					fillerColorVal = fmod(timeSinceStart, 1.);
		
				//	clear
				glClearColor(0., 0., 0., 1.);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
				VVGL::Size			orthoSize = n.getOrthoSize();
				VVGL::Rect			boundsRect(0, 0, orthoSize.width, orthoSize.height);
				VVGL::Rect			geometryRect = ResizeRect(imgBuffer->srcRect, boundsRect, SizingMode_Fit);
				Quad<VertXYZSTRGBA>		targetQuad;
				targetQuad.populateGeo(geometryRect);
				targetQuad.populateTex((imgBuffer==nullptr) ? geometryRect : imgBuffer->glReadySrcRect(), (imgBuffer==nullptr) ? false : imgBuffer->flipped);
				targetQuad.bl.color = GLColor(1., 1., 1., 1.);
				targetQuad.tl.color = GLColor(1., fillerColorVal, fillerColorVal, 1.);
				targetQuad.tr.color = GLColor(fillerColorVal, 1., fillerColorVal, 1.);
				targetQuad.br.color = GLColor(fillerColorVal, fillerColorVal, 1., 1.);
		
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
					//double					timeSinceStart = [[(id)selfPtr date] timeIntervalSinceNow] * -1.;
					GLfloat					opacity = fmod(timeSinceStart, 1.);
					glUniform1f(fadeValUni->loc, opacity);
				}
		
				//	bind the VAO
				//GLBufferRef		tmpVAO = [(id)selfPtr vao];
				glBindVertexArray(vao->name);
		
				uint32_t			vbo = 0;
				if (lastVBOCoords != targetQuad)	{
					//	make a new VBO to contain vertex + texture coord data
					glGenBuffers(1, &vbo);
					glBindBuffer(GL_ARRAY_BUFFER, vbo);
					glBufferData(GL_ARRAY_BUFFER, sizeof(targetQuad), (void*)&targetQuad, GL_STATIC_DRAW);
					//	configure the attribute pointers to use the VBO
					if (xyzAttr->loc >= 0)	{
						glVertexAttribPointer(xyzAttr->loc, targetQuad.bl.geo.numComponents(), GL_FLOAT, GL_FALSE, targetQuad.stride(), BUFFER_OFFSET(targetQuad.geoOffset()));
						xyzAttr->enable();
					}
					if (stAttr->loc >= 0)	{
						glVertexAttribPointer(stAttr->loc, targetQuad.bl.tex.numComponents(), GL_FLOAT, GL_FALSE, targetQuad.stride(), BUFFER_OFFSET(targetQuad.texOffset()));
						stAttr->enable();
					}
					if (rgbaAttr->loc >= 0)	{
						glVertexAttribPointer(rgbaAttr->loc, targetQuad.bl.color.numComponents(), GL_FLOAT, GL_FALSE, targetQuad.stride(), BUFFER_OFFSET(targetQuad.colorOffset()));
						rgbaAttr->enable();
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
			
#endif	//	VVGL_TARGETENV_GL3PLUS || VVGL_TARGETENV_GLES3

		}
	}
	
	
	
	//	the window emits a signal after each frame, which we're going to use to drive rendering with this lambda.
	QObject::connect(&window, &GLBufferQWindow::renderedAFrame, [&window,renderScene](){
		//qDebug() << "\twindow rendered a frame block";

		//	size the target texture so it's the same size as the window
		double				ltbbm = window.devicePixelRatio();
		VVGL::Size			windowSize = VVGL::Size(window.width()*ltbbm, window.height()*ltbbm);
		GLBufferRef			newBuffer = CreateRGBATex(windowSize,true);
		//	tell the scene to render to the target texture
		renderScene->renderToBuffer(newBuffer);
		//	tell the window to draw the texture we just rendered!
		window.drawBuffer(newBuffer);

	});
	

	return a.exec();
}
