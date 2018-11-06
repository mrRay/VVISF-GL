#include "ImgVideoSource.h"




ImgVideoSource::ImgVideoSource(QObject *parent) : VideoSource(parent)	{
}
ImgVideoSource::~ImgVideoSource()	{
}



//VVGL::GLBufferRef ImgVideoSource::getBuffer()	{
//	return nullptr;
//}
void ImgVideoSource::start()	{
	VideoSource::start();
}
void ImgVideoSource::stop()	{
	VideoSource::stop();
}
bool ImgVideoSource::playingBackItem(const MediaFile & n)	{
	Q_UNUSED(n);
	return false;
}
void ImgVideoSource::loadFile(const MediaFile & n)	{
	Q_UNUSED(n);
}
