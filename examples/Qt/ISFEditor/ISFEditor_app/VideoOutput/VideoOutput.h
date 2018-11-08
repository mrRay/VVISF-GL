#ifndef VIDEOOUTPUT_H
#define VIDEOOUTPUT_H

#include <QObject>
#include <QString>

#include <mutex>

#include "VVGL.hpp"




class VideoOutput : public QObject
{
	Q_OBJECT
public:
	explicit VideoOutput(QObject *parent = nullptr) : QObject(parent) {}
	
	virtual void publishBuffer(const VVGL::GLBufferRef & inBuffer) = 0;

protected:
	std::recursive_mutex	_lock;
	bool					_running = false;
	QString					_humanName = QString("");
	QString					_uid = QString("");
};




#endif // VIDEOOUTPUT_H
