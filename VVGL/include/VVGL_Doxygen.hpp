#ifndef VVGL_Doxygen_h
#define VVGL_Doxygen_h








//! These are the basic objects used by VVGL for GL rendering
/*!
\defgroup VVGL_BASIC VVGL- Basic Classes
	- VVGL::GLContext is a simple cross-platform wrapper for a GL context.  Includes support for shared contexts, which allows GL resources (like buffers, textures, models, etc) to be shared between different OpenGL contexts.
	- VVGL::GLScene provides a simplified interface for performing drawing commands.  Makes it particularly easy to render-to-texture and set up orthogonal drawing.  Every GLScene must be backed by a GLContextRef- each scene can own its own context, or multiple scenes can use the same context.
	- VVGL::GLBuffer is a wrapper for an OpenGL buffer of some sort.  Most of the GLBuffers you'll encounter will be wrappers for GL textures that contain an image of some sort- but a GLBuffer can also be an FBO/VBO/EBO/etc.  When the GLBuffer is released, its underlying GL resources is either deleted or pooled- which is why most of the time you'll probably just work with #GLBufferRef and the GL resources will delete themselves.
	- VVGL::GLBufferPool creates GLBuffers- it also pools many types of GLBuffers, which offers substantial performance improvements over creating/deleting textures repeatedly.  You'll probably create a single global buffer pool when setting up your app's GL environment, but there's no hard limit on the number of pools you can create (different pools for different GL environments in the same app, for example).
	- VVGL::GLTexToTexCopier copies the contents of one GLBuffer into another GLBuffer.


*/








//! GLBuffer creation functions
/*!
\defgroup VVGL_BUFFERCREATE VVGL- GLBuffer create functions
These functions create GLBuffer instances.  Note that these functions all require a GLBufferPool- you need to create a buffer pool before you can create any buffers!
*/








//! These are some basic structs used to describe geometry in VVGL and VVISF.
/*!
\defgroup VVGL_GEOM VVGL- Geometry
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








//! Some of the less frequently encountered classes in VVGL- this is the stuff you'll probably be less likely to run into or have to work with if you're here because you just want to get to the ISF goodies.
/*!
\defgroup VVGL_MISC VVGL- other classes

	- VVGL::Range
	- VVGL::Timestamp
	- There are some basic geometric primitives defined in \ref VVGL_GEOM
	- VVGL::GLCachedProperty, VVGL::GLCachedAttrib and VVGL::GLCachedUni
*/








/*!
\defgroup VVGL_ADD_MORE_SDK VVGL- Adding new SDKs/platforms

This is a rough outline of the procedure necessary to add support for a new SDK or platform to VVGL/VVISF:

- Create a new "VVGL_SDK_****" define for the platform/SDK you want to add.  Open VVGL_Defines.hpp, and add your new SDK define to the if/elif/endif statement, being sure to define which GL environments you want your SDK to be compiled against.  This just defines which GL environments (like GLES/GL4/etc) VVGL/VVISF will be compiled against- you should make sure that all GL environments that will be available at runtime are specified here (VVGL and VVISF support multiple versions of GL at runtime, but when you compile these libs you need to ensure that they include code for the relevant versions of OpenGL you'll be working with).  You should probably add the define you created to "VVGL_HardCodedDefines.hpp", too.
- Add a new implementation for GLContext for the new platform/SDK by creating a new GLContext_XXXX.cpp file and populating it with an implementation of GLContext using the desired platform/SDK.  GLContext is a platform-agnostic representation of a GL context- typically, this means that the GLContext implementation you're writing needs a strong reference to your SDK-specific GL context object, and you should also fill in a couple other basic methods that make the context current if it's not current, make a context in the same sharegroup, etc.  Five implementations of GLContext are provided for various SDKs/platforms, and can be used as templates for adding more.
- The texture types available differ from platform to platform- for convenience, you'll want to create a header file that lists the enums for the OpenGL texture targets/internal formats/pixel formats/pixel types available in your SDK.  You should pattern this header file after one of the existing header files (GLBuffer_XXXX_Enums.h), and ensure that it's included in GLBuffer.hpp using the VVGL_SDK define you created.
- GLBufferPool.hpp declares a variety of functions that create GL textures, returned as #GLBufferRef instances.  These functions are defined in GLBufferPool.cpp, and you should check their implementations briefly to ensure that the resources they're creating are available on your platform (it's likely that you'll get a compiler error if they aren't, but it can't hurt to check).
- Send me a pull request!
*/








#endif /* VVGL_Doxygen_h */
