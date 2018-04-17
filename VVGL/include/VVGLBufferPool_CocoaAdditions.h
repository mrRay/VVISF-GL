#ifndef VVGLBufferPool_h
#define VVGLBufferPool_h

#include "VVGL_Defines.hpp"

#include "VVGLBuffer.hpp"
#include "VVGLBufferPool.hpp"

#if ISF_SDK_MAC || ISF_SDK_IOS
#import <CoreGraphics/CoreGraphics.h>
#endif

#if ISF_SDK_MAC
#import <AppKit/AppKit.h>
#endif




using namespace std;




namespace VVGL
{


#if ISF_SDK_MAC

VVGLBufferRef CreateBufferForNSImage(NSImage * inImg, const bool & createInCurrentContext=false, const VVGLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
VVGLBufferRef CreateBufferForBitmapRep(NSBitmapImageRep * inRep, const bool & createInCurrentContext=false, const VVGLBufferPoolRef & inPoolRef=GetGlobalBufferPool());

#endif


}




#if ISF_SDK_MAC
@interface NSBitmapImageRep (VVGLNSBitmapImageRepAdditions)
- (void) unpremultiply;
@end
#endif	//	ISF_SDK_MAC




#endif /* VVGLBufferPool_h */
