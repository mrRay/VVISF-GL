#include "InterAppVideoSource.h"




InterAppVideoSource::InterAppVideoSource(QObject *parent) :
	src()
{
	Q_UNUSED(parent);
	QObject::connect(&src, &VideoSource::staticSourceUpdated, [&](VideoSource * n)	{
		emit staticSourceUpdated(n);
	});
	QObject::connect(&src, &VideoSource::frameProduced, [&](VVGL::GLBufferRef n)	{
		emit frameProduced(n);
	});
}
InterAppVideoSource::~InterAppVideoSource()	{
}




//VVGL::GLBufferRef InterAppVideoSource::getBuffer()	{
//	return nullptr;
//}
QList<MediaFile> InterAppVideoSource::createListOfStaticMediaFiles()	{
	return src.createListOfStaticMediaFiles();
}
void InterAppVideoSource::start()	{
	//qDebug() << __PRETTY_FUNCTION__;
	src.start();
}
void InterAppVideoSource::stop()	{
	//qDebug() << __PRETTY_FUNCTION__;
	src.stop();
}
bool InterAppVideoSource::playingBackItem(const MediaFile & n)	{
	return src.playingBackItem(n);
}
void InterAppVideoSource::loadFile(const MediaFile & n)	{
	//qDebug() << __PRETTY_FUNCTION__;
	src.loadFile(n);
}



bool InterAppVideoSource::running()	{
	return src.running();
}
void InterAppVideoSource::renderABuffer()	{
	src.renderABuffer();
}
