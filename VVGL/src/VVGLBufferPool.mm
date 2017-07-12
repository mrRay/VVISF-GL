#include <string>
#include "VVGLBufferPool.h"
//#include "VVGLBuffer.hpp"
#include "VVBase.hpp"

#import <Foundation/Foundation.h>
//#import <CoreGraphics/CoreGraphics.h>
#import <CoreVideo/CoreVideo.h>

#if ISF_TARGET_IOS
	#import <UIKit/UIKit.h>
#endif




using namespace std;
using namespace VVGL;




namespace VVGL
{




#if ISF_TARGET_MAC || ISF_TARGET_IOS




VVGLBufferRef CreateTexFromImage(const string & inPath, const bool & inCreateInCurrentContext, const VVGLBufferPoolRef & inPoolRef)	{
	
	NSString			*pathString = [NSString stringWithUTF8String:inPath.c_str()];
	
#if ISF_TARGET_MAC
	NSData				*imgData = [NSData dataWithContentsOfFile:pathString];
	//	make an img source from the CFData blob
	CGImageSourceRef	imgSource = CGImageSourceCreateWithData((CFDataRef)imgData, NULL);
	//	get an image from the image source
	CGImageRef			img = CGImageSourceCreateImageAtIndex(imgSource, 0, nil);
#elif ISF_TARGET_IOS
	UIImage				*tmpImg = [UIImage imageNamed:pathString];
	CGImageRef			img = [tmpImg CGImage];
#endif
	//	make an VVGLBufferRef from the image
	VVGLBufferRef		returnMe = CreateTexFromCGImageRef(img);
	
	//	release the various resources
	if (img != NULL)	{
		CFRelease(img);
		img = NULL;
	}
#if ISF_TARGET_MAC
	if (imgSource != NULL)	{
		CFRelease(imgSource);
		imgSource = NULL;
	}
#endif
	
	return returnMe;
	
	//return nullptr;
}
VVGLBufferRef CreateTexFromCGImageRef(const CGImageRef & n, const bool & inCreateInCurrentContext, const VVGLBufferPoolRef & inPoolRef)	{
	if (inPoolRef == nullptr)
		return nullptr;
	
	lock_guard<recursive_mutex>		lock(inPoolRef->getContextLock());
	if (!inCreateInCurrentContext)	{
		VVGLContextRef				context = inPoolRef->getContext();
		if (context == nullptr)
			return nullptr;
		//context->makeCurrentIfNull();
		//context->makeCurrent();
		context->makeCurrentIfNotCurrent();
	}
	
	bool				directUploadOK = true;
	CGBitmapInfo		newImgInfo = CGImageGetBitmapInfo(n);
	CGImageAlphaInfo	calculatedAlphaInfo = static_cast<CGImageAlphaInfo>(newImgInfo & kCGBitmapAlphaInfoMask);
	
	//((newImgInfo & kCGBitmapByteOrderDefault) == kCGBitmapByteOrderDefault)	||
	if (((newImgInfo & kCGBitmapFloatComponents) == kCGBitmapFloatComponents)	||
	((newImgInfo & kCGBitmapByteOrder16Little) == kCGBitmapByteOrder16Little)	||
	((newImgInfo & kCGBitmapByteOrder16Big) == kCGBitmapByteOrder16Big)	||
	//((newImgInfo & kCGBitmapByteOrder32Little) == kCGBitmapByteOrder32Little)	||
	((newImgInfo & kCGBitmapByteOrder32Big) == kCGBitmapByteOrder32Big))	{
		directUploadOK = false;
	}
	
	switch (calculatedAlphaInfo)	{
	case kCGImageAlphaPremultipliedLast:
	case kCGImageAlphaPremultipliedFirst:
	case kCGImageAlphaOnly:
	case kCGImageAlphaNone:
		directUploadOK = false;
		break;
	case kCGImageAlphaLast:
	case kCGImageAlphaFirst:
	case kCGImageAlphaNoneSkipLast:
	case kCGImageAlphaNoneSkipFirst:
		break;
	}
	
	VVGLBufferRef		returnMe = nullptr;
	
	Size				imgSize = Size(CGImageGetWidth(n), CGImageGetHeight(n));
	//	if i can upload the pixel data from the CGImageRef directly to a texture...
	if (directUploadOK)	{
		//	this just copies the data right out of the image provider, let's give it a shot...
		CFDataRef	frameData = CGDataProviderCopyData(CGImageGetDataProvider(n));
		if (frameData != NULL)	{
			
			VVGLBuffer::Descriptor	desc;
			desc.type = VVGLBuffer::Type_Tex;
			desc.target = VVGLBuffer::Target_2D;
#if ISF_TARGET_MAC
			desc.internalFormat = VVGLBuffer::IF_RGBA8;
#else
			desc.internalFormat = VVGLBuffer::IF_RGBA;
#endif
			desc.pixelFormat = VVGLBuffer::PF_RGBA;
#if ISF_TARGET_MAC
			desc.pixelType = VVGLBuffer::PT_UInt_8888_Rev;
#else
			desc.pixelType = VVGLBuffer::PT_UByte;
#endif
			desc.cpuBackingType = VVGLBuffer::Backing_External;
			desc.gpuBackingType = VVGLBuffer::Backing_Internal;
			desc.texRangeFlag = true;
			desc.texClientStorageFlag = true;
			desc.msAmount = 0;
			desc.localSurfaceID = 0;
			
			returnMe = inPoolRef->createBufferRef(desc, imgSize, (void*)CFDataGetBytePtr(frameData), imgSize, inCreateInCurrentContext);
			if (returnMe != nullptr)	{
				returnMe->parentBufferPool = inPoolRef;
			
				CFRetain(frameData);
				returnMe->backingContext = (void*)(frameData);
				returnMe->backingReleaseCallback = [](VVGLBuffer& inBuffer, void* inReleaseContext) {
					CFRelease((CFDataRef)(inReleaseContext));
				};
				returnMe->preferDeletion = true;
				returnMe->flipped = true;
			}
			
			CFRelease(frameData);
			frameData = NULL;
		}
	}
	//	else the direct upload isn't okay...
	else	{
		//	alloc some memory, make a CGBitmapContext that uses the memory to store pixel data
		GLubyte			*imgData = (GLubyte*)calloc((long)imgSize.width * (long)imgSize.height, sizeof(GLubyte)*4);
		CGContextRef	ctx = CGBitmapContextCreate(imgData,
			(long)imgSize.width,
			(long)imgSize.height,
			8,
			((long)(imgSize.width))*4,
			inPoolRef->getColorSpace(),
			kCGImageAlphaPremultipliedLast);
		if (ctx == NULL)	{
			cout << "ERR: ctx null in " << __PRETTY_FUNCTION__ << endl;
			free(imgData);
			return nullptr;
		}
		//	draw the image in the bitmap context, flush it
		CGContextDrawImage(ctx, CGRectMake(0,0,imgSize.width,imgSize.height), n);
		CGContextFlush(ctx);
		//	the bitmap context we just drew into has premultiplied alpha, so we need to un-premultiply before uploading it
		CGBitmapContextUnpremultiply(ctx);
		CGContextRelease(ctx);
		
		VVGLBuffer::Descriptor	desc;
		desc.type = VVGLBuffer::Type_Tex;
		desc.target = VVGLBuffer::Target_2D;
#if ISF_TARGET_MAC
		desc.internalFormat = VVGLBuffer::IF_RGBA8;
#else
		desc.internalFormat = VVGLBuffer::IF_RGBA;
#endif
		desc.pixelFormat = VVGLBuffer::PF_RGBA;
		desc.pixelType = VVGLBuffer::PT_UByte;
		desc.cpuBackingType = VVGLBuffer::Backing_External;
		desc.gpuBackingType = VVGLBuffer::Backing_Internal;
		desc.texRangeFlag = false;
		desc.texClientStorageFlag = true;
		desc.msAmount = 0;
		desc.localSurfaceID = 0;
		
		returnMe = inPoolRef->createBufferRef(desc, imgSize, (void*)imgData, imgSize, inCreateInCurrentContext);
		if (returnMe != nullptr)	{
			returnMe->parentBufferPool = inPoolRef;
			
			returnMe->backingContext = (void*)imgData;
			returnMe->backingReleaseCallback = [](VVGLBuffer& inBuffer, void *inReleaseContext)	{
				if (inReleaseContext != nullptr)	{
					free(inReleaseContext);
				}
			};
			
			returnMe->backingID = VVGLBuffer::BackingID_Pixels;
			returnMe->preferDeletion = true;
			returnMe->flipped = true;
		}
		else	{
			free(imgData);
			imgData = NULL;
		}
	}
	
	if (returnMe != nullptr)
		inPoolRef->timestampThisBuffer(returnMe);
	
	return returnMe;
	
	//return nullptr;
}




VVGLBufferRef CreateCubeTexFromImagePaths(const vector<string> & inPaths, const bool & inCreateInCurrentContext, const VVGLBufferPoolRef & inPoolRef)	{
	//cout << __FUNCTION__ << endl;
	if (inPaths.size() != 6)
		return nullptr;
	vector<CGImageRef>		images;
	for (const auto & pathIt : inPaths)	{
		//cout << "\tpath: " << pathIt << endl;
		NSString		*pathString = [NSString stringWithUTF8String:pathIt.c_str()];
#if ISF_TARGET_MAC
		NSData			*imgData = (pathString==nil) ? nil : [NSData dataWithContentsOfFile:pathString];
		CGImageSourceRef	imgSource = (imgData==nil) ? NULL : CGImageSourceCreateWithData((CFDataRef)imgData, NULL);
		CGImageRef		img = (imgSource==NULL) ? NULL : CGImageSourceCreateImageAtIndex(imgSource, 0, nil);
#elif ISF_TARGET_IOS
		UIImage			*tmpImg = [UIImage imageNamed:pathString];
		CGImageRef		img = [tmpImg CGImage];
#endif
		
		//	add the image to the array of image refs
		if (img != NULL)
			images.push_back(img);
#if ISF_TARGET_MAC
		//	free the source now- we'll free the image when we're done uploading it
		if (imgSource != NULL)
			CFRelease(imgSource);
#endif
	}
	VVGLBufferRef		returnMe = nullptr;
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
VVGLBufferRef CreateCubeTexFromImages(const vector<CGImageRef> & inImgs, const bool & inCreateInCurrentContext, const VVGLBufferPoolRef & inPoolRef)	{
	//cout << __FUNCTION__ << endl;
	if (inPoolRef == nullptr)
		return nullptr;
	//	make sure that i was passed six images, and that all six images have the same dimensions
	if (inImgs.size() != 6)	{
		cout << "ERR: bailing, not passed 6 images, " << __FUNCTION__ << endl;
		return nullptr;
	}
	//	make sure that the dimensions of all the passed images are identical
	VVGL::Size		baseSize(CGImageGetWidth(inImgs[0]), CGImageGetHeight(inImgs[0]));
	for (const auto & imgIt : inImgs)	{
		VVGL::Size		tmpSize(CGImageGetWidth(imgIt), CGImageGetHeight(imgIt));
		if (baseSize != tmpSize)
			return nullptr;
	}
	
#if ISF_TARGET_RPI
	return nullptr;
#else
	//	assemble a descriptor for the resource we want to create
	VVGLBuffer::Descriptor	desc;
	desc.type = VVGLBuffer::Type_Tex;
	desc.target = VVGLBuffer::Target_Cube;
#if ISF_TARGET_MAC
	desc.internalFormat = VVGLBuffer::IF_RGBA8;
	desc.pixelFormat = VVGLBuffer::PF_RGBA;
	desc.pixelType = VVGLBuffer::PT_UInt_8888_Rev;
#else
	desc.internalFormat = VVGLBuffer::IF_RGBA;
	desc.pixelFormat = VVGLBuffer::PF_RGBA;
	desc.pixelType = VVGLBuffer::PT_UByte;
#endif
	desc.cpuBackingType = VVGLBuffer::Backing_None;
	desc.gpuBackingType = VVGLBuffer::Backing_Internal;
	desc.texRangeFlag = false;
	desc.texClientStorageFlag = false;
	desc.msAmount = 0;
	desc.localSurfaceID = 0;
	//	create the VVGLBuffer we'll be returning, set it up with the basic info
	VVGLBufferRef		returnMe = make_shared<VVGLBuffer>();
	returnMe->parentBufferPool = inPoolRef;
	returnMe->desc = desc;
	returnMe->preferDeletion = true;
	returnMe->size = baseSize;
	returnMe->srcRect = Rect(0,0,baseSize.width,baseSize.height);
	inPoolRef->timestampThisBuffer(returnMe);
	
	
	//	lock, set the current context
	lock_guard<recursive_mutex>		lock(inPoolRef->getContextLock());
	if (!inCreateInCurrentContext)	{
		VVGLContextRef				context = inPoolRef->getContext();
		if (context == nullptr)
			return nullptr;
		//context->makeCurrentIfNull();
		//context->makeCurrent();
		context->makeCurrentIfNotCurrent();
	}
	
	//	create the GL resource, do some basic setup before we upload data to it
	glActiveTexture(GL_TEXTURE0);
	GLERRLOG
#if !ISF_TARGET_IOS && !ISF_TARGET_RPI
	if (inPoolRef->getContext()->version <= GLVersion_2)	{
		glEnable(desc.target);
		GLERRLOG
	}
#endif
	glGenTextures(1, &(returnMe->name));
	GLERRLOG
	glBindTexture(desc.target, returnMe->name);
	GLERRLOG
	
	glPixelStorei(GL_UNPACK_SKIP_ROWS, GL_FALSE);
	GLERRLOG
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, GL_FALSE);
	GLERRLOG
#if !ISF_TARGET_IOS
	glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
	GLERRLOG
#endif
	
	glTexParameteri(desc.target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	GLERRLOG
	glTexParameteri(desc.target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	GLERRLOG
	glTexParameteri(desc.target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	GLERRLOG
	glTexParameteri(desc.target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	GLERRLOG
	glFlush();
	GLERRLOG
	
	
	//	allocate some "clipboard data"- we're going to draw each of the images into this data (it's bitmap data) and then upload this
	int				faceCount = 0;
	uint32_t		bytesPerRow = 32 * baseSize.width / 8;
	void			*clipboardData = malloc(bytesPerRow * baseSize.height);
	CGContextRef	ctx = CGBitmapContextCreate(clipboardData,
		(long)baseSize.width,
		(long)baseSize.height,
		8,
		((long)(baseSize.width))*4,
		inPoolRef->getColorSpace(),
		kCGImageAlphaPremultipliedLast);
	if (ctx != NULL)	{
		//	run through the images, drawing them into the clipboard and then uploading the clipboard
		for (const auto & imgIt : inImgs)	{
			//	draw the image in the bitmap context, flush it
			CGContextDrawImage(ctx, CGRectMake(0,0,baseSize.width,baseSize.height), imgIt);
			CGContextFlush(ctx);
			//	the context we just drew into has premultiplied alpha, so we need to un-premultiply it
			CGBitmapContextUnpremultiply(ctx);
			//	upload the bitmap data to the texture
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + faceCount,
				0,
				desc.internalFormat,
				baseSize.width,
				baseSize.height,
				0,
				desc.pixelFormat,
				desc.pixelType,
				clipboardData);
			GLERRLOG
			glFlush();
			GLERRLOG
			
			++faceCount;
		}
		
		CGContextRelease(ctx);
	}
	else	{
		cout << "\tERR: bailing, couldn't create CGContext, " << __FUNCTION__ << endl;
		returnMe = nullptr;
	}
	
	glBindTexture(desc.target, 0);
	GLERRLOG
#if !ISF_TARGET_IOS && !ISF_TARGET_RPI
	if (inPoolRef->getContext()->version <= GLVersion_2)	{
		glDisable(desc.target);
		GLERRLOG
	}
#endif
	
	free(clipboardData);
	
	return returnMe;
#endif	/*	ISF_TARGET_RPI	*/
}




#if ISF_TARGET_MAC
VVGLBufferRef CreateRGBATexIOSurface(const Size & inSize, const bool & inCreateInCurrentContext, const VVGLBufferPoolRef & inPoolRef)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	if (inPoolRef == nullptr)
		return nullptr;
	
	VVGLBuffer::Descriptor	desc;
	
	desc.type = VVGLBuffer::Type_Tex;
	desc.target = VVGLBuffer::Target_Rect;
	desc.internalFormat = VVGLBuffer::IF_RGBA8;
	desc.pixelFormat = VVGLBuffer::PF_BGRA;
	desc.pixelType = VVGLBuffer::PT_UInt_8888_Rev;
	desc.cpuBackingType = VVGLBuffer::Backing_None;
	desc.gpuBackingType = VVGLBuffer::Backing_Internal;
	desc.texRangeFlag = false;
	desc.texClientStorageFlag = false;
	desc.msAmount = 0;
	desc.localSurfaceID = 1;
	
	VVGLBufferRef	returnMe = inPoolRef->createBufferRef(desc, inSize, nullptr, Size(), inCreateInCurrentContext);
	returnMe->parentBufferPool = inPoolRef;
	
	return returnMe;
}
VVGLBufferRef CreateRGBAFloatTexIOSurface(const Size & inSize, const bool & inCreateInCurrentContext, const VVGLBufferPoolRef & inPoolRef)	{
	if (inPoolRef == nullptr)
		return nullptr;
	
	VVGLBuffer::Descriptor	desc;
	
	desc.type = VVGLBuffer::Type_Tex;
	desc.target = VVGLBuffer::Target_Rect;
	desc.internalFormat = VVGLBuffer::IF_RGBA32F;
	desc.pixelFormat = VVGLBuffer::PF_RGBA;
	desc.pixelType = VVGLBuffer::PT_Float;
	desc.cpuBackingType = VVGLBuffer::Backing_None;
	desc.gpuBackingType = VVGLBuffer::Backing_Internal;
	desc.texRangeFlag = false;
	desc.texClientStorageFlag = false;
	desc.msAmount = 0;
	desc.localSurfaceID = 1;
	
	VVGLBufferRef	returnMe = inPoolRef->createBufferRef(desc, inSize, nullptr, Size(), inCreateInCurrentContext);
	returnMe->parentBufferPool = inPoolRef;
	
	return returnMe;
}
VVGLBufferRef CreateRGBATexFromIOSurfaceID(const IOSurfaceID & inID, const bool & inCreateInCurrentContext, const VVGLBufferPoolRef & inPoolRef)	{
	if (inPoolRef == nullptr)
		return nullptr;
	
	//	look up the surface for the ID i was passed, bail if i can't
	IOSurfaceRef		newSurface = IOSurfaceLookup(inID);
	if (newSurface == NULL)
		return nullptr;
	
	//	figure out how big the IOSurface is and what its pixel format is
	Size			newAssetSize(IOSurfaceGetWidth(newSurface), IOSurfaceGetHeight(newSurface));
	uint32_t		pixelFormat = IOSurfaceGetPixelFormat(newSurface);
	size_t			bytesPerRow = IOSurfaceGetBytesPerRow(newSurface);
	bool			isRGBAFloatTex = (bytesPerRow >= (32*4*newAssetSize.width/8)) ? true : false;
	//	make the buffer i'll be returning, set up as much of it as i can
	VVGLBufferRef		returnMe = make_shared<VVGLBuffer>(inPoolRef);
	inPoolRef->timestampThisBuffer(returnMe);
	returnMe->desc.type = VVGLBuffer::Type_Tex;
	returnMe->desc.target = VVGLBuffer::Target_Rect;
	
	switch (pixelFormat)	{
	case kCVPixelFormatType_32BGRA:	//	'BGRA'
	case VVGLBuffer::PF_BGRA:
		returnMe->desc.pixelFormat = VVGLBuffer::PF_BGRA;
		if (isRGBAFloatTex)	{
			returnMe->desc.internalFormat = VVGLBuffer::IF_RGBA32F;
			returnMe->desc.pixelType = VVGLBuffer::PT_Float;
		}
		else	{
			returnMe->desc.internalFormat = VVGLBuffer::IF_RGBA8;
			returnMe->desc.pixelType = VVGLBuffer::PT_UInt_8888_Rev;
		}
		break;
	case kCVPixelFormatType_32RGBA:	//	'RGBA'
	case VVGLBuffer::PF_RGBA:
		returnMe->desc.pixelFormat = VVGLBuffer::PF_RGBA;
		if (isRGBAFloatTex)	{
			returnMe->desc.internalFormat = VVGLBuffer::IF_RGBA32F;
			returnMe->desc.pixelType = VVGLBuffer::PT_Float;
		}
		else	{
			returnMe->desc.internalFormat = VVGLBuffer::IF_RGBA8;
			returnMe->desc.pixelType = VVGLBuffer::PT_UInt_8888_Rev;
		}
		break;
	case kCVPixelFormatType_422YpCbCr8:	//	'2vuy'
	case VVGLBuffer::PF_YCbCr_422:
		returnMe->desc.internalFormat = VVGLBuffer::IF_RGB;
		returnMe->desc.pixelFormat = VVGLBuffer::PF_YCbCr_422;
		returnMe->desc.pixelType = VVGLBuffer::PT_UShort88;
		break;
	default:
		cout << "\tERR: unknown pixel format (" << pixelFormat << ") in " << __PRETTY_FUNCTION__ << endl;
		returnMe->desc.internalFormat = VVGLBuffer::IF_RGBA8;
		returnMe->desc.pixelFormat = VVGLBuffer::PF_BGRA;
		returnMe->desc.pixelType = VVGLBuffer::PT_UInt_8888_Rev;
		break;
	}
	
	returnMe->desc.cpuBackingType = VVGLBuffer::Backing_None;
	returnMe->desc.gpuBackingType = VVGLBuffer::Backing_Internal;
	returnMe->desc.texRangeFlag = false;
	returnMe->desc.texClientStorageFlag = false;
	returnMe->desc.msAmount = 0;
	returnMe->desc.localSurfaceID = inID;
	
	returnMe->name = 0;
	returnMe->preferDeletion = true;
	returnMe->size = newAssetSize;
	returnMe->srcRect = Rect(0,0,newAssetSize.width,newAssetSize.height);
	//returnMe->flipped = 
	returnMe->backingSize = newAssetSize;
	//returnMe->backingReleaseCallback = 
	//returnMe->backingContext = 
	returnMe->backingID = VVGLBuffer::BackingID_RemoteIOSrf;
	
	returnMe->setRemoteSurfaceRef(newSurface);
	
	//	we can free the surface now that the buffer we'll be returning has retained it
	CFRelease(newSurface);
	
	//	...now that i've created and set up the VVGLBuffer, take care of the GL resource setup...
	
	//	grab a context lock so we can do stuff with the GL context
	lock_guard<recursive_mutex>		lock(inPoolRef->getContextLock());
	if (!inCreateInCurrentContext)	{
		VVGLContextRef					context = (inPoolRef==nullptr) ? nullptr : inPoolRef->getContext();
		context->makeCurrentIfNotCurrent();
	}
	CGLContextObj		cglCtx = CGLGetCurrentContext();
	if (cglCtx != NULL)	{
		glActiveTexture(GL_TEXTURE0);
		GLERRLOG
		if (inPoolRef->getContext()->version <= GLVersion_2)	{
			glEnable(returnMe->desc.target);
			GLERRLOG
		}
		glGenTextures(1,&(returnMe->name));
		GLERRLOG
		glBindTexture(returnMe->desc.target, returnMe->name);
		GLERRLOG
		CGLError		err = CGLTexImageIOSurface2D(cglCtx,
			returnMe->desc.target,
			returnMe->desc.internalFormat,
			newAssetSize.width,
			newAssetSize.height,
			returnMe->desc.pixelFormat,
			returnMe->desc.pixelType,
			newSurface,
			0);
		if (err != noErr)
			cout << "\tERR: " << err << " at CGLTexImageIOSurface2D() in " << __PRETTY_FUNCTION__ << endl;
		glTexParameteri(returnMe->desc.target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		GLERRLOG
		glTexParameteri(returnMe->desc.target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		GLERRLOG
		glTexParameteri(returnMe->desc.target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		GLERRLOG
		glTexParameteri(returnMe->desc.target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		GLERRLOG
		glFlush();
		GLERRLOG
		glBindTexture(returnMe->desc.target, 0);
		GLERRLOG
		if (inPoolRef->getContext()->version <= GLVersion_2)	{
			glDisable(returnMe->desc.target);
			GLERRLOG
		}
	}
	
	return returnMe;
}
#endif




void CGBitmapContextUnpremultiply(CGContextRef ctx)	{
	Size				actualSize = Size(CGBitmapContextGetWidth(ctx), CGBitmapContextGetHeight(ctx));
	unsigned long		bytesPerRow = CGBitmapContextGetBytesPerRow(ctx);
	unsigned char		*bitmapData = (unsigned char *)CGBitmapContextGetData(ctx);
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



#endif	//	#if ISF_TARGET_MAC || ISF_TARGET_IOS




}

