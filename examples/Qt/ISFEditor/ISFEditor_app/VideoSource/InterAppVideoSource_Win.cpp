#include "InterAppVideoSource_Win.h"

//#include <QtPlatformHeaders/QWindowsWindowFunctions>
//#include <QtPlatformHeaders/QXcbWindowFunctions>
//#import <QCocoaNativeContext>
//#include <QCocoaWindowFunctions>
//#include <QCocoaNativeContext>
//#include <QtPlatformHeaders/qcocoanativecontext.h>
//#include <QtPlatformHeaders/QCocoaNativeContext>
#include <QDebug>



#include "VVGL.hpp"




using namespace VVGL;








//	this c++ class stores objective-c objects- it exists so we can isolate the obj-c from the c++ header
class InterAppVideoSource_WinOpaque	{
public:
	/*	we need to use NSNotificationCenter to register for notifications that the list of syphon
	servers has been updated.  we also need to UN-register for these notifications, or we'll crash.  in
	order to UN-register, you need to retain an NSObject that was automatically created for you when you
	register for the notification.  which means we need to retain an array of NSObjects.				*/
	InterAppVideoSource_WinOpaque()	{
	}
	~InterAppVideoSource_WinOpaque()	{
	}

};








InterAppVideoSource_Win::InterAppVideoSource_Win(QObject *parent) : VideoSource(parent)	{
	//qDebug() << __PRETTY_FUNCTION__;
	//	make a new opaque object
	opaque = new InterAppVideoSource_WinOpaque();
}
InterAppVideoSource_Win::~InterAppVideoSource_Win()	{
	stop();
	delete opaque;
	opaque = nullptr;
}



//VVGL::GLBufferRef InterAppVideoSource_Win::getBuffer()	{
//	return nullptr;
//}
QList<MediaFile> InterAppVideoSource_Win::createListOfStaticMediaFiles()	{
	//qDebug() << __PRETTY_FUNCTION__;
	QList<MediaFile>		returnMe;
	return returnMe;
}
void InterAppVideoSource_Win::start()	{
	std::lock_guard<std::recursive_mutex> tmpLock(_lock);
	if (_running)
		return;

	if (_file.type() != MediaFile::Type_App)
		return;

	_running=true;
}
void InterAppVideoSource_Win::stop()	{
	//qDebug() << __PRETTY_FUNCTION__;
	VideoSource::stop();

}
bool InterAppVideoSource_Win::playingBackItem(const MediaFile & n)	{
	return (_file == n);
}
void InterAppVideoSource_Win::loadFile(const MediaFile & n)	{
	//if (n.type() != MediaFile::Type_App)	//	if it's not the right kind of file we'll still accept it- but we won't actually start
	//	return;

	std::lock_guard<std::recursive_mutex> tmpLock(_lock);
	if (_file == n)
		return;

	stop();
	_file = n;
	start();
}
