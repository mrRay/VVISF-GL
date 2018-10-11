#ifndef INTERAPPVIDEOSOURCE_H
#define INTERAPPVIDEOSOURCE_H

#include <QObject>

#include "VideoSource.h"




class InterAppVideoSource : public VideoSource
{
	Q_OBJECT
	
public:
	InterAppVideoSource(QObject *parent = nullptr);
	~InterAppVideoSource();
	
	virtual VVGL::GLBufferRef getBuffer() override;
	virtual void start() override;
	virtual void stop() override;
	virtual bool playingBackItem(const MediaFile & n) override;
	virtual void loadFile(const MediaFile & n) override;
	
private:
	
};




#endif // INTERAPPVIDEOSOURCE_H