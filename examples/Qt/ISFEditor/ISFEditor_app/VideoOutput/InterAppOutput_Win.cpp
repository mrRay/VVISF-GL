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




using namespace VVGL;








class InterAppOutput_WinOpaque	{
public:
	InterAppOutput_WinOpaque()	{
		GLBufferPoolRef		bp = GetGlobalBufferPool();
		if (bp != nullptr)	{
			ctx = bp->context()->newContextSharingMe();
		}
	}
	~InterAppOutput_WinOpaque()	{
	}

	GLContextRef			ctx = nullptr;
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

	opaque->ctx->makeCurrent();
}
