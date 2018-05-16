#ifndef VVGL_h
#define VVGL_h








/*!
\defgroup VVGL_BASIC VVGL Basics

\brief These are the basic objects used by VVGL for GL rendering

\detail
	- VVGL::GLContext is a simple cross-platform wrapper for a GL context.  Includes support for shared contexts, which allows GL resources (like buffers, textures, models, etc) to be shared between different OpenGL contexts.
	- VVGL::GLScene provides a simplified interface for performing drawing commands.  Makes it particularly easy to render-to-texture and set up orthogonal drawing.  Every GLScene must be backed by a GLContextRef- each scene can own its own context, or multiple scenes can use the same context.
	- VVGL::GLBuffer is a wrapper for an OpenGL buffer of some sort.  Most of the GLBuffers you'll encounter will be wrappers for GL textures that contain an image of some sort- but a GLBuffer can also be an FBO/VBO/EBO/etc.  When the GLBuffer is released, its underlying GL resources is either deleted or pooled- which is why most of the time you'll probably just work with VVGL::GLBufferRef and the GL resources will delete themselves.
	- VVGL::GLBufferPool creates GLBuffers- it also pools many types of GLBuffers, which offers substantial performance improvements over creating/deleting textures repeatedly.  You'll probably create a single global buffer pool when setting up your app's GL environment, but there's no hard limit on the number of pools you can create (different pools for different GL environments in the same app, for example).
	- VVGL::GLBufferCopier copies the contents of one GLBuffer into another GLBuffer.

Sample Code for making a global buffer pool using a shared context:
\code{.cpp}
//	make a shared context- this example just makes a simple GL context, you can wrap a GLContext 
//	around an existing platform-specific OpenGL context or use one of the various 
//	platform-specific functions to create an OpenGL context that meets your requirements.
GLContextRef		sharedContext = CreateNewGLContextRef();

//	make the global buffer pool.  if there's a global buffer pool, calls to create 
//	textures/etc will be shorter and the API will be easier to use.
CreateGlobalBufferPool(sharedContext);

//	at this point, the global buffer pool exists, and can be used to create 
//	buffers/textures/etc.  This line creates a 320x240 RGBA texture:
GLBufferRef			rgbaTexture = CreateRGBATex(VVGL::Size(320,240));
\endcode

Sample code for XXXXX
*/




/*!
\defgroup VVGL_BUFFERCREATE GLBuffer creation functions
\brief GLBuffer creation functions

\detail These functions create GLBuffer instances.  Note that these functions all require a GLBufferPool- you need to create a buffer pool before you can create any buffers!
*/




/*!
\defgroup VVGL_MISC Other VVGL Objects

\brief Some of the less frequently encountered classes in VVGL- this is the stuff you'll probably be less likely to run into or have to work with if you're here because you just want to get to the ISF goodies.

\detail
	- VVGL::Range
	- VVGL::Timestamp and VVGL::Timestamper
	- There are some basic geometric primitives defined in \ref VVGL_GEOM
	- VVGL::GLCachedProperty, VVGL::GLCachedAttrib and VVGL::GLCachedUni
*/




/*!
\defgroup VVGL_GEOM VVGL Geometry

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
