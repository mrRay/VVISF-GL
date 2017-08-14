#ifndef VVGLBufferPool_h
#define VVGLBufferPool_h

#include "VVGLBuffer.hpp"
#include "VVGLBufferPool.hpp"

#if ISF_TARGET_MAC || ISF_TARGET_IOS
#import <CoreGraphics/CoreGraphics.h>
#endif

#if ISF_TARGET_MAC
#import <AppKit/AppKit.h>
#endif




using namespace std;




namespace VVGL
{


#if ISF_TARGET_MAC
VVGLBufferRef CreateBufferForNSImage(NSImage * inImg, const bool & createInCurrentContext=false, const VVGLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
VVGLBufferRef CreateBufferForBitmapRep(NSBitmapImageRep * inRep, const bool & createInCurrentContext=false, const VVGLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
#endif





}




#if ISF_TARGET_MAC
@interface NSBitmapImageRep (VVGLNSBitmapImageRepAdditions)
- (void) unpremultiply;
@end
#endif




#endif /* VVGLBufferPool_h */
