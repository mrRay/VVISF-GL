#include "MovieVideoSource.h"




MovieVideoSource::MovieVideoSource(QObject *parent) : VideoSource(parent)	{
}
MovieVideoSource::~MovieVideoSource()	{
}



//VVGL::GLBufferRef MovieVideoSource::getBuffer()	{
//	return nullptr;
//}
void MovieVideoSource::start()	{
	VideoSource::start();
}
void MovieVideoSource::stop()	{
	VideoSource::stop();
}
bool MovieVideoSource::playingBackItem(const MediaFile & n)	{
	Q_UNUSED(n);
	return false;
}
void MovieVideoSource::loadFile(const MediaFile & n)	{
	Q_UNUSED(n);
}
