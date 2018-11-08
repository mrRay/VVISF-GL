#include "InterAppOutput_Mac.h"

//#include <QtPlatformHeaders/QWindowsWindowFunctions>
//#include <QtPlatformHeaders/QXcbWindowFunctions>
//#import <QCocoaNativeContext>
//#include <QCocoaWindowFunctions>
//#include <QCocoaNativeContext>
//#include <QtPlatformHeaders/qcocoanativecontext.h>
#include <QtPlatformHeaders/QCocoaNativeContext>
#include <QDebug>

//#import "Syphon.h"
#import <Syphon/Syphon.h>

#import <Cocoa/Cocoa.h>

#include "VVGL.hpp"




using namespace VVGL;








class InterAppOutput_MacOpaque	{
public:
	InterAppOutput_MacOpaque()	{
		ctx = GetGlobalBufferPool()->context()->newContextSharingMe();
		
		QVariant		nativeHandle = ctx->nativeHandle();
		if (nativeHandle.type() != QVariant::nameToType("QCocoaNativeContext"))	{
			qDebug() << "ERR: variant (" << nativeHandle << ") is wrong type, " << __PRETTY_FUNCTION__;
		}
		else	{
			QCocoaNativeContext		nativeCtx = nativeHandle.value<QCocoaNativeContext>();
			server = [[SyphonServer alloc]
				initWithName:@"ISF Test App"
				context:[nativeCtx.context() CGLContextObj]
				options:nil];
		}
		
		
	}
	~InterAppOutput_MacOpaque()	{
	}
	
	GLContextRef			ctx = nullptr;
	__block SyphonServer			*server = nil;
};








InterAppOutput_Mac::InterAppOutput_Mac(QObject *parent) :
	VideoOutput(parent)
{
	opaque = new InterAppOutput_MacOpaque();
}
InterAppOutput_Mac::~InterAppOutput_Mac()	{
	if (opaque != nullptr)	{
		delete opaque;
		opaque = nullptr;
	}
}


void InterAppOutput_Mac::publishBuffer(const GLBufferRef & inBuffer)	{
	if (inBuffer == nullptr)
		return;
	if (opaque == nullptr)	{
		opaque = new InterAppOutput_MacOpaque();
	}
	
	if (opaque==nullptr || opaque->ctx==nullptr || opaque->server==nullptr)	{
		qDebug() << "err: bailing, opaque is null, " << __PRETTY_FUNCTION__;
		return;
	}
	
	opaque->ctx->makeCurrent();
	@autoreleasepool	{
		[opaque->server
			publishFrameTexture:inBuffer->name
			textureTarget:inBuffer->desc.target
			imageRegion:NSMakeRect(inBuffer->srcRect.origin.x, inBuffer->srcRect.origin.y, inBuffer->srcRect.size.width, inBuffer->srcRect.size.height)
			textureDimensions:NSMakeSize(inBuffer->size.width, inBuffer->size.height)
			flipped:inBuffer->flipped];
	}
}

