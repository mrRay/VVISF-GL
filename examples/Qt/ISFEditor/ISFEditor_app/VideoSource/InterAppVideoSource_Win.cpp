#include "InterAppVideoSource_Win.h"

//#include <QtPlatformHeaders/QWindowsWindowFunctions>
//#include <QtPlatformHeaders/QXcbWindowFunctions>
//#import <QCocoaNativeContext>
//#include <QCocoaWindowFunctions>
//#include <QCocoaNativeContext>
//#include <QtPlatformHeaders/qcocoanativecontext.h>
//#include <QtPlatformHeaders/QCocoaNativeContext>
#include <QDebug>
#include <QTimer>

#include "VVGL.hpp"
#include "Spout.h"
#include "SpoutSourcesWatcher.h"
#include "ISFController.h"




using namespace VVGL;








//	this class exists to ensure that none of the spout headers will interfere with the use of GLEW in any of VVGL/VVISF's headers
class InterAppVideoSource_WinOpaque	{
public:
	InterAppVideoSource_WinOpaque(MediaFile * inTargetFile=nullptr)	{
		memset(_name, 0, 256);
		if (inTargetFile != nullptr)	{
			QString			inTargetName = inTargetFile->name();
			char			tmpChars[256];
			unsigned int	tmpWidth = 1;
			unsigned int	tmpHeight = 1;
			memset(tmpChars, 0, 256);
			//strcpy(tmpChars, _name.toStdString().c_str());
			if (!inTargetName.isNull())	{
				_nameLength = inTargetName.length();
				memcpy(_name, inTargetName.toStdString().c_str(), _nameLength);
				memcpy(tmpChars, _name, _nameLength);
			}

			rxr.CreateReceiver(tmpChars, tmpWidth, tmpHeight, true);

			_file = MediaFile(MediaFile::Type_App, QString(tmpChars), QString(tmpChars));
		}
	}
	~InterAppVideoSource_WinOpaque()	{
		rxr.ReleaseReceiver();
	}
	MediaFile file() const { return _file; }

	char				_name[256];	//	the name of the spout server
	int					_nameLength = 0;	//	the length of '_name'
	MediaFile			_file = MediaFile(MediaFile::Type_App, QString(), QString());	//	a MediaFile that describes this video source
	SpoutReceiver		rxr;
};








InterAppVideoSource_Win::InterAppVideoSource_Win(QObject *parent) : VideoSource(parent)	{
	//qDebug() << __PRETTY_FUNCTION__;
	//	make a new opaque object
	opaque = new InterAppVideoSource_WinOpaque();
	//	we want to emit a signal every time the list of spout sources has changed
	SpoutSourcesWatcher		*watcher = SpoutSourcesWatcher::GetGlobalWatcher();
	QObject::connect(watcher, &SpoutSourcesWatcher::spoutSourcesUpdated, [&]()	{
		//qDebug() << "************* list of sources changed!";
		emit staticSourceUpdated(this);
	});
}
InterAppVideoSource_Win::~InterAppVideoSource_Win()	{
	stop();
	if (opaque != nullptr)	{
		delete opaque;
		opaque = nullptr;
	}
}



//VVGL::GLBufferRef InterAppVideoSource_Win::getBuffer()	{
//	return nullptr;
//}
QList<MediaFile> InterAppVideoSource_Win::createListOfStaticMediaFiles()	{
	//qDebug() << __PRETTY_FUNCTION__;
	QList<MediaFile>		returnMe;
	QStringList		tmpFileNames = SpoutSourcesWatcher::GetListOfSources();
	for (const QString & tmpFileName : tmpFileNames)	{
		returnMe.append( MediaFile(MediaFile::Type_App, tmpFileName, tmpFileName) );
	}
	return returnMe;
	/*
	QList<MediaFile>		returnMe;

	bool					createdTmpOpaque = false;
	if (opaque == nullptr)	{
		opaque = new InterAppVideoSource_WinOpaque();
		createdTmpOpaque = true;
	}

	if (opaque != nullptr)	{
		int			senderCount = opaque->rxr.GetSenderCount();
		char		tmpChars[256];
		for (int i=0; i<senderCount; ++ i)	{
			memset(tmpChars, 0, 256);
			if (opaque->rxr.GetSenderName(i, tmpChars))	{
				QString		tmpString(tmpChars);
				//qDebug() << "\tshould be adding file named " << tmpString;
				returnMe.append( MediaFile(MediaFile::Type_App, tmpString, tmpString) );
			}
			else
				qDebug() << "\tERR: problem getting sender name in " << __PRETTY_FUNCTION__;
		}
	}
	else
		qDebug() << "\tERR: opaque NULL in " << __PRETTY_FUNCTION__;

	if (createdTmpOpaque && opaque!=nullptr)	{
		delete opaque;
		opaque = nullptr;
	}

	return returnMe;
	*/
}
void InterAppVideoSource_Win::start()	{
	std::lock_guard<std::recursive_mutex> tmpLock(_lock);
	if (_running)
		return;

	if (_file.type() != MediaFile::Type_App)
		return;

	_running=true;

	if (opaque != nullptr)	{
		delete opaque;
		opaque = nullptr;
	}
	opaque = new InterAppVideoSource_WinOpaque(&_file);
	_file = opaque->file();
}
void InterAppVideoSource_Win::stop()	{
	//qDebug() << __PRETTY_FUNCTION__;
	VideoSource::stop();
	if (opaque != nullptr)	{
		delete opaque;
		opaque = nullptr;
	}
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




void InterAppVideoSource_Win::renderABuffer()	{
	//qDebug() << __PRETTY_FUNCTION__;

	if (_file.type() != MediaFile::Type_App)
		return;

	std::lock_guard<std::recursive_mutex> tmpLock(_lock);

	if (opaque == nullptr)
		return;

	//	get the info about the receiver's dimensions- which may have changed
	unsigned int	tmpWidth = 1;
	unsigned int	tmpHeight = 1;
	HANDLE			tmpHandle;
	DWORD			tmpFormat;
	opaque->rxr.GetSenderInfo(opaque->_name, tmpWidth, tmpHeight, tmpHandle, tmpFormat);
	//	make a new buffer
	ISFController	*isfc = GetISFController();
	GLBufferPoolRef		bp = isfc->renderThreadBufferPool();
	GLBufferRef		newBuffer = CreateBGRATex(VVGL::Size(tmpWidth,tmpHeight), false, bp);
	newBuffer->flipped = true;
	//	receive into the new buffer
	if ( !opaque->rxr.ReceiveTexture(opaque->_name, tmpWidth, tmpHeight, newBuffer->name, newBuffer->desc.target, false, 0) )
		qDebug() << "ERR: couldnt receive texture from spout";
	else	{
		//cout << "should have uploaded spout from " << opaque->_name <<  " to " << *newBuffer << endl;
		//	update the properties of the buffer to reflect the size of the texture copied from spout
		newBuffer->size = VVGL::Size(tmpWidth, tmpHeight);
		newBuffer->srcRect.size = newBuffer->size;
		//	emit a signal with the new buffer
		emit frameProduced(newBuffer);
	}

}
