#ifndef INTERAPPOUTPUT_MAC_H
#define INTERAPPOUTPUT_MAC_H

#include <QObject>

#include "VideoOutput.h"

class InterAppOutput_MacOpaque;




class InterAppOutput_Mac : public VideoOutput
{
	Q_OBJECT
	
public:
	explicit InterAppOutput_Mac(QObject *parent = nullptr);
	~InterAppOutput_Mac();
	
	virtual void publishBuffer(const VVGL::GLBufferRef & inBuffer) override;

private:
	InterAppOutput_MacOpaque		*opaque = nullptr;	//	used to store objective-c stuff
};




#endif // INTERAPPOUTPUT_MAC_H