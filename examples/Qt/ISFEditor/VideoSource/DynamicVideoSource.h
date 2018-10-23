#ifndef DYNAMICVIDEOSOURCE_H
#define DYNAMICVIDEOSOURCE_H

#include <mutex>

#include <QObject>

#include "WebCamVideoSource.h"
#include "MovieVideoSource.h"
#include "ImgVideoSource.h"
#if defined(Q_OS_MAC)
#include "InterAppVideoSource_Mac.h"
//#elif defined(Q_OS_WIN)
#else
#include "InterAppVideoSource.h"
#endif
#include "MediaFile.h"

#include "VVGL.hpp"




class DynamicVideoSource : public QObject
{
	Q_OBJECT
	
public:
	explicit DynamicVideoSource(QObject *parent = nullptr);
	~DynamicVideoSource();
	
	//	may cause the source to render- either returns the current buffer or the last-rendered buffer (always returns something once the source has been started)
	VVGL::GLBufferRef getBuffer();
	//	returns a list of media files that should be displayed in the combo box with the list of static sources
	QList<MediaFile> createListOfStaticMediaFiles();
	//	returns the type of the currently loaded source file
	MediaFile::Type srcType();
	
	//	loads the passed file
	void loadFile(const MediaFile & n);
	//	returns true if the receiver is playing back the passed file or a file equivalent to the passed file
	bool playingBackItem(const MediaFile & n);

signals:
	//	this signal is emitted whenever any of the sources update their list of static media files (like if a new webcam is plugged in or a new inter-app source appears)
	Q_SIGNAL void listOfStaticSourcesUpdated(DynamicVideoSource * inSrc);
	//	this signal is emitted whenever a new frame has been produced by the source
	Q_SIGNAL void frameProduced(VVGL::GLBufferRef n);



private:
	//	the file that's currently loaded (will be an "empty file"/MediaFile::Type_None if no file is playing back)
	std::recursive_mutex	srcLock;
	MediaFile				srcFile;
	
	//	these are the actual video sources
	WebCamVideoSource		camSrc;
	MovieVideoSource		movSrc;
	ImgVideoSource			imgSrc;
#if defined(Q_OS_MAC)
	InterAppVideoSource_Mac		appSrc;
//#elif defined(Q_OS_WIN)
#else
	InterAppVideoSource		appSrc;
#endif
	
	//	the last-rendered buffer and a mutex to lock it
	std::recursive_mutex	lastBufferLock;
	VVGL::GLBufferRef		lastBuffer = nullptr;
	
private:
	//	stops playback of all sources
	void stopSources();
	inline MediaFile::Type _srcType() const { return srcFile.type(); }
	
private slots:
	//	the video sources connect to this- this is how they notify the receiver that their list of static media files has updated
	void staticSourceUpdated(VideoSource * inSrc);
};




//	returns the global singleton
DynamicVideoSource * GetDynamicVideoSource();




#endif // DYNAMICVIDEOSOURCE_H