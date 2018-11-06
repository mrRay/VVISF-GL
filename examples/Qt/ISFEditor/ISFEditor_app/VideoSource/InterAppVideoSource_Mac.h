#ifndef INTERAPPVIDEOSOURCE_MAC_H
#define INTERAPPVIDEOSOURCE_MAC_H

#include <QObject>

#include "VideoSource.h"

class InterAppVideoSource_MacOpaque;




class InterAppVideoSource_Mac : public VideoSource
{
	Q_OBJECT
	
public:
	InterAppVideoSource_Mac(QObject *parent = nullptr);
	~InterAppVideoSource_Mac();
	
	//virtual VVGL::GLBufferRef getBuffer() override;
	virtual QList<MediaFile> createListOfStaticMediaFiles() override;
	virtual void start() override;
	virtual void stop() override;
	virtual bool playingBackItem(const MediaFile & n) override;
	virtual void loadFile(const MediaFile & n) override;
	
private:
	InterAppVideoSource_MacOpaque		*opaque = nullptr;	//	used to store objective-c stuff
};




#endif // INTERAPPVIDEOSOURCE_MAC_H