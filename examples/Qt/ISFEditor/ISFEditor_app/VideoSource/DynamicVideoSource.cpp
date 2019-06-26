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
	connect(&isfSrc, &VideoSource::staticSourceUpdated, this, &DynamicVideoSource::staticSourceUpdated);
	connect(&appSrc, &VideoSource::staticSourceUpdated, this, &DynamicVideoSource::staticSourceUpdated);
	//	video sources emit a signal when they produce new frames...
	connect(&camSrc, &VideoSource::frameProduced, [&](GLBufferRef n) { { lock_guard<recursive_mutex> tmpLock(lastBufferLock); lastBuffer=n; } emit frameProduced(n); });
	connect(&movSrc, &VideoSource::frameProduced, [&](GLBufferRef n) { { lock_guard<recursive_mutex> tmpLock(lastBufferLock); lastBuffer=n; } emit frameProduced(n); });
	connect(&imgSrc, &VideoSource::frameProduced, [&](GLBufferRef n) { { lock_guard<recursive_mutex> tmpLock(lastBufferLock); lastBuffer=n; } emit frameProduced(n); });
	connect(&isfSrc, &VideoSource::frameProduced, [&](GLBufferRef n) { { lock_guard<recursive_mutex> tmpLock(lastBufferLock); lastBuffer=n; } emit frameProduced(n); });
	connect(&appSrc, &VideoSource::frameProduced, [&](GLBufferRef n) { { lock_guard<recursive_mutex> tmpLock(lastBufferLock); lastBuffer=n; } emit frameProduced(n); });
	
	//	when the app is about to quit we want to stop the sources
	connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(aboutToQuit()));
}
DynamicVideoSource::~DynamicVideoSource()	{
	//stopSources();
}


GLBufferRef DynamicVideoSource::getBuffer()	{
	
	lock_guard<recursive_mutex>		tmpLock(lastBufferLock);
	
	
	
	
	//	if there's a file to load, do so
	if (!fileToLoad.isNull())	{
		MediaFile			tmpFile = *fileToLoad.data();
		MediaFile::Type		origType = srcFile.type();
		MediaFile::Type		newType = tmpFile.type();
	
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
			case MediaFile::Type_ISF:
				isfSrc.stop();
				break;
			case MediaFile::Type_App:
				appSrc.stop();
				break;
			}
		}
	
		switch (newType)	{
		case MediaFile::Type_None:		break;
		case MediaFile::Type_Cam:		camSrc.loadFile(tmpFile);		break;
		case MediaFile::Type_Mov:		movSrc.loadFile(tmpFile);		break;
		case MediaFile::Type_Img:		imgSrc.loadFile(tmpFile);		break;
		case MediaFile::Type_ISF:		isfSrc.loadFile(tmpFile);		break;
		case MediaFile::Type_App:		appSrc.loadFile(tmpFile);		break;
		}
	
		//srcFile = n;
		srcFile = tmpFile;
		
		fileToLoad = nullptr;
	}
	
	
	
	
	switch (srcFile.type())	{
	case MediaFile::Type_None:
		return nullptr;
	case MediaFile::Type_Cam:
		return lastBuffer;
#if defined(Q_OS_MAC)
	case MediaFile::Type_App:
		return lastBuffer;
#endif
	
	//	these source types have to be told to render a buffer!
#if defined(Q_OS_WIN)
	case MediaFile::Type_App:
		appSrc.renderABuffer();
		return lastBuffer;
#endif
	case MediaFile::Type_Mov:
		movSrc.renderABuffer();
		return lastBuffer;
	case MediaFile::Type_Img:
		imgSrc.renderABuffer();
		return lastBuffer;
	case MediaFile::Type_ISF:
		isfSrc.renderABuffer();
		return lastBuffer;
	}
	
	return nullptr;
}
QList<MediaFile> DynamicVideoSource::createListOfStaticMediaFiles()	{
	//qDebug() << __PRETTY_FUNCTION__;
	
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
	
	tmpList = isfSrc.createListOfStaticMediaFiles();
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
	//qDebug() << __PRETTY_FUNCTION__ << "... " << n.name();
	
	lock_guard<recursive_mutex>		tmpLock(srcLock);
	
	fileToLoad = QSharedPointer<MediaFile>(new MediaFile(n));
	
}
bool DynamicVideoSource::playingBackItem(const MediaFile & n)	{
	//qDebug() << __PRETTY_FUNCTION__ << ", " << n;
	if (n.type() == MediaFile::Type_None)
		return false;
	//qDebug() << __PRETTY_FUNCTION__ << ", " << camSrc.playingBackItem(n) << ", " << movSrc.playingBackItem(n) << ", " << imgSrc.playingBackItem(n) << ", " << isfSrc.playingBackItem(n) << ", " << appSrc.playingBackItem(n);
	return (camSrc.playingBackItem(n) || movSrc.playingBackItem(n) || imgSrc.playingBackItem(n) || isfSrc.playingBackItem(n) || appSrc.playingBackItem(n));
}
void DynamicVideoSource::setRenderSize(const VVGL::Size & n)	{
	isfSrc.setRenderSize(n);
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
	case MediaFile::Type_ISF:
		isfSrc.stop();
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
