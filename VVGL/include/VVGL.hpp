#ifndef VVGL_h
#define VVGL_h




//	base types- basic time structs, range structs, string utils, geometry types, things like that
#include "VVBase.hpp"


#include "VVGLContext.hpp"
#include "VVGLCachedAttrib.hpp"
#include "VVGLCachedUni.hpp"


//	wraps a GL object of some sort- usually a texture, but can also be of other types (VBOs, etc).  if a texture, may also contain other texture-related resources (CPU-side backing data, for example).
#include "VVGLBuffer.hpp"
//	a pool for recycling GL objects.  should be in the same sharegroup as all your other GL contexts.  has its own GL context, so it can create/delete GL resources (and can thus act as a sort of autorelease pool for VVGLBufferRefs)
#include "VVGLBufferPool.hpp"
//	basic render-to-texture class, API geared towards rendering to VVGLBuffers.  encapsulates a GL context
#include "VVGLScene.hpp"
#include "VVGLBufferCopier.hpp"




#endif /* VVGL_h */
