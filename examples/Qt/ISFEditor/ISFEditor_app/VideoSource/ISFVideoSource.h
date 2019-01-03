#ifndef ISFVIDEOSOURCE_H
#define ISFVIDEOSOURCE_H

#include <QObject>

#include "VVISF.hpp"

#include "VideoSource.h"




class ISFVideoSource : public VideoSource
{
	Q_OBJECT
	
public:
	ISFVideoSource(QObject *parent = nullptr);
	~ISFVideoSource();
	
	//virtual VVGL::GLBufferRef getBuffer() override;
	virtual QList<MediaFile> createListOfStaticMediaFiles() override;
	virtual void start() override;
	virtual void stop() override;
	virtual bool playingBackItem(const MediaFile & n) override;
	virtual void loadFile(const MediaFile & n) override;
	
	virtual void renderABuffer() override;
	
private:
	VVISF::ISFSceneRef			scene = nullptr;
};




#endif // ISFVIDEOSOURCE_H
