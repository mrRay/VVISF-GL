#include <string>
#include "GLBufferPool_CocoaAdditions.h"
//#include "GLBuffer.hpp"
#include "VVGL_Base.hpp"

#import <Foundation/Foundation.h>
//#import <CoreGraphics/CoreGraphics.h>
#import <CoreVideo/CoreVideo.h>

#if defined(VVGL_SDK_IOS)
	#import <UIKit/UIKit.h>
#endif




using namespace std;
using namespace VVGL;




namespace VVGL
{




#if defined(VVGL_SDK_MAC)
GLBufferRef CreateBufferForNSImage(NSImage * inImg, const bool & createInCurrentContext, const GLBufferPoolRef & inPoolRef)	{
	if (inImg==nil || inPoolRef==nullptr)
		return nullptr;
	NSSize				origImageSize = [inImg size];
	NSRect				origImageRect = NSMakeRect(0, 0, origImageSize.width, origImageSize.height);
	NSImageRep			*bestRep = [inImg bestRepresentationForRect:origImageRect context:nil hints:nil];
	NSSize				bitmapSize = NSMakeSize([bestRep pixelsWide], [bestRep pixelsHigh]);
	if (bitmapSize.width==0 || bitmapSize.height==0)
		bitmapSize = [inImg size];
	NSRect				bitmapRect = NSMakeRect(0,0,bitmapSize.width,bitmapSize.height);
	
	//	make a bitmap image rep
	NSBitmapImageRep		*rep = [[NSBitmapImageRep alloc]
		initWithBitmapDataPlanes:nil
		pixelsWide:bitmapSize.width
		pixelsHigh:bitmapSize.height
		bitsPerSample:8
		samplesPerPixel:4
		hasAlpha:YES
		isPlanar:NO
		colorSpaceName:NSCalibratedRGBColorSpace
		bitmapFormat:0
		bytesPerRow:32 * bitmapSize.width / 8
		bitsPerPixel:32];
	if (rep == nil)
		return nullptr;
	//	save the current NSGraphicsContext, make a new one based on the bitmap image rep i just created
	NSGraphicsContext		*origContext = [NSGraphicsContext currentContext];
	if (origContext != nil)
		[origContext retain];
	NSGraphicsContext		*newContext = [NSGraphicsContext graphicsContextWithBitmapImageRep:rep];
	if (newContext != nil)	{
		//	set up & start drawing in the new graphics context (draws into the bitmap image rep)
		[NSGraphicsContext setCurrentContext:newContext];
		[newContext setShouldAntialias:NO];
		
		//[[NSColor colorWithDeviceRed:0.0 green:0.0 blue:0.0 alpha:1.0] set];
		//VVRECTFill(bitmapRect);
		[inImg
			drawInRect:bitmapRect
			fromRect:origImageRect
			operation:NSCompositingOperationCopy
			fraction:1.0];
		
		//	flush the graphics
		[newContext flushGraphics];
	}
	[NSGraphicsContext setCurrentContext:origContext];
	if (origContext != nil)	{
		[origContext release];
		origContext = nil;
	}
	
	//	the bitmap rep we just drew into was premultiplied, and we have to fix that before uploading it
	[rep unpremultiply];
	
	//VVBuffer		*returnMe = [self allocBufferForBitmapRep:rep prefer2DTexture:prefer2D];
	GLBufferRef	returnMe = CreateBufferForBitmapRep(rep, createInCurrentContext, inPoolRef);
	[rep release];
	//[returnMe setSrcRect:VVMAKERECT(0,0,bitmapSize.width,bitmapSize.height)];
	returnMe->srcRect = VVGL::Rect(0,0,bitmapSize.width,bitmapSize.height);
	//[returnMe setFlipped:YES];
	returnMe->flipped = true;
	/*	the static analyzer flags this as a leak, but it isn't.  the GLBuffer instance retains the NSBitmapRep underlying the GL texture, which is interpreted here as a leak.		*/
	return returnMe;
	/*	the static analyzer flags this as a leak, but it isn't.  the GLBuffer instance retains the NSBitmapRep underlying the GL texture, which is interpreted here as a leak.		*/
}
GLBufferRef CreateBufferForBitmapRep(NSBitmapImageRep * inRep, const bool & createInCurrentContext, const GLBufferPoolRef & inPoolRef)	{
	if (inRep==nil || inPoolRef==nullptr)
		return nullptr;
	void					*pixelData = (void *)[inRep bitmapData];
	if (pixelData == nil)
		return nullptr;
	NSSize					cgRepSize = [inRep size];
	VVGL::Size				repSize(cgRepSize.width, cgRepSize.height);
	VVGL::Size				gpuSize;
	/*
	if (prefer2D)	{
		int						tmpInt;
		tmpInt = 1;
		while (tmpInt < repSize.width)
			tmpInt <<= 1;
		gpuSize.width = tmpInt;
		tmpInt = 1;
		while (tmpInt < repSize.height)
			tmpInt <<= 1;
		gpuSize.height = tmpInt;
	}
	else	{
	*/
		gpuSize = repSize;
	/*
	}
	*/
	
	GLBuffer::Descriptor		desc;
	desc.type = GLBuffer::Type_Tex;
	desc.target = GLBuffer::Target_Rect;
	desc.internalFormat = GLBuffer::IF_RGBA8;
	desc.pixelFormat = GLBuffer::PF_RGBA;
	desc.pixelType = GLBuffer::PT_UInt_8888_Rev;
	desc.cpuBackingType = GLBuffer::Backing_External;
	desc.gpuBackingType = GLBuffer::Backing_Internal;
	desc.texRangeFlag = false;
	desc.texClientStorageFlag = true;
	desc.msAmount = 0;
	desc.localSurfaceID = 0;
	
	//VVBuffer			*returnMe = [self allocBufferForDescriptor:&desc sized:gpuSize backingPtr:pixelData backingSize:repSize];
	GLBufferRef		returnMe = inPoolRef->createBufferRef(desc, gpuSize, pixelData, repSize, createInCurrentContext);
	//[returnMe setSrcRect:VVMAKERECT(0,0,repSize.width,repSize.height)];
	returnMe->srcRect = VVGL::Rect(0,0,repSize.width,repSize.height);
	//	the backing release callback should release a bitmap rep- set it, and the context (which is the rep)
	//[returnMe setBackingID:VVBufferBackID_NSBitImgRep];
	returnMe->backingID = GLBuffer::BackingID_NSBitImgRep;
	//[returnMe setBackingReleaseCallback:VVBuffer_ReleaseBitmapRep];
	[inRep retain];	//	we want to explicitly retain the rep we were passed- when the underlying GLBuffer is freed, its backing release callback will release the rep
	returnMe->backingReleaseCallback = [](GLBuffer & inBuffer, void * inReleaseContext)	{
		if (inReleaseContext != nil)	{
			NSBitmapImageRep	*tmpRep = (NSBitmapImageRep *)inReleaseContext;
			[tmpRep release];
		}
	};
	//[returnMe setBackingReleaseCallbackContextObject:inRep];
	returnMe->backingContext = (void*)inRep;
	//[returnMe setBackingSize:repSize];
	returnMe->backingSize = repSize;
	
	returnMe->preferDeletion = true;
	
	return returnMe;
}
#endif	//	VVGL_SDK_MAC




#if defined(VVGL_SDK_MAC) || defined(VVGL_SDK_IOS)
GLBufferRef CreateTexFromImage(const string & inPath, const bool & inCreateInCurrentContext, const GLBufferPoolRef & inPoolRef)	{
	NSString			*pathString = [NSString stringWithUTF8String:inPath.c_str()];
#if defined(VVGL_SDK_MAC)
	NSData				*imgData = [NSData dataWithContentsOfFile:pathString];
	//	make an img source from the CFData blob
	CGImageSourceRef	imgSource = CGImageSourceCreateWithData((CFDataRef)imgData, NULL);
	//	get an image from the image source
	CGImageRef			img = CGImageSourceCreateImageAtIndex(imgSource, 0, nil);
#elif defined(VVGL_SDK_IOS)
	UIImage				*tmpImg = [UIImage imageNamed:pathString];
	CGImageRef			img = [tmpImg CGImage];
#endif
	//	make an GLBufferRef from the image
	GLBufferRef		returnMe = CreateTexFromCGImageRef(img, inCreateInCurrentContext, inPoolRef);
	
	//	release the various resources
	if (img != NULL)	{
		CFRelease(img);
		img = NULL;
	}
#if defined(VVGL_SDK_MAC)
	if (imgSource != NULL)	{
		CFRelease(imgSource);
		imgSource = NULL;
	}
#endif
	
	return returnMe;
	
	//return nullptr;
}
GLBufferRef CreateCubeTexFromImagePaths(const vector<string> & inPaths, const bool & inCreateInCurrentContext, const GLBufferPoolRef & inPoolRef)	{
	//cout << __FUNCTION__ << endl;
	if (inPaths.size() != 6)
		return nullptr;
	vector<CGImageRef>		images;
	for (const auto & pathIt : inPaths)	{
		//cout << "\tpath: " << pathIt << endl;
		NSString		*pathString = [NSString stringWithUTF8String:pathIt.c_str()];
#if defined(VVGL_SDK_MAC)
		NSData			*imgData = (pathString==nil) ? nil : [NSData dataWithContentsOfFile:pathString];
		CGImageSourceRef	imgSource = (imgData==nil) ? NULL : CGImageSourceCreateWithData((CFDataRef)imgData, NULL);
		CGImageRef		img = (imgSource==NULL) ? NULL : CGImageSourceCreateImageAtIndex(imgSource, 0, nil);
#elif defined(VVGL_SDK_IOS)
		UIImage			*tmpImg = [UIImage imageNamed:pathString];
		CGImageRef		img = [tmpImg CGImage];
#endif
		
		//	add the image to the array of image refs
		if (img != NULL)
			images.push_back(img);
#if defined(VVGL_SDK_MAC)
		//	free the source now- we'll free the image when we're done uploading it
		if (imgSource != NULL)
			CFRelease(imgSource);
#endif
	}
	GLBufferRef		returnMe = nullptr;
	//	if there are six images, we're clear to upload them to the GL texture
	if (images.size() == 6)	{
		returnMe = CreateCubeTexFromImages(images, inCreateInCurrentContext, inPoolRef);
	}
	//	run through the images, releasing them
	for (const auto & imgIt : images)	{
		if (imgIt != NULL)
			CFRelease(imgIt);
	}
	return returnMe;
}
#endif




}




