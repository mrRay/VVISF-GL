#include "WebCamVideoSource.h"

#include <QDebug>

#include "ISFController.h"




WebCamVideoSource::WebCamVideoSource(QObject *parent) : VideoSource(parent)	{
	//qDebug() << __PRETTY_FUNCTION__;
	ISFController	*isfc = GetISFController();
	QThread			*renderThread = isfc->renderThread();
	GLBufferPoolRef			isfPool = isfc->renderThreadBufferPool();
	GLTexToTexCopierRef		isfCopier = isfc->renderThreadTexCopier();
	
	if (_sfc == nullptr)	{
		//GLContextRef		sceneCtx = (scene==nullptr) ? nullptr : scene->context();
		GLContextRef		tmpCtx = CreateNewGLContextRef();
		//tmpCtx->moveToThread(isfRenderThread);
	
		//GLBufferPoolRef			isfPool = isfc->renderThreadBufferPool();
		//scene->setPrivatePool(isfPool);
		//GLTexToTexCopierRef		isfCopier = isfc->renderThreadTexCopier();
		//scene->setPrivateCopier(isfCopier);
		
		//_sfc = new GLBufferQVideoSurface(this);
		//_sfc = new GLBufferQVideoSurface(tmpCtx, this);
		_sfc = new GLBufferQVideoSurface(tmpCtx);
		_sfc->moveToThread(renderThread, isfPool, isfCopier);
		//connect(_sfc, SIGNAL(frameProduced()), this, SLOT());
		
		
		connect(_sfc, &GLBufferQVideoSurface::frameProduced, [&](GLBufferRef n)	{
			//qDebug() << "GLBufferQVideoSurface produced a frame...";
			emit frameProduced(n);
		});
	}
}
WebCamVideoSource::~WebCamVideoSource()	{
	if (_sfc != nullptr)	{
		delete _sfc;
		_sfc = nullptr;
	}
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
	
	return returnMe;
}
void WebCamVideoSource::start()	{
	//qDebug() << __PRETTY_FUNCTION__;
	
	std::lock_guard<std::recursive_mutex> tmpLock(_lock);
	if (_running)
		return;
	
	_running=true;
	
	ISFController		*isfc = GetISFController();
	VVGLRenderQThread	*isfRenderThread = (isfc==nullptr) ? nullptr : isfc->renderThread();
	
	if (_cam == nullptr && _file.type()==MediaFile::Type_Cam)	{
		//_cam = new QCamera(_file.cameraInfo(), this);
		_cam = new QCamera(_file.cameraInfo());
		//if (isfRenderThread != nullptr)
		//	_cam->moveToThread(isfRenderThread);
		
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
	/*
	if (_sfc == nullptr)	{
		//GLContextRef		sceneCtx = (scene==nullptr) ? nullptr : scene->context();
		GLContextRef		tmpCtx = CreateNewGLContextRef();
		//tmpCtx->moveToThread(isfRenderThread);
	
		GLBufferPoolRef			isfPool = isfc->renderThreadBufferPool();
		//scene->setPrivatePool(isfPool);
		GLTexToTexCopierRef		isfCopier = isfc->renderThreadTexCopier();
		//scene->setPrivateCopier(isfCopier);
		
		//_sfc = new GLBufferQVideoSurface(this);
		//_sfc = new GLBufferQVideoSurface(tmpCtx, this);
		_sfc = new GLBufferQVideoSurface(tmpCtx);
		_sfc->moveToThread(isfRenderThread, isfPool, isfCopier);
		//connect(_sfc, SIGNAL(frameProduced()), this, SLOT());
		
		
		connect(_sfc, &GLBufferQVideoSurface::frameProduced, [&](GLBufferRef n)	{
			//qDebug() << "GLBufferQVideoSurface produced a frame...";
			emit frameProduced(n);
		});
	}
	*/
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
	/*
	if (_sfc != nullptr)	{
		delete _sfc;
		_sfc = nullptr;
	}
	*/
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

