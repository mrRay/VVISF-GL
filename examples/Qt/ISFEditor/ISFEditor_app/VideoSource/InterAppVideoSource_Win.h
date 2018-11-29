#ifndef INTERAPPVIDEOSOURCE_WIN_H
#define INTERAPPVIDEOSOURCE_WIN_H

#include <QObject>

#include "VideoSource.h"

class InterAppVideoSource_WinOpaque;




class InterAppVideoSource_Win : public VideoSource
{
	Q_OBJECT
public:
	InterAppVideoSource_Win(QObject *parent = nullptr);
	~InterAppVideoSource_Win();

	//virtual VVGL::GLBufferRef getBuffer() override;
	virtual QList<MediaFile> createListOfStaticMediaFiles() override;
	virtual void start() override;
	virtual void stop() override;
	virtual bool playingBackItem(const MediaFile & n) override;
	virtual void loadFile(const MediaFile & n) override;

private:
	InterAppVideoSource_WinOpaque		*opaque = nullptr;	//	used to store objective-c stuff
};

#endif // INTERAPPVIDEOSOURCE_WIN_H
