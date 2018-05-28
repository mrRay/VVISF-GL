#ifndef VVGL_Doxygen_h
#define VVGL_Doxygen_h




/*!
\defgroup VVGL_SAMPLE VVGL- Sample code

A variety of sample projects that include libs and apps are included with the repos, but some sample code snippets for common or introductory actions in VVGL are listed here as a quick overview:

<BR>
<b>Creating a new GLContext from an existing GLContext (all SDKs)</b>
\code{.cpp}
GLContextRef		origCtx;	//	this is assumed to be non-nil in the real world...
GLContextRef		newCtx = origCtx->newContextSharingMe();
\endcode

<b>Creating a new GLContext- simplest approach, but least control over the kind of context that gets created</b>
\code{.cpp}
GLContextRef		newCtx = CreateNewGLContextRef();
\endcode

<b>Creating GLContext with the mac SDK:</b>
\code{.cpp}
NSOpenGLContext		*origMacCtx;	//	this is assumed to be non-nil in the real world...
CGLContextObj		tmpMacCtx = [origMacCtx CGLContextObj];
CGLPixelFormatObj	tmpMacPxlFmt = [[origMacCtx pixelFormat] CGLPixelFormatObj];

//	this makes a GLContext that wraps (and retains) an existing 
//	mac context (doesn't create a new OpenGL context)
GLContextRef		vvglCtx = CreateGLContextRefUsing(tmpMacCtx, tmpMacCtx, tmpMacPxlFmt);

//	this makes a GLContext that creates a new OpenGL context.  this new 
//	context shares the passed context (they can share resources)
GLContextRef		vvglCtx = CreateNewGLContextRef(tmpMacCtx, tmpMacPxlFmt);

//	if you don't have an existing mac context- if you're creating the first context, for example...

//	this makes a GLContext (and new OpenGL context) using 
//	the compatibility version of GL (GL 2.1 on os x)
GLContextRef		vvglCtx = CreateNewGLContextRef(NULL, CreateCompatibilityGLPixelFormat());

//	this makes a GLContext (and new OpenGL context) using GL4
GLContextRef		vvglCtx = CreateNewGLContextRef(NULL, CreateGL4PixelFormat());
\endcode

<b>Creating GLContext with the iOS SDK:</b>
\code{.cpp}
EAGLContext			*tmpCtx;	//	this is assumed to be non-nil in the real world...

//	this makes a GLContext that wraps (and retains) an existing 
//	iOS context (doesn't create a new OpenGL context)
GLContextRef		vvglCtx = CreateGLContextRefUsing(tmpCtx);
\endcode

<b>Creating GLContext with the GLFW SDK:</b>
\code{.cpp}
GLFWwindow *	window;	//	this is assumed to be non-nil in the real world...

//	this makes a GLContext that wraps the window's GL context 
//	(doesn't create a new OpenGL context)
GLContextRef	ctxRef = CreateGLContextRefUsing(window);
\endcode

<b>Creating GLContext with the Qt SDK:</b>
\code{.cpp}
QSurface *			origSfc;	//	this is assumed to be non-nil in the real world...
QOpenGLContext *	origCtx;	//	this is assumed to be non-nil in the real world...
QSurfaceFormat		origSfcFmt;	//	this is assumed to be non-nil in the real world...

//	this makes a GLContext that wraps and establishes a strong ref to 
//	the passed vars (doesn't create a new OpenGL context)
GLContextRef		vvglCtx = CreateGLContextRefUsing(origSfc, origCtx, origSfcFmt);

//	this makes a GLContext that creates a new OpenGL context.  this new 
//	context shares the passed context (they can share resources)
GLContextRef		vvglCtx = CreateNewGLContextRef(origSfc, origCtx, origSfcFmt);

//	if you don't have an existing Qt context- if you're creating the first context, for example...

//	this makes a GLContext (and new OpenGL context) using the default version of GL...
GLContextRef		vvglCtx = CreateNewGLContextRef(nullptr, nullptr, CreateDefaultSurfaceFormat());

//	this makes a GLContext (and new OpenGL context) using GL 4...
GLContextRef		vvglCtx = CreateNewGLContextRef(nullptr, nullptr, CreateGL4SurfaceFormat());
\endcode


<BR>
<b>Creating the global buffer pool, then a couple GL resources:</b>
\code{.cpp}
//	first create a shared context using one of the above methods (this is just a quick example)
GLContextRef		sharedContext = CreateNewGLContextRef();

//	make the global buffer pool- this line creates the global buffer pool 
//	using the shared context (the buffer pool will use the shared context's 
//	OpenGL context to create or destroy any GL resources).
CreateGlobalBufferPool(sharedContext);

//	make the global buffer pool- this is the same function call, but it 
//	creates a new context (a new OpenGL context) for the buffer pool.
CreateGlobalBufferPool(sharedContext->newContextSharingMe());

//	makes a 1920x1080 GL texture (32 bits per pixel).  the texture 
//	will be created by the global buffer pool's GL context
GLBufferRef			tmpTex = CreateRGBATex(VVGL::Size(1920,1080));

//	makes a 1920x1080 GL texture (32 bits per pixel).  the texture 
//	will be created by whatever GL context is current in the executing thread
GLBufferRef			tmpTex = CreateRGBATex(VVGL::Size(1920,1080), true);

//	makes a 1920x1080 GL texture (128 bits per pixel).
GLBufferRef			tmpTex = CreateRGBAFloatTex(VVGL::Size(1920,1080));
\endcode
*/




