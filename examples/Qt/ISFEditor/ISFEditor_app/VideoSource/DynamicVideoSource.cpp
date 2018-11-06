#include "DynamicVideoSource.h"




using namespace std;
using namespace VVGL;

static DynamicVideoSource * globalDynamicVideoSource = nullptr;




DynamicVideoSource::DynamicVideoSource(QObject *parent) : QObject(parent)	{
	globalDynamicVideoSource = this;
	//	every time any video source's list of static sources update, we need to respond by emitting our own signal...
	connect(&camSrc, &VideoSource::staticSourceUpdated, this, &DynamicVideoSource::staticSourceUpdated);
	connect(&movSrc, &VideoSource::staticSourceUpdated, this, &DynamicVideoSource::staticSourceUpdated);
	connect(&imgSrc, &VideoSource::staticSourceUpdated, this, &DynamicVideoSource::staticSourceUpdated);
	connect(&appSrc, &VideoSource::staticSourceUpdated, this, &DynamicVideoSource::staticSourceUpdated);
	//	video sources emit a signal when they produce new frames...
	connect(&camSrc, &VideoSource::frameProduced, [&](GLBufferRef n) { { lock_guard<recursive_mutex> tmpLock(lastBufferLock); lastBuffer=n; } emit frameProduced(n); });
	connect(&movSrc, &VideoSource::frameProduced, [&](GLBufferRef n) { { lock_guard<recursive_mutex> tmpLock(lastBufferLock); lastBuffer=n; } emit frameProduced(n); });
	connect(&imgSrc, &VideoSource::frameProduced, [&](GLBufferRef n) { { lock_guard<recursive_mutex> tmpLock(lastBufferLock); lastBuffer=n; } emit frameProduced(n); });
	connect(&appSrc, &VideoSource::frameProduced, [&](GLBufferRef n) { { lock_guard<recursive_mutex> tmpLock(lastBufferLock); lastBuffer=n; } emit frameProduced(n); });
	
	//	when the app is about to quit we want to stop the sources
	connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(aboutToQuit()));
}
DynamicVideoSource::~DynamicVideoSource()	{
	//stopSources();
}


GLBufferRef DynamicVideoSource::getBuffer()	{
	lock_guard<recursive_mutex>		tmpLock(lastBufferLock);
	return lastBuffer;
}
QList<MediaFile> DynamicVideoSource::createListOfStaticMediaFiles()	{
	QList<MediaFile>		returnMe;
	QList<MediaFile>		tmpList;
	
	tmpList = camSrc.createListOfStaticMediaFiles();
	for (const MediaFile & tmpItem : tmpList)
		returnMe.append(tmpItem);
	
	tmpList = movSrc.createListOfStaticMediaFiles();
	for (const MediaFile & tmpItem : tmpList)
		returnMe.append(tmpItem);
	
	tmpList = imgSrc.createListOfStaticMediaFiles();
	for (const MediaFile & tmpItem : tmpList)
		returnMe.append(tmpItem);
	
	tmpList = appSrc.createListOfStaticMediaFiles();
	for (const MediaFile & tmpItem : tmpList)
		returnMe.append(tmpItem);
	
	return returnMe;
}
MediaFile::Type DynamicVideoSource::srcType()	{
	lock_guard<recursive_mutex>		tmpLock(srcLock);
	return srcFile.type();
}


void DynamicVideoSource::loadFile(const MediaFile & n)	{
	//qDebug() << __PRETTY_FUNCTION__;
	
	lock_guard<recursive_mutex>		tmpLock(srcLock);
	
	MediaFile::Type		origType = srcFile.type();
	MediaFile::Type		newType = n.type();
	
	if (origType != newType)	{
		switch (origType)	{
		case MediaFile::Type_None:
			break;
		case MediaFile::Type_Cam:
			camSrc.stop();
			break;
		case MediaFile::Type_Mov:
			movSrc.stop();
			break;
		case MediaFile::Type_Img:
			imgSrc.stop();
			break;
		case MediaFile::Type_App:
			appSrc.stop();
			break;
		}
	}
	
	switch (newType)	{
	case MediaFile::Type_None:		break;
	case MediaFile::Type_Cam:		camSrc.loadFile(n);		break;
	case MediaFile::Type_Mov:		movSrc.loadFile(n);		break;
	case MediaFile::Type_Img:		imgSrc.loadFile(n);		break;
	case MediaFile::Type_App:		appSrc.loadFile(n);		break;
	}
	
	srcFile = n;
}
bool DynamicVideoSource::playingBackItem(const MediaFile & n)	{
	return (camSrc.playingBackItem(n) || movSrc.playingBackItem(n) || imgSrc.playingBackItem(n) || appSrc.playingBackItem(n));
}


void DynamicVideoSource::stopSources()	{
	lock_guard<recursive_mutex>		tmpLock(srcLock);
	switch (srcType())	{
	case MediaFile::Type_None:
		break;
	case MediaFile::Type_Cam:
		camSrc.stop();
		break;
	case MediaFile::Type_Mov:
		movSrc.stop();
		break;
	case MediaFile::Type_Img:
		imgSrc.stop();
		break;
	case MediaFile::Type_App:
		appSrc.stop();
		break;
	}
	srcFile = MediaFile();
}




void DynamicVideoSource::staticSourceUpdated(VideoSource * inSrc)	{
	Q_UNUSED(inSrc);
	emit listOfStaticSourcesUpdated(this);
}
void DynamicVideoSource::aboutToQuit()	{
	stopSources();
}




DynamicVideoSource * GetDynamicVideoSource()	{
	return globalDynamicVideoSource;
}
