#include "InterAppOutput.h"




using namespace VVGL;




InterAppOutput::InterAppOutput(QObject *parent) :
	VideoOutput(parent)
{
}




void InterAppOutput::publishBuffer(const GLBufferRef & inBuffer)	{
	output.publishBuffer(inBuffer);
}

