#ifndef VVGL_GLBufferPool_CocoaAdditions_h
#define VVGL_GLBufferPool_CocoaAdditions_h

#include "VVGL_Defines.hpp"

#include "GLBuffer.hpp"
#include "GLBufferPool.hpp"

#if defined(VVGL_SDK_MAC) || defined(VVGL_SDK_IOS)
#import <CoreGraphics/CoreGraphics.h>
#endif

#if defined(VVGL_SDK_MAC)
#import <AppKit/AppKit.h>
#endif




using namespace std;




namespace VVGL
{


#if defined(VVGL_SDK_MAC)

/*!
\ingroup VVGL_BUFFERCREATE
\brief Creates and returns an OpenGL texture with the contents of the passed NSImage.  The GL texture will be 8 bits per pixel/RGBA, alpha is supported.
\param inImg The image you want to upload to the GL texture.
\param createInCurrentContext If true, the GL resource will be created in the current context (assumes that a GL context is active in the current thread).  If false, the GL resource will be created by the GL context owned by the buffer pool.
\param inPoolRef The pool that the GLBuffer should be created with.  When the GLBuffer is freed, its underlying GL resources will be returned to this pool (where they will be either freed or recycled).
*/
GLBufferRef CreateBufferForNSImage(NSImage * inImg, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
/*!
\ingroup VVGL_BUFFERCREATE
\brief Creates and returns an OpenGL texture with the contents of the passed NSBitmapImageRep.  The GL texture will be 8 bits per pixel/RGBA, alpha is supported.
\param inRep The bitmap image rep you want to upload to the GL texture.
\param createInCurrentContext If true, the GL resource will be created in the current context (assumes that a GL context is active in the current thread).  If false, the GL resource will be created by the GL context owned by the buffer pool.
\param inPoolRef The pool that the GLBuffer should be created with.  When the GLBuffer is freed, its underlying GL resources will be returned to this pool (where they will be either freed or recycled).
*/
GLBufferRef CreateBufferForBitmapRep(NSBitmapImageRep * inRep, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());

#endif


}




#if defined(VVGL_SDK_MAC)
@interface NSBitmapImageRep (VVGLNSBitmapImageRepAdditions)
/*!
\ingroup VVGL_MISC
\brief Unpremultiplies the receiver- NSImages/NSBitmapImageReps tend to be premultiplied.
*/
- (void) unpremultiply;
@end
#endif	//	VVGL_SDK_MAC




#endif /* VVGL_GLBufferPool_CocoaAdditions_h */
