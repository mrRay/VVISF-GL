#include "InterAppOutput.h"

#include "ISFController.h"
#include <QThread>




using namespace VVGL;




InterAppOutput::InterAppOutput(QObject *parent) :
	VideoOutput(parent)
{
	ISFController		*isfc = GetISFController();
	QThread				*rt = (isfc==nullptr) ? nullptr : isfc->renderThread();
	if (rt != nullptr)
		output.moveGLToThread(rt);
	else
		qDebug() << "ERR: render thread NULL in " << __PRETTY_FUNCTION__;
}




void InterAppOutput::publishBuffer(const GLBufferRef & inBuffer)	{
	output.publishBuffer(inBuffer);
}
void InterAppOutput::moveGLToThread(const QThread * n)	{
	output.moveGLToThread(n);
}

