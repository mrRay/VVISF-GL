#ifndef VVGL_h
#define VVGL_h




//	base types- basic time structs, geometry types, things like that
#include "VVBase.hpp"

//	VVGLBuffer encapsulates a GL resource (usually a texture).  try not to use it directly- use VVGLBufferRef instead.
#include "VVGLBuffer.hpp"
//	VVGLBufferPool pools buffers (pools GL textures)
#include "VVGLBufferPool.hpp"
#if ISF_TARGET_MAC || ISF_TARGET_IOS
#include "VVGLBufferPool.h"
#endif

//	basic render-to-texture class, API geared towards rendering to VVGLBuffers.  encapsulates a GL context
#include "VVGLScene.hpp"
//	copies GL textures (duplicates the texture)
#include "VVGLBufferCopier.hpp"
//	basic GLSL execution class- provide it with a frag & vert shader string and it will render to VVGLBuffers
#include "VVGLShaderScene.hpp"




#endif /* VVGL_h */