/*!
\defgroup VVGL_SAMPLE_II VVGL- Sample Code II

Some examples require more than a few lines of code:

<BR>
<b>Creating and GLScene and rendering it to a texture for older versions of GL:</b>
\code{.cpp}
//	make the shared context, set up the global buffer pool to use it
GLContextRef		sharedContext = CreateNewGLContextRef();
CreateGlobalBufferPool(sharedContext);
//	you can create a scene around an existing context or- as shown here- make a new context for the scene
GLSceneRef			glScene = CreateGLSceneRef();
//	configure the scene's render callback
glScene->setRenderCallback([](const GLScene & inScene)	{
	//	populate a tex quad with the geometry, tex, and color vals- this struct is organized in a manner compatible with GL such that it's both easy to work with and can be uploaded directly.
	Quad<VertXYZRGBA>		texQuad;
	VVGL::Size				sceneSize = inScene.getOrthoSize();
	texQuad.populateGeo(VVGL::Rect(0,0,sceneSize.width,sceneSize.height));
	texQuad.bl.color = GLColor(1., 1., 1., 1.);
	texQuad.tl.color = GLColor(1., 0., 0., 1.);
	texQuad.tr.color = GLColor(0., 1., 0., 1.);
	texQuad.br.color = GLColor(0., 0., 1., 1.);
	
	//	configure GL to expect vertex and color coords when it draws.
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	
	//	set the pointers to the vertex and color coords
	glVertexPointer(texQuad.bl.geo.numComponents(), GL_FLOAT, texQuad.stride(), &texQuad.bl.geo[0]);
	glColorPointer(texQuad.bl.color.numComponents(), GL_FLOAT, texQuad.stride(), &texQuad.bl.color[0]);
	
	//	draw
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
});
//	create a GLBuffer for an OpenGL texture, render the scene to it
GLBufferRef			renderedTexture = CreateRGBATex(VVGL::Size(1920,1080));
glScene->renderToBuffer(renderedTexture);
\endcode


<BR>
<b>Creating a GLScene and rendering it to a texture for desktop GL 3+:</b>
\code{.cpp}
//	make the shared context, set up the global buffer pool to use it
GLContextRef		sharedContext = CreateNewGLContextRef();
CreateGlobalBufferPool(sharedContext);
//	you can create a scene around an existing context or- as shown here- make a new context for the scene
GLSceneRef			glScene = CreateGLSceneRef();
//	create the vertex & fragment shaders, tell the scene to use them
string			vsString("\r\
#version 330 core\r\
in vec3		inXYZ;\r\
in vec4		inRGBA;\r\
uniform mat4	vvglOrthoProj;\r\
out vec4		programRGBA;\r\
void main()	{\r\
	gl_Position = vec4(inXYZ.x, inXYZ.y, inXYZ.z, 1.0) * vvglOrthoProj;\r\
	programRGBA = inRGBA;\r\
}\r\
");
	string			fsString("\r\
#version 330 core\r\
in vec4		programRGBA;\r\
out vec4		FragColor;\r\
void main()	{\r\
FragColor = programRGBA;\r\
}\r\
");
glScene->setVertexShaderString(vsString);
glScene->setFragmentShaderString(fsString);

//	we're going to create a VAO, we also need attribs for the XYZ and RGBA inputs, and quad to store the last-uploaded data
static GLBufferRef				vao = nullptr;
static GLCachedAttribRef		xyzAttr = make_shared<GLCachedAttrib>("inXYZ");
static GLCachedAttribRef		rgbaAttr = make_shared<GLCachedAttrib>("inRGBA");
static Quad<VertXYZRGBA>		vboData = Quad<VertXYZRGBA>();

//	configure the scene's render prep callback to cache the location of the vertex attributes and uniforms
glScene->setRenderPrepCallback([](const GLScene & inScene, const bool & inReshaped, const bool & inPgmChanged)	{
	if (inPgmChanged)	{
		//	cache all the locations for the vertex attributes & uniform locations
		GLint				myProgram = inScene.getProgram();
		xyzAttr->cacheTheLoc(myProgram);
		rgbaAttr->cacheTheLoc(myProgram);
		
		//	make a new VAO
		vao = CreateVAO(true);
	}
});

//	configure the scene's render callback to pass the data to the GL program if it's changed (the params to targetQuad were animated, but i took it out because it wasn't x-platform)
glScene->setRenderCallback([](const GLScene & inScene)	{
	VVGL::Size			orthoSize = inScene.getOrthoSize();
	VVGL::Rect			boundsRect(0, 0, orthoSize.width, orthoSize.height);
	Quad<VertXYZRGBA>		targetQuad;
	targetQuad.populateGeo(boundsRect);
	targetQuad.bl.color = GLColor(1., 1., 1., 1.);
	targetQuad.tl.color = GLColor(1., 0., 0., 1.);
	targetQuad.tr.color = GLColor(0., 1., 0., 1.);
	targetQuad.br.color = GLColor(0., 0., 1., 1.);
	
	//	bind the VAO
	glBindVertexArray(vao->name);
	
	uint32_t			vbo = 0;
	//	if the target quad differs from the vbo data, there's been a change/animation and we need to push new data to the GL program
	if (vboData != targetQuad)	{
		vboData = targetQuad;
		//	make a new VBO to contain vertex + color data
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(targetQuad), (void*)(&targetQuad), GL_STATIC_DRAW);
		//	configure the attribute pointers to use the VBO
		if (xyzAttr->loc >= 0)	{
			glVertexAttribPointer(xyzAttr->loc, targetQuad.bl.geo.numComponents(), GL_FLOAT, GL_FALSE, targetQuad.stride(), BUFFER_OFFSET(targetQuad.geoOffset()));
			xyzAttr->enable();
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
	
	//	if we created a vbo earlier, delete it now (the vao will retain it)
	if (vbo != 0)	{
		glDeleteBuffers(1, &vbo);
	}
});
//	create a GLBuffer for an OpenGL texture, render the scene to it
GLBufferRef			renderedTexture = CreateRGBATex(VVGL::Size(1920,1080));
glScene->renderToBuffer(renderedTexture);
\endcode
*/




