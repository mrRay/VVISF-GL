#include "VVGLBuffer.hpp"
#include "VVGLBufferPool.hpp"

#include <mutex>




namespace VVGL
{


using namespace std;




uint32_t VVGLBuffer::Descriptor::backingLengthForSize(const Size & s) const	{
	uint32_t		bytesPerRow = 4 * s.width;
	
	switch (this->pixelType)	{
	case PT_Float:
		switch (this->internalFormat)	{
#if !ISF_TARGET_RPI
		case IF_R:
			bytesPerRow = 32 * 1 * s.width / 8;
			break;
#endif
		default:
			bytesPerRow = 32 * 4 * s.width / 8;
			break;
		}
		break;
#if !ISF_TARGET_RPI
	case PT_HalfFloat:
		switch (this->internalFormat)	{
		case IF_R:
			bytesPerRow = 32 * 1 * s.width / 8;
			break;
		case IF_Depth24:
			bytesPerRow = 32 * 2 * s.width / 8;
			break;
		case IF_RGB:
			bytesPerRow = 32 * 3 * s.width / 8;
			break;
		case IF_RGBA:
		case IF_RGBA32F:
			bytesPerRow = 32 * 4 * s.width / 8;
			break;
		//case IF_RGBA16F:
		//	bytesPerRow = 16 * 4 * s.width / 8;
		//	break;
		default:
			break;
		}
		break;
#endif
	case PT_UByte:
		switch (this->internalFormat)	{
#if !ISF_TARGET_RPI
		case IF_R:
			bytesPerRow = 8 * 1 * s.width / 8;
			break;
#endif
		default:
			bytesPerRow = 8 * 4 * s.width / 8;
			break;
		}
		break;
#if !ISF_TARGET_IOS && !ISF_TARGET_RPI
	case PT_UInt_8888_Rev:
		bytesPerRow = 8 * 4 * s.width / 8;
		break;
#endif
	case PT_UShort88:
		bytesPerRow = 8 * 2 * s.width / 8;
		break;
	}
	
	switch (this->internalFormat)	{
#if !ISF_TARGET_IOS && !ISF_TARGET_RPI
	case IF_RGB_DXT1:
	case IF_A_RGTC:
		bytesPerRow = 4 * s.width / 8;
		break;
	case IF_RGBA_DXT5:
		bytesPerRow = 8 * s.width / 8;
		break;
#endif
	default:
		break;
	}
	
	return bytesPerRow * s.height;
}


/*	========================================	*/
#pragma mark --------------------- create/destroy


VVGLBuffer::VVGLBuffer(VVGLBufferPoolRef inParentPool)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	
	parentBufferPool = inParentPool;
}

VVGLBuffer::VVGLBuffer(const VVGLBuffer & n)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	
	desc = n.desc;
	name = n.name;
	preferDeletion = n.preferDeletion;
	size = n.size;
	srcRect = n.srcRect;
	flipped = n.flipped;
	backingSize = n.backingSize;
	contentTimestamp = n.contentTimestamp;
	
	backingReleaseCallback = n.backingReleaseCallback;
	backingContext = n.backingContext;
	
	backingID = n.backingID;
	cpuBackingPtr = n.cpuBackingPtr;
#if ISF_TARGET_MAC
	//setUserInfo(n.getUserInfo());
	setLocalSurfaceRef(n.getLocalSurfaceRef());
	setRemoteSurfaceRef(n.getRemoteSurfaceRef());
#endif
	
	parentBufferPool = n.parentBufferPool;
}

