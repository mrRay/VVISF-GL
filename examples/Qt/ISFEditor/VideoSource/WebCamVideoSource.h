#ifndef WEBCAMVIDEOSOURCE_H
#define WEBCAMVIDEOSOURCE_H

#include <QObject>
#include <QCamera>
#include <QCameraInfo>

#include "VVGL.hpp"

#include "VideoSource.h"
#include "GLBufferQVideoSurface.h"




class WebCamVideoSource : public VideoSource
{
	Q_OBJECT
	
public:
	WebCamVideoSource(QObject *parent = nullptr);
	~WebCamVideoSource();
	
	//virtual VVGL::GLBufferRef getBuffer() override;
	virtual QList<MediaFile> createListOfStaticMediaFiles() override;
	virtual void start() override;
	virtual void stop() override;
	virtual bool playingBackItem(const MediaFile & n) override;
	virtual void loadFile(const MediaFile & n) override;
	
private:
	QCamera			*_cam = nullptr;
	GLBufferQVideoSurface	*_sfc = nullptr;
};




#endif // WEBCAMVIDEOSOURCE_H