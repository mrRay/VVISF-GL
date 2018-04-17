#ifndef VVGL_h
#define VVGL_h


//	base SDK defines.  the SDK will define the range of GL calls that are available (not all flavors of GL are available in all SDKs- for example, you're not going to get GL3+ on a raspberry pi, nor are you likely to see GL ES on a desktop).
#include "VVGL_Defines.hpp"


//	base types- basic time structs, range structs, string utils, geometry types, things like that
#include "VVBase.hpp"


#include "VVGLContext.hpp"
#include "VVGLCachedAttrib.hpp"
#include "VVGLCachedUni.hpp"


//	wraps a GL object of some sort- usually a texture, but can also be of other types (VBOs, etc).  if a texture, may also contain other texture-related resources (CPU-side backing data, for example).
#include "VVGLBuffer.hpp"
//	a pool for recycling GL objects.  should be in the same sharegroup as all your other GL contexts.  has its own GL context, so it can create/delete GL resources (and can thus act as a sort of autorelease pool for VVGLBufferRefs)
#include "VVGLBufferPool.hpp"
//#if ISF_SDK_MAC
//#import "VVGLBufferPool_CocoaAdditions.h"
//#endif


//	basic render-to-texture class, API geared towards rendering to VVGLBuffers.  encapsulates a GL context
#include "VVGLScene.hpp"
#include "VVGLBufferCopier.hpp"




#endif /* VVGL_h */
