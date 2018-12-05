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
	
	//	tells the source to render a buffer- used for source types that have to be explicitly told to render frames
	virtual void renderABuffer() {}
	//	sets the size of the source- only really applicable where the size is customizable (ISF sources, for example)
	virtual void setRenderSize(const VVGL::Size & n) { std::lock_guard<std::recursive_mutex> tmpLock(_lock); _size=n; }
	
	virtual bool running() { std::lock_guard<std::recursive_mutex> tmpLock(_lock); return _running; }
	
signals:
	void staticSourceUpdated(VideoSource * n);
	void frameProduced(VVGL::GLBufferRef n);

protected:
	std::recursive_mutex	_lock;
	bool					_running;
	MediaFile				_file;
	VVGL::Size				_size = VVGL::Size(640,480);
};




#endif // VIDEOSOURCE_H