VVGLBuffer::~VVGLBuffer()	{
	//cout << __PRETTY_FUNCTION__ << endl;
	
	//	if this buffer was created by copying another buffer, clear out the source buffer
	if (copySourceBuffer != nullptr)	{
		//cout << "\tbuffer was copied, neither pooling nor releasing" << endl;
		copySourceBuffer = nullptr;
	}
	//	else this buffer was created (not copied) it has resources that need to be freed
	else	{
		//cout << "\tbuffer was created, may have resources to free\n";
		//	if the cpu backing was external, free the resource immediately (assume i can't write into it again)
		if (desc.cpuBackingType == Backing_External)	{
			//	if the gpu backing was internal, release it
			if (desc.gpuBackingType == Backing_Internal)	{
				if (parentBufferPool != nullptr)	{
					//(*parentBufferPool).releaseBufferResources(this);
					parentBufferPool->releaseBufferResources(this);
				}
			}
			//	else the gpu backing was external or non-existent: do nothing
		}
		//	else the cpu backing as internal, or there's no cpu backing
		else	{
			//	if the gpu backing was internal, release it
			if (desc.gpuBackingType == Backing_Internal)	{
				//	if my idleCount is 0, i'm being freed from rendering and i go back in the pool
				if (idleCount==0 && !preferDeletion)	{
					if (parentBufferPool != nullptr)	{
						//(*parentBufferPool).returnBufferToPool(this);
						parentBufferPool->returnBufferToPool(this);
					}
				}
				//	else i was in the pool (or i just want to be deleted), and now the resources i contain need to be freed
				else	{
					if (parentBufferPool != nullptr)	{
						//(*parentBufferPool).releaseBufferResources(this);
						parentBufferPool->releaseBufferResources(this);
					}
				}
			}
			//	else this buffer was either external, or non-existent: do nothing
		}
		//	now call the backing release callback
		if (backingReleaseCallback != nullptr)	{
			backingReleaseCallback(*this, backingContext);
		}
	}
	
#if ISF_TARGET_MAC
	//setUserInfo(nullptr);
	setLocalSurfaceRef(nullptr);
	setRemoteSurfaceRef(nullptr);
#endif
	
	parentBufferPool = nullptr;
	
}
ostream & operator<<(ostream & os, const VVGLBuffer & n)	{
	//os << "<VVGLBuffer " << n.name << ", " << (int)n.size.width << "x" << (int)n.size.height << ">";
	
	os << n.getDescriptionString();
	return os;
}


/*	========================================	*/
#pragma mark --------------------- getter/setter- mac stuff


#if ISF_TARGET_MAC
/*
id VVGLBuffer::getUserInfo() const	{
	return userInfo;
}
void VVGLBuffer::setUserInfo(id n)	{
	if (userInfo!=nil)
		CFRelease(userInfo);
	userInfo = (n==NULL) ? nullptr : (id)CFRetain(n);
}
*/
IOSurfaceRef VVGLBuffer::getLocalSurfaceRef() const	{
	return localSurfaceRef;
}

void VVGLBuffer::setLocalSurfaceRef(const IOSurfaceRef & n)	{
	if (localSurfaceRef != NULL)
		CFRelease(localSurfaceRef);
	localSurfaceRef = (IOSurfaceRef)n;
	if (localSurfaceRef != NULL)	{
		CFRetain(localSurfaceRef);
		desc.localSurfaceID = IOSurfaceGetID(localSurfaceRef);
		//	can't have a remote surface if i just made a local surface...
		setRemoteSurfaceRef(NULL);
	}
	else
		desc.localSurfaceID = 0;
}

IOSurfaceRef VVGLBuffer::getRemoteSurfaceRef() const	{
	return remoteSurfaceRef;
}

void VVGLBuffer::setRemoteSurfaceRef(const IOSurfaceRef & n)	{
	if (remoteSurfaceRef != NULL)
		CFRelease(remoteSurfaceRef);
	remoteSurfaceRef = (IOSurfaceRef)n;
	if (remoteSurfaceRef != NULL)	{
		CFRetain(remoteSurfaceRef);
		//	can't have a local surface if i've got a remote surface!
		setLocalSurfaceRef(NULL);
		
		preferDeletion = true;
	}
}
#endif	//	ISF_TARGET_MAC


/*	========================================	*/
#pragma mark --------------------- public methods


