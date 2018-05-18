#ifndef VVGL_h
#define VVGL_h




/*!
\defgroup VVGL_SAMPLE VVGL- Sample code

A variety of sample projects that include libs and apps are included with the repos, but some sample code snippets for common or introductory actions in VVGL are listed here as a quick overview:

<BR>
Creating a new GLContext from an existing GLContext (all SDKs)
\code{.cpp}
GLContextRef		origCtx;	//	this is assumed to be non-nil in the real world...
GLContextRef		newCtx = origCtx->newContextSharingMe();
\endcode

Creating a new GLContext- simplest approach, but least control over the kind of context that gets created
\code{.cpp}
GLContextRef		newCtx = CreateNewGLContextRef();
\endcode

Creating GLContext with the mac SDK:
\code{.cpp}
NSOpenGLContext		*origMacCtx;	//	this is assumed to be non-nil in the real world...
CGLContextObj		tmpMacCtx = [origMacCtx CGLContextObj];
CGLPixelFormatObj	tmpMacPxlFmt = [[origMacCtx pixelFormat] CGLPixelFormatObj];

//	this makes a GLContext that wraps (and retains) an existing mac context (doesn't create a new OpenGL context)
GLContextRef		vvglCtx = CreateGLContextRefUsing(tmpMacCtx, tmpMacCtx, tmpMacPxlFmt);

//	this makes a GLContext that creates a new OpenGL context.  this new context shares the passed context (they can share resources)
GLContextRef		vvglCtx = CreateNewGLContextRef(tmpMacCtx, tmpMacPxlFmt);

//	if you don't have an existing mac context- like if you're creating the first context...

//	this makes a GLContext (and new OpenGL context) using the compatibility version of GL (GL 2.1 on os x)
GLContextRef		vvglCtx = CreateNewGLContextRef(NULL, CreateCompatibilityGLPixelFormat());

//	this makes a GLContext (and new OpenGL context) using GL4
GLContextRef		vvglCtx = CreateNewGLContextRef(NULL, CreateGL4PixelFormat());
\endcode

Creating GLContext with the iOS SDK:
\code{.cpp}
EAGLContext			*tmpCtx;	//	this is assumed to be non-nil in the real world...

//	this makes a GLContext that wraps (and retains) an existing iOS context (doesn't create a new OpenGL context)
GLContextRef		vvglCtx = CreateGLContextRefUsing(tmpCtx);
\endcode

Creating GLContext with the GLFW SDK:
\code{.cpp}
GLFWwindow *	window;	//	this is assumed to be non-nil in the real world...

//	this makes a GLContext that wraps the window's GL context (doesn't create a new OpenGL context)
GLContextRef	ctxRef = CreateGLContextRefUsing(window);
\endcode

Creating GLContext with the Qt SDK:
\code{.cpp}
QSurface *			origSfc;	//	this is assumed to be non-nil in the real world...
QOpenGLContext *	origCtx;	//	this is assumed to be non-nil in the real world...
QSurfaceFormat		origSfcFmt;	//	this is assumed to be non-nil in the real world...

//	this makes a GLContext that wraps and establishes a strong ref to the passed vars (doesn't create a new OpenGL context)
GLContextRef		vvglCtx = CreateGLContextRefUsing(origSfc, origCtx, origSfcFmt);

//	this makes a GLContext that creates a new OpenGL context.  this new context shares the passed context (they can share resources)
GLContextRef		vvglCtx = CreateNewGLContextRef(origSfc, origCtx, origSfcFmt);
\endcode


<BR>
Creating the global buffer pool
\code{.cpp}
//	first create a shared context using one of the above methods (this is just a quick example)
GLContextRef		sharedContext = CreateNewGLContextRef();

//	make the global buffer pool- this line creates the global buffer pool using the shared context (the buffer pool will use the shared context's OpenGL context to create or destroy any GL resources).
CreateGlobalBufferPool(sharedContext);

//	make the global buffer pool- this is the same function call, but it creates a new context (a new OpenGL context) for the buffer pool.
CreateGlobalBufferPool(sharedContext->newContextSharingMe());
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








//	base SDK defines.  the SDK will define the range of GL calls that are available (not all flavors of GL are available in all SDKs- for example, you're not going to get GL3+ on a raspberry pi, nor are you likely to see GL ES on a desktop).
#include "VVGL_Defines.hpp"


//	base types- basic time structs, range structs, string utils, geometry types, things like that
#include "VVGL_Base.hpp"


//	GLContext is a GL context- this class wraps up whatever the native object is for whatever platform you're compiling against, and presents a single standard interface across all platforms for the rest of VVGL/VVISF.  you can create a GLContext around an existing platform-specific OpenGL context, and you can also create new GL contexts.  explicit support for sharing of multiple contexts so resources can be shared.  you'll probably work almost exclusively with GLContextRef, which is a std::shared_ptr around a GLContext.
#include "GLContext.hpp"
#include "GLCachedProperty.hpp"


#include "GLBuffer.hpp"


//	a pool for recycling GL objects.  should be in the same sharegroup as all your other GL contexts.  has its own GL context, so it can create/delete GL resources (and can thus act as a sort of autorelease pool for GLBufferRefs)
#include "GLBufferPool.hpp"


#include "GLScene.hpp"


//	a GLBufferCopier will copy texture-based GLBuffer instances into other texture-based GLBuffer instances.  it's a GL texture copier, basically.  it has some frills for easily resizing textures or creating textures filled with a solid color.
#include "GLBufferCopier.hpp"


//	if we're compiling against the os x sdk then we've got some cocoa additions.  os x, not ios.
#if defined(VVGL_SDK_MAC) && defined(__OBJC__)
#include "GLBufferPool_CocoaAdditions.h"
#endif




#endif /* VVGL_h */
