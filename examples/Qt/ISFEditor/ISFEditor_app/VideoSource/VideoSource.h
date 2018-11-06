#ifndef VIDEOSOURCE_H
#define VIDEOSOURCE_H

#include <mutex>

#include <QObject>

#include "MediaFile.h"
#include "VVGL.hpp"




class VideoSource : public QObject
{
	Q_OBJECT
	
public:
	explicit VideoSource(QObject *parent = nullptr) : QObject(parent)	{}
	
	//virtual VVGL::GLBufferRef getBuffer() = 0;
	virtual QList<MediaFile> createListOfStaticMediaFiles() { return QList<MediaFile>(); }
	virtual void start() { std::lock_guard<std::recursive_mutex> tmpLock(_lock); _running=true; }
	//	must be called on eject- sets _running to false and sets _file to a null file
	virtual void stop() { std::lock_guard<std::recursive_mutex> tmpLock(_lock); _running=false; _file=MediaFile(); }
	virtual bool playingBackItem(const MediaFile & n) { Q_UNUSED(n); return false; }
	virtual void loadFile(const MediaFile & n) = 0;
	
	virtual bool running() { std::lock_guard<std::recursive_mutex> tmpLock(_lock); return _running; }
	
signals:
	void staticSourceUpdated(VideoSource * n);
	void frameProduced(VVGL::GLBufferRef n);

protected:
	std::recursive_mutex	_lock;
	bool					_running;
	MediaFile				_file;
};




#endif // VIDEOSOURCE_H
