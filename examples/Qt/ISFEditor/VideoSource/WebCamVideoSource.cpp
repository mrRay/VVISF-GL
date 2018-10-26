#include "WebCamVideoSource.h"

#include <QDebug>




WebCamVideoSource::WebCamVideoSource(QObject *parent) : VideoSource(parent)	{
	//qDebug() << __PRETTY_FUNCTION__;
}
WebCamVideoSource::~WebCamVideoSource()	{
}




//VVGL::GLBufferRef WebCamVideoSource::getBuffer()	{
//	return nullptr;
//}
QList<MediaFile> WebCamVideoSource::createListOfStaticMediaFiles()	{
	//qDebug() << __PRETTY_FUNCTION__;
	QList<MediaFile>		returnMe;
	
	QList<QCameraInfo>		cameraInfos = QCameraInfo::availableCameras();
	for (const QCameraInfo & cameraInfo : cameraInfos)	{
		returnMe.append( MediaFile(cameraInfo) );
	}
	//qDebug() << "\t" << __PRETTY_FUNCTION__ << " - FINISHED";
	return returnMe;
}
void WebCamVideoSource::start()	{
	//qDebug() << __PRETTY_FUNCTION__;
	
	std::lock_guard<std::recursive_mutex> tmpLock(_lock);
	if (_running)
		return;
	
	_running=true;
	
	if (_cam == nullptr && _file.type()==MediaFile::Type_Cam)	{
		_cam = new QCamera(_file.cameraInfo(), this);
		
		connect(_cam, &QCamera::captureModeChanged, [&](QCamera::CaptureModes inMode)	{
			qDebug() << "QCamera::captureModeChanged ... " << inMode;
		});
		connect(_cam, QOverload<QCamera::Error>::of(&QCamera::error), [&](QCamera::Error inErr)	{
			qDebug() << "QCamera::error ... " << inErr;
		});
		connect(_cam, &QCamera::stateChanged, [&](QCamera::State inState)	{
			qDebug() << "QCamera::stateChanged ... " << inState;
		});
		connect(_cam, &QCamera::statusChanged, [&](QCamera::Status inStatus)	{
			qDebug() << "QCamera::statusChanged ... " << inStatus;
		});
		
	}
	
	if (_sfc == nullptr)	{
		_sfc = new GLBufferQVideoSurface(this);
		//connect(_sfc, SIGNAL(frameProduced()), this, SLOT());
		connect(_sfc, &GLBufferQVideoSurface::frameProduced, [&](GLBufferRef n)	{
			emit frameProduced(n);
		});
	}
	
	if (_cam==nullptr || _sfc==nullptr)	{
		qDebug() << "ERR: cam or sfc nil, " << __PRETTY_FUNCTION__;
		return;
	}
	
	_cam->setViewfinder(_sfc);
	
	_cam->start();
	
}
void WebCamVideoSource::stop()	{
	//qDebug() << __PRETTY_FUNCTION__;
	
	std::lock_guard<std::recursive_mutex> tmpLock(_lock);
	
	VideoSource::stop();
	
	if (_cam != nullptr)	{
		_cam->stop();
		_cam->unload();
		delete _cam;
		_cam = nullptr;
	}
	
	if (_sfc != nullptr)	{
		delete _sfc;
		_sfc = nullptr;
	}
	
}
bool WebCamVideoSource::playingBackItem(const MediaFile & n)	{
	if (n.type()==MediaFile::Type_Cam		&&
	_file.type()==MediaFile::Type_Cam		&&
	_file.cameraInfo()==n.cameraInfo())	{
		return true;
	}
	return false;
}
void WebCamVideoSource::loadFile(const MediaFile & n)	{
	//qDebug() << __PRETTY_FUNCTION__;
	
	if (n.type() != MediaFile::Type_Cam)
		return;
	
	std::lock_guard<std::recursive_mutex> tmpLock(_lock);
	if (_file == n)
		return;
	
	//	cams have to be deleted & recreated if we're changing their "file", so stop/update the file/start
	stop();
	_file=n;
	start();
}

