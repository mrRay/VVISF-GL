#ifndef INTERAPPVIDEOSOURCE_H
#define INTERAPPVIDEOSOURCE_H

#include <QObject>

#include "VideoSource.h"

#if defined(Q_OS_MAC)
#include "InterAppVideoSource_Mac.h"
#elif defined(Q_OS_WIN)
#include "InterAppVideoSource_Win.h"
#endif




//	platform-neutral wrapper class for platform-specific classes
class InterAppVideoSource : public VideoSource
{
	Q_OBJECT
	
public:
	InterAppVideoSource(QObject *parent = nullptr);
	~InterAppVideoSource();
	
	//virtual VVGL::GLBufferRef getBuffer() override;
	virtual QList<MediaFile> createListOfStaticMediaFiles() override;
	virtual void start() override;
	virtual void stop() override;
	virtual bool playingBackItem(const MediaFile & n) override;
	virtual void loadFile(const MediaFile & n) override;
	
	virtual bool running() override;
	virtual void renderABuffer() override;
	
private:
#if defined(Q_OS_MAC)
	InterAppVideoSource_Mac		src;
#elif defined(Q_OS_WIN)
	InterAppVideoSource_Win		src;
#endif
	int				derp;
};




#endif // INTERAPPVIDEOSOURCE_H
