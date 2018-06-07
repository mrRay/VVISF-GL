#ifndef VVGL_h
#define VVGL_h


//	we store a number of explicitly-defined doxygen modules here, because doxygen needs to execute them early on to work around some weird behavior with this combination of settings.
#include "VVGL_Doxygen.hpp"


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


//	these classes use PBOs to implement efficient texture upload/downloads
#include "GLCPUToTexCopier.hpp"
#include "GLTexToCPUCopier.hpp"


//	if we're compiling against the os x sdk then we've got some cocoa additions.  os x, not ios.
#if defined(VVGL_SDK_MAC) && defined(__OBJC__)
#include "GLBufferPool_CocoaAdditions.h"
#endif




#endif /* VVGL_h */