bool VVGLBuffer::isComparableForRecycling(const VVGLBuffer::Descriptor & n) const	{
	//	if any of these things DON'T match, return false- the comparison failed
	if ((desc.type != n.type)	||
	//(this->backingID != n.backingID)	||
	(desc.cpuBackingType != n.cpuBackingType)	||
	(desc.gpuBackingType != n.gpuBackingType)	||
	(desc.target != n.target)	||
	(desc.internalFormat != n.internalFormat)	||
	(desc.pixelFormat != n.pixelFormat)	||
	(desc.pixelType != n.pixelType) ||
	//(name != n.name) ||
	(desc.texRangeFlag != n.texRangeFlag)	||
	(desc.texClientStorageFlag != n.texClientStorageFlag)	||
	(desc.msAmount != n.msAmount)
	)	{
		return false;
	}
	
	//	...if i'm here, all of the above things matched
	
	//	if neither "wants" a local IOSurface, this is a match- return true
	if (desc.localSurfaceID==0 && n.localSurfaceID==0)
		return true;
	//	if both have a local IOSurface, this is a match- even if the local IOSurfaces aren't an exact match
	if (desc.localSurfaceID!=0 && n.localSurfaceID!=0)
		return true;
	
	return true;
}
uint32_t VVGLBuffer::backingLengthForSize(Size s) const	{
	return desc.backingLengthForSize(s);
}

Rect VVGLBuffer::glReadySrcRect() const	{
#if ISF_TARGET_MAC
	if (this->desc.target == Target_Rect)
		return srcRect;
#endif
	return { this->srcRect.origin.x/this->size.width, this->srcRect.origin.y/this->size.height, this->srcRect.size.width/this->size.width, this->srcRect.size.height/this->size.height };
}
/*
Rect VVGLBuffer::croppedSrcRect(Rect & cropRect, bool & takeFlipIntoAccount) const	{
	Rect		flippedCropRect = cropRect;
	if (takeFlipIntoAccount && this->flipped)
		flippedCropRect.origin.y = (1.0 - cropRect.size.height - cropRect.origin.y);
	
	Rect		returnMe = { 0., 0., 0., 0. };
	returnMe.size = { this->srcRect.size.width*flippedCropRect.size.width, this->srcRect.size.height*flippedCropRect.size.height };
	returnMe.origin.x = flippedCropRect.origin.x*this->srcRect.size.width + this->srcRect.origin.x;
	returnMe.origin.y = flippedCropRect.origin.y*this->srcRect.size.height + this->srcRect.origin.y;
	return returnMe;
}
*/
bool VVGLBuffer::isFullFrame() const	{
	if (this->srcRect.origin.x==0.0 && this->srcRect.origin.y==0.0 && this->srcRect.size.width==this->size.width && this->srcRect.size.height==this->size.height)
		return true;
	return false;
}

bool VVGLBuffer::isNPOT2DTex() const	{
	bool		returnMe = true;
	if (this->desc.target==Target_2D)	{
		int			tmpInt;
		tmpInt = 1;
		while (tmpInt<this->size.width)	{
			tmpInt <<= 1;
		}
		if (tmpInt==this->size.width)	{
			tmpInt = 1;
			while (tmpInt<this->size.height)	{
				tmpInt<<=1;
			}
			if (tmpInt==this->size.height)
				returnMe = false;
		}
	}
	else
		returnMe = false;
	return returnMe;
}

bool VVGLBuffer::isPOT2DTex() const	{
	bool		returnMe = false;
	if (this->desc.target==Target_2D)	{
		int			tmpInt;
		tmpInt = 1;
		while (tmpInt<this->size.width)	{
			tmpInt <<= 1;
		}
		if (tmpInt==this->size.width)	{
			tmpInt = 1;
			while (tmpInt<this->size.height)	{
				tmpInt<<=1;
			}
			if (tmpInt==this->size.height)
				returnMe = true;
		}
	}
	else
		returnMe = false;
	return returnMe;
}

#if ISF_TARGET_MAC
bool VVGLBuffer::safeToPublishToSyphon() const	{
	if (localSurfaceRef == nil)
		return false;
	if (flipped || desc.pixelFormat!=PF_BGRA)
		return false;
	if (srcRect.origin.x==0. && srcRect.origin.y==0. && srcRect.size.width==size.width && srcRect.size.height==size.height)
		return true;
	return false;
}
#endif