/*!
\defgroup VVGL_BASIC VVGL- Basic Classes

\brief These are the basic objects used by VVGL for GL rendering

\detail
	- VVGL::GLContext is a simple cross-platform wrapper for a GL context.  Includes support for shared contexts, which allows GL resources (like buffers, textures, models, etc) to be shared between different OpenGL contexts.
	- VVGL::GLScene provides a simplified interface for performing drawing commands.  Makes it particularly easy to render-to-texture and set up orthogonal drawing.  Every GLScene must be backed by a GLContextRef- each scene can own its own context, or multiple scenes can use the same context.
	- VVGL::GLBuffer is a wrapper for an OpenGL buffer of some sort.  Most of the GLBuffers you'll encounter will be wrappers for GL textures that contain an image of some sort- but a GLBuffer can also be an FBO/VBO/EBO/etc.  When the GLBuffer is released, its underlying GL resources is either deleted or pooled- which is why most of the time you'll probably just work with VVGL::GLBufferRef and the GL resources will delete themselves.
	- VVGL::GLBufferPool creates GLBuffers- it also pools many types of GLBuffers, which offers substantial performance improvements over creating/deleting textures repeatedly.  You'll probably create a single global buffer pool when setting up your app's GL environment, but there's no hard limit on the number of pools you can create (different pools for different GL environments in the same app, for example).
	- VVGL::GLBufferCopier copies the contents of one GLBuffer into another GLBuffer.


*/








