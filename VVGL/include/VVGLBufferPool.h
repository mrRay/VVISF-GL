#ifndef VVGLBufferPool_h
#define VVGLBufferPool_h

#include "VVGLBuffer.hpp"
#include "VVGLBufferPool.hpp"

#if ISF_TARGET_MAC || ISF_TARGET_IOS
#import <CoreGraphics/CoreGraphics.h>
#endif




using namespace std;




namespace VVGL
{



/*
VVGLBufferRef CreateTexFromImage(const string & inPath, const VVGLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
VVGLBufferRef CreateTexFromCGImageRef(const CGImageRef & n, const VVGLBufferPoolRef & inPoolRef=GetGlobalBufferPool());

VVGLBufferRef CreateCubeTexFromImagePaths(const vector<string> & inPaths, const VVGLBufferPoolRef & inPoolRef=GetGlobalBufferPool());
VVGLBufferRef CreateCubeTexFromImages(const vector<CGImageRef> & inPaths, const VVGLBufferPoolRef & inPoolRef=GetGlobalBufferPool());

VVGLBufferRef CreateRGBATexIOSurface(const Size & inSize, const VVGLBufferPoolRef & inPoolRef=GetGlobalBufferPool());


void CGBitmapContextUnpremultiply(CGContextRef ctx);
*/



}




#endif /* VVGLBufferPool_h */
