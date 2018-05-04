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

GLBufferRef CreateBufferForNSImage(NSImage * inImg, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
GLBufferRef CreateBufferForBitmapRep(NSBitmapImageRep * inRep, const bool & createInCurrentContext=false, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());

#endif


}




#if defined(VVGL_SDK_MAC)
@interface NSBitmapImageRep (VVGLNSBitmapImageRepAdditions)
- (void) unpremultiply;
@end
#endif	//	VVGL_SDK_MAC




#endif /* VVGL_GLBufferPool_CocoaAdditions_h */