/*!
\defgroup VVGL_BUFFERCREATE VVGL- GLBuffer create functions
\brief GLBuffer creation functions

\detail These functions create GLBuffer instances.  Note that these functions all require a GLBufferPool- you need to create a buffer pool before you can create any buffers!
*/








/*!
\defgroup VVGL_GEOM VVGL- Geometry

\brief These are some basic structs used to describe geometry in VVGL and VVISF.

\detail
VVGL does not aim to provide significant GL math or modelling facilities- there are other, and much better, solutions for that sort of thing out there which can be used in conjunction with VVGL.  These basic structs exist because there is a need within this lib for some basic geometric primitives to do things like describe bounding rectangles and two-dimensional points/sizes for texture layout, and some basic quad rendering.
	- VVGL::Point describes a simple 2D point.
	- VVGL::Size describes a simple two-dimensional size.
	- VVGL::Rect describes a rectangle using a VVGL::Point and VVGL::Size.

Additionally, VVGL has a couple structs that can be used to populate OpenGL buffers with geometry/texture/color data.
	- The abstract base struct VVGL::VT has several concrete structs derived from it.  These structs describe the different kinds of data that can be stored in vertices:
		- VVGL::VT_XY
		- VVGL::VT_XYZ
		- VVGL::VT_ST
		- VVGL::VT_RGBA
	- The abstract base struct VVGL::Vertex has several concrete structs derived from it that describe a number of different kinds of vertices that store different kinds of data:
		- VVGL::VertXY
		- VVGL::VertXYST
		- VVGL::VertXYRGBA
		- VVGL::VertXYSTRGBA
		- VVGL::VertXYZ
		- VVGL::VertXYZST
		- VVGL::VertXYZRGBA
		- VVGL::VertXYZSTRGBA
	- The template class VVGL::Quad describes a rectangular quadrilateral.  The vertices of the rect are derived from VVGL::Vertex.
*/








/*!
\defgroup VVGL_MISC VVGL- other classes

\brief Some of the less frequently encountered classes in VVGL- this is the stuff you'll probably be less likely to run into or have to work with if you're here because you just want to get to the ISF goodies.

\detail
	- VVGL::Range
	- VVGL::Timestamp and VVGL::Timestamper
	- There are some basic geometric primitives defined in \ref VVGL_GEOM
	- VVGL::GLCachedProperty, VVGL::GLCachedAttrib and VVGL::GLCachedUni
*/








/*!
\defgroup VVGL_ADD_MORE_SDK VVGL- Adding new SDKs/platforms

This is a rough outline of the procedure necessary to add support for a new SDK or platform to VVGL/VVISF:

- Create a new "VVGL_SDK_****" define for the platform/SDK you want to add.  Open VVGL_Defines.hpp, and add your new SDK define to the if/elif/endif statement, being sure to define which GL environments you want your SDK to be compiled against.  This just defines which GL environments (like GLES/GL4/etc) VVGL/VVISF will be compiled against- you should make sure that all GL environments that will be available at runtime are specified here (VVGL and VVISF support multiple versions of GL at runtime, but when you compile these libs you need to ensure that they include code for the relevant versions of OpenGL you'll be working with).  You should probably add the define you created to "VVGL_HardCodedDefines.hpp", too.
- Locate GLContext.hpp and GLContext.cpp.  GLContext is a platform-agnostic representation of a GL context- you need to use the VVGL_SDK_XXX define you created to populate the instance variables and methods of GLContext with the SDK-specific variables it needs to interact with OpenGL in your target SDK.  Typically, this means that GLContext needs a strong reference to your SDK-specific GL context object.  GLContext.hpp and GLContext.cpp are populated with source code for at least four different SDKs, all of which are very similar to one another and can be used as an exemplar.
- The texture types available differ from platform to platform- for convenience, you'll want to create a header file that lists the enums for the OpenGL texture targets/internal formats/pixel formats/pixel types available in your SDK.  You should pattern this header file after one of the existing header files (GLBuffer_XXXX_Enums.h), and ensure that it's included in GLBuffer.hpp using the VVGL_SDK define you created.
- GLBufferPool.hpp declares a variety of functions that create GL textures, returned as GLBufferRef instances.  These functions are defined in GLBufferPool.cpp, and you should check their implementations briefly to ensure that the resources they're creating are available on your platform (it's likely that you'll get a compiler error if they aren't, but it can't hurt to check).
- Send me a pull request!
*/




#endif /* VVGL_Doxygen_h */
