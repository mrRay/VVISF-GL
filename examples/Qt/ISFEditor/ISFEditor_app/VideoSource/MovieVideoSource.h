#ifndef MOVIEVIDEOSOURCE_H
#define MOVIEVIDEOSOURCE_H

#include <QObject>

#include "VideoSource.h"




class MovieVideoSource : public VideoSource
{
	Q_OBJECT
	
public:
	MovieVideoSource(QObject *parent = nullptr);
	~MovieVideoSource();
	
	//virtual VVGL::GLBufferRef getBuffer() override;
	virtual void start() override;
	virtual void stop() override;
	virtual bool playingBackItem(const MediaFile & n) override;
	virtual void loadFile(const MediaFile & n) override;
	
private:
	
};




#endif // MOVIEVIDEOSOURCE_H