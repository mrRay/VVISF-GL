#ifndef IMGVIDEOSOURCE_H
#define IMGVIDEOSOURCE_H

#include <QObject>

#include "VideoSource.h"




class ImgVideoSource : public VideoSource
{
	Q_OBJECT
	
public:
	ImgVideoSource(QObject *parent = nullptr);
	~ImgVideoSource();
	
	//virtual VVGL::GLBufferRef getBuffer() override;
	virtual void start() override;
	virtual void stop() override;
	virtual bool playingBackItem(const MediaFile & n) override;
	virtual void loadFile(const MediaFile & n) override;
	
private:
	
};




#endif // IMGVIDEOSOURCE_H