#if defined(VVGL_SDK_MAC)
@implementation NSBitmapImageRep (VVGLNSBitmapImageRepAdditions)
- (void) unpremultiply	{
	NSSize				actualSize = NSMakeSize([self pixelsWide], [self pixelsHigh]);
	unsigned long		bytesPerRow = [self bytesPerRow];
	unsigned char		*bitmapData = [self bitmapData];
	unsigned char		*pixelPtr = nil;
	double				colors[4];
	if (bitmapData==nil || bytesPerRow<=0 || actualSize.width<1 || actualSize.height<1)
		return;
	for (int y=0; y<actualSize.height; ++y)	{
		pixelPtr = bitmapData + (y * bytesPerRow);
		for (int x=0; x<actualSize.width; ++x)	{
			//	convert unsigned chars to normalized doubles
			for (int i=0; i<4; ++i)
				colors[i] = ((double)*(pixelPtr+i))/255.;
			//	unpremultiply if there's an alpha and it won't cause a divide-by-zero
			if (colors[3]>0. && colors[3]<1.)	{
				for (int i=0; i<3; ++i)
					colors[i] = colors[i] / colors[3];
			}
			//	convert the normalized components back into unsigned chars
			for (int i=0; i<4; ++i)
				*(pixelPtr+i) = (unsigned char)(colors[i]*255.);
			
			//	don't forget to increment the pixel ptr!
			pixelPtr += 4;
		}
	}
}
@end
#endif	//	VVGL_SDK_MAC


