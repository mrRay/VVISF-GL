#ifndef INTERAPPOUTPUT_H
#define INTERAPPOUTPUT_H

#include <QObject>

#include "VideoOutput.h"

#if defined(Q_OS_MAC)
#include "InterAppOutput_Mac.h"
#elif defined(Q_OS_WIN)
#endif




class InterAppOutput : public VideoOutput
{
	Q_OBJECT
	
public:
	explicit InterAppOutput(QObject *parent = nullptr);
	
	virtual void publishBuffer(const VVGL::GLBufferRef & inBuffer) override;

private:
#if defined(Q_OS_MAC)
	InterAppOutput_Mac			output;
#elif defined(Q_OS_WIN)
#endif
};




#endif // INTERAPPOUTPUT_H