#include "InterAppOutput_Win.h"

//#include <QtPlatformHeaders/QWindowsWindowFunctions>
//#include <QtPlatformHeaders/QXcbWindowFunctions>
//#import <QCocoaNativeContext>
//#include <QCocoaWindowFunctions>
//#include <QCocoaNativeContext>
//#include <QtPlatformHeaders/qcocoanativecontext.h>
//#include <QtPlatformHeaders/QCocoaNativeContext>
#include <QDebug>

#include "VVGL.hpp"
#include "SpoutSender.h"




using namespace VVGL;








class InterAppOutput_WinOpaque	{
public:
	InterAppOutput_WinOpaque()	{
		GLBufferPoolRef		bp = GetGlobalBufferPool();
		if (bp != nullptr)	{
			ctx = bp->context()->newContextSharingMe();
		}

		if (!txr.CreateSender("ISF Editor", 1920, 1080))
			qDebug() << "ERR: could not create spout sender, " << __PRETTY_FUNCTION__;
	}
	~InterAppOutput_WinOpaque()	{
		txr.ReleaseSender();
	}
	void moveGLToThread(const QThread * n)	{
		if (n!=nullptr && ctx!=nullptr)
			ctx->moveToThread(const_cast<QThread*>(n));
	}

	GLContextRef			ctx = nullptr;
	SpoutSender				txr;
};








InterAppOutput_Win::InterAppOutput_Win(QObject *parent) :
	VideoOutput(parent)
{
	opaque = new InterAppOutput_WinOpaque();
}
InterAppOutput_Win::~InterAppOutput_Win()	{
	if (opaque != nullptr)	{
		delete opaque;
		opaque = nullptr;
	}
}


void InterAppOutput_Win::publishBuffer(const GLBufferRef & inBuffer)	{
	if (inBuffer == nullptr)
		return;
	if (opaque == nullptr)	{
		opaque = new InterAppOutput_WinOpaque();
	}

	if (opaque==nullptr || opaque->ctx==nullptr)	{
		qDebug() << "err: bailing, opaque is null, " << __PRETTY_FUNCTION__;
		return;
	}

	//opaque->ctx->makeCurrent();

	//cout << "\tspout should be publishing buffer " << *inBuffer << endl;
	opaque->txr.SendTexture(inBuffer->name, inBuffer->desc.target, inBuffer->srcRect.size.width, inBuffer->srcRect.size.height, !inBuffer->flipped);
}

void InterAppOutput_Win::moveGLToThread(const QThread * n)	{
	opaque->moveGLToThread(n);
}

