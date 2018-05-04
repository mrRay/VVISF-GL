#ifndef VVGL_h
#define VVGL_h


//	base SDK defines.  the SDK will define the range of GL calls that are available (not all flavors of GL are available in all SDKs- for example, you're not going to get GL3+ on a raspberry pi, nor are you likely to see GL ES on a desktop).
#include "VVGL_Defines.hpp"


//	base types- basic time structs, range structs, string utils, geometry types, things like that
#include "VVGL_Base.hpp"


//	GLContext is a GL context- this class wraps up whatever the native object is for whatever platform you're compiling against, and presents a single standard interface across all platforms for the rest of VVGL/VVISF.  you can create a GLContext around an existing platform-specific OpenGL context, and you can also create new GL contexts.  explicit support for sharing of multiple contexts so resources can be shared.  you'll probably work almost exclusively with GLContextRef, which is a std::shared_ptr around a GLContext.
#include "GLContext.hpp"
#include "GLCachedProperty.hpp"


//	wraps a GL object of some sort- usually a texture, but can also be of other types (VBOs, etc).  if a texture, may also contain other texture-related resources (CPU-side backing data, for example).  you'll probably be working almost exclusively with GLBufferRef, which is a std::shared_ptr around a GLBuffer.
#include "GLBuffer.hpp"


//	a pool for recycling GL objects.  should be in the same sharegroup as all your other GL contexts.  has its own GL context, so it can create/delete GL resources (and can thus act as a sort of autorelease pool for GLBufferRefs)
#include "GLBufferPool.hpp"


//	a GLScene is a container for a GL context that lets you provide various callbacks which are executed when the scene is rendered.  interface is geared towards making it easy to load frag/vert/geo shaders, perform orthographic projection, providing customized drawing code, being subclassed, and rendering to textures/buffers/GLBuffers.  used as a subclass of several other classes, also used in sample apps to perform GL rendering and output.
#include "GLScene.hpp"


//	a GLBufferCopier will copy texture-based GLBuffer instances into other texture-based GLBuffer instances.  it's a GL texture copier, basically.  it has some frills for easily resizing textures or creating textures filled with a solid color.
#include "GLBufferCopier.hpp"


//	if we're compiling against the os x sdk then we've got some cocoa additions.  os x, not ios.
#if defined(VVGL_SDK_MAC) && defined(__OBJC__)
#include "GLBufferPool_CocoaAdditions.h"
#endif




#endif /* VVGL_h */
