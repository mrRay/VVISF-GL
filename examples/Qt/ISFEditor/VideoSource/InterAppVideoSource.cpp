#include "InterAppVideoSource.h"




InterAppVideoSource::InterAppVideoSource(QObject *parent) : VideoSource(parent)	{
}
InterAppVideoSource::~InterAppVideoSource()	{
}



//VVGL::GLBufferRef InterAppVideoSource::getBuffer()	{
//	return nullptr;
//}
void InterAppVideoSource::start()	{
	VideoSource::start();
}
void InterAppVideoSource::stop()	{
	VideoSource::stop();
}
bool InterAppVideoSource::playingBackItem(const MediaFile & n)	{
	return false;
}
void InterAppVideoSource::loadFile(const MediaFile & n)	{
}
