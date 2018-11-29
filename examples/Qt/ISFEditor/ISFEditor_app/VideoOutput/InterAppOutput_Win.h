#ifndef INTERAPPOUTPUT_WIN_H
#define INTERAPPOUTPUT_WIN_H

#include <QObject>

#include "VideoOutput.h"

class InterAppOutput_WinOpaque;




class InterAppOutput_Win : public VideoOutput
{
	Q_OBJECT

public:
	explicit InterAppOutput_Win(QObject *parent = nullptr);
	~InterAppOutput_Win();

	virtual void publishBuffer(const VVGL::GLBufferRef & inBuffer) override;

private:
	InterAppOutput_WinOpaque		*opaque = nullptr;	//	used to store objective-c stuff
};

#endif // INTERAPPOUTPUT_WIN_H
