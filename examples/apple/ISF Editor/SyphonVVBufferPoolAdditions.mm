//
//  SyphonVVBufferPoolAdditions.m
//  ISF Syphon Filter Tester
//
//  Created by bagheera on 11/25/13.
//  Copyright (c) 2013 zoidberg. All rights reserved.
//

#import "SyphonVVBufferPoolAdditions.h"




GLBufferRef CreateBufferForSyphonClient(SyphonClient * c, const GLBufferPoolRef & inPoolRef)	{
	if (c == nil)	{
		NSLog(@"\t\terr: passed nil img %s",__func__);
		return nullptr;
	}
	if (inPoolRef==nullptr)
		return nullptr;
	
	GLContextRef		tmpCtx = inPoolRef->getContext();
	if (tmpCtx == nullptr)
		return nullptr;
	
	
	//	probably not necessary, but ensures that nothing else uses the GL context while we are- unlock as soon as we're done working with the context
	//pthread_mutex_lock(&contextLock);
	//	get a new image from the client!
	//SyphonImage			*newImage = [c newFrameImageForContext:[context CGLContextObj]];
	SyphonImage			*newImage = [c newFrameImageForContext:tmpCtx->ctx];
	//pthread_mutex_unlock(&contextLock);
	if (newImage == nil)
		return nullptr;
	
	VVGL::Rect			newSrcRect(0,0,0,0);
	NSSize				tmpSize = [newImage textureSize];
	newSrcRect.size = VVGL::Size(tmpSize.width, tmpSize.height);
	
	/*		make and configure the buffer i'll be returning.  syphon actually created the GL texture, 
	so instead of asking the vvgl lib to allocate a texture, we're just going to 
	create a VVBuffer instance, populate it with the properties of the texture created by Syphon, 
	and then set up the VVBuffer to retain the underlying SyphonImage resource (which will be freed 
	when the VVBuffer is released)		*/
	//VVBuffer			*returnMe = [[VVBuffer alloc] initWithPool:self];
	GLBufferRef		returnMe = make_shared<GLBuffer>(inPoolRef);
	inPoolRef->timestampThisBuffer(returnMe);		//	timestamp the buffer.  if done judiciously, can help with tracking "new" content
	GLBuffer::Descriptor	&desc = returnMe->desc;	//	get the buffer's descriptor.  this is a ptr to a C structure which contains the basic properties of the buffer.  C struct because its contents are used to compare buffers/locate "free" buffers in the pool, and as such its contents need to be immediately accessible
	
	desc.type = GLBuffer::Type_Tex;						//	it's a GL texture resources (there are other values in this typdef representing other types of resources)
	desc.target = GLBuffer::Target_Rect;			//	this is what kind of texture it is (rect or 2d)
	desc.internalFormat = GLBuffer::IF_RGBA8;			//	the GL texture internal format (tex will be created with this property- if tex is created externally, this should match the tex's actual properties)
	desc.pixelFormat = GLBuffer::PF_BGRA;				//	the GL texture pixel format (tex will be created with this property- if tex is created externally, this should match the tex's actual properties)
	desc.pixelType = GLBuffer::PT_UInt_8888_Rev;		//	the GL texture pixel type (tex will be created with this property- if tex is created externally, this should match the tex's actual properties)
	desc.cpuBackingType = GLBuffer::Backing_None;		//	what kind of CPU backing there is- none, internal (created/freed by this framework/API), or external (created/freed by another API)
	desc.gpuBackingType = GLBuffer::Backing_External;	//	what kind of GPU backing there is- none, internal (created/freed by this framework/API), or external (created/freed by another API)
	//desc.name = [newImage textureName];				//	the actual GL texture name.  this is what you bind when you want to draw a texture.
	desc.texRangeFlag = false;							//	reserved, set to NO for now.
	desc.texClientStorageFlag = false;					//	reserved, set to NO for now.
	desc.msAmount = 0;									//	only used with renderbuffers doing multi-sample anti-aliasing.  ignored with textures, set to 0.
	desc.localSurfaceID = 0;							//	only used when working with associating textures with IOSurfaces- set to 0.
	
	returnMe->name = [newImage textureName];
	returnMe->preferDeletion = true;	//	we want to make sure that this buffer isn't pooled (the VVBuffer is just a wrapper around a syphon-created and syphon-controlled GL resource- it doesn't belong in this buffer pool)
	returnMe->size = newSrcRect.size;	//	set the buffer's size.  the "size" is the size of the GL resource, and is always in pixels.
	returnMe->srcRect = newSrcRect;		//	set the buffer's "srcRect".  the "srcRect" is the area of the GL resource that is used to describe the image this VVBuffer represents.  the units are always in pixels (even if the buffer is a GL_TEXTURE_2D, and its tex coords are normalized).  this is used to describe images that don't occupy the full region of a texture, and do zero-cost cropping.  the srcRect is respected by everything in this framework.
	returnMe->backingSize = newSrcRect.size;	//	the backing size is the size (in pixels) of whatever's backing the GL resource.  there's no CPU backing in this case- just set it to be the same as the buffer's "size".
	returnMe->backingID = (VVGL::GLBuffer::BackingID)VVGLBufferBackingID_Syphon;	//	set the backing ID to indicate that this buffer was created by wrapping a syphon image.
	
	
	//	set up the buffer i'm returning to use this callback when it's released- we'll free the SyphonImage in this callback
	returnMe->backingReleaseCallback = [](GLBuffer& inBuffer, void* inReleaseContext)	{
		SyphonImage		*tmpImg = (SyphonImage*)inReleaseContext;
		if (tmpImg != nil)	{
			[tmpImg release];
		}
	};
	//	make sure the buffer i'm returning retains the image from the client!
	[newImage retain];
	returnMe->backingContext = newImage;
	
	//	the 'newImage' we got from the syphon client was retained, so release it
	[newImage release];
	return returnMe;
}