bool VVGLBuffer::isContentMatch(VVGLBuffer & n) const	{
	return this->contentTimestamp == n.contentTimestamp;
}
/*
void VVGLBuffer::draw(const Rect & dst) const	{
	if (desc.type != Type_Tex)
		return;
#if ISF_TARGET_MAC
	//inCtx.makeCurrentIfNotCurrent();
	float			verts[] = {
		(float)MinX(dst), (float)MinY(dst), 0.0,
		(float)MaxX(dst), (float)MinY(dst), 0.0,
		(float)MaxX(dst), (float)MaxY(dst), 0.0,
		(float)MinX(dst), (float)MaxY(dst), 0.0
	};
	Rect			src = glReadySrcRect();
	float			texs[] = {
		(float)MinX(src), (flipped) ? (float)MaxY(src) : (float)MinY(src),
		(float)MaxX(src), (flipped) ? (float)MaxY(src) : (float)MinY(src),
		(float)MaxX(src), (flipped) ? (float)MinY(src) : (float)MaxY(src),
		(float)MinX(src), (flipped) ? (float)MinY(src) : (float)MaxY(src)
	};
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	
	glVertexPointer(3, GL_FLOAT, 0, verts);
	glTexCoordPointer(2, GL_FLOAT, 0, texs);
	glBindTexture(desc.target, name);
	glDrawArrays(GL_QUADS, 0, 4);
	glBindTexture(desc.target, 0);
#else
	cout << "\tincomplete- " << __PRETTY_FUNCTION__ << endl;
#endif
	
}
*/
string VVGLBuffer::getDescriptionString() const	{
	if (this == nullptr)
		return string("nullptr");
	char		typeChar = '?';
	switch (this->desc.type)	{
	case VVGLBuffer::Type_RB:	typeChar='R'; break;
	case VVGLBuffer::Type_FBO:	typeChar='F'; break;
	case VVGLBuffer::Type_Tex:	typeChar='T'; break;
	case VVGLBuffer::Type_PBO:	typeChar='P'; break;
	case VVGLBuffer::Type_VBO:	typeChar='V'; break;
	case VVGLBuffer::Type_EBO:	typeChar='E'; break;
#if ISF_TARGET_GL3PLUS || ISF_TARGET_GLES3
	case VVGLBuffer::Type_VAO:	typeChar='A'; break;
#endif
	}
	return FmtString("<VVGLBuffer %c, %d, %dx%d>",typeChar,name,(int)this->size.width,(int)this->size.height);
}


/*	========================================	*/
#pragma mark --------------------- VVGLBuffer copy function


VVGLBufferRef VVGLBufferCopy(const VVGLBufferRef & n)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	
	if (n==nullptr)
		return nullptr;
	VVGLBuffer			*srcBuffer = n.get();
	if (srcBuffer == nullptr)
		return nullptr;
	VVGLBufferRef		returnMe = make_shared<VVGLBuffer>(srcBuffer->parentBufferPool);
	VVGLBuffer			*newBuffer = returnMe.get();
	
	//(*newBuffer).desc = (*srcBuffer).desc;
	newBuffer->desc = srcBuffer->desc;
	
	newBuffer->name = srcBuffer->name;
	newBuffer->preferDeletion = true;	//	we want the copy to be deleted immediately
	newBuffer->size = srcBuffer->size;
	newBuffer->srcRect = srcBuffer->srcRect;
	newBuffer->flipped = srcBuffer->flipped;
	newBuffer->backingSize = srcBuffer->backingSize;
	newBuffer->contentTimestamp = srcBuffer->contentTimestamp;
	
#if ISF_TARGET_MAC
	newBuffer->setLocalSurfaceRef(srcBuffer->getLocalSurfaceRef());
	newBuffer->setRemoteSurfaceRef(srcBuffer->getRemoteSurfaceRef());
#endif
	
	newBuffer->copySourceBuffer = n;	//	the copy needs a smart ptr so the buffer it's based on is retained
	
	return returnMe;
}




}

