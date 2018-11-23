#include "GLBufferQVideoSurface.h"

#include <QDebug>




GLBufferQVideoSurface::GLBufferQVideoSurface(QObject * parent) :
	QAbstractVideoSurface(parent),
	uploader(CreateGLCPUToTexCopierRef()),
	swizzleScene(CreateISFSceneRef())
{
	generalInit();
}
GLBufferQVideoSurface::GLBufferQVideoSurface(const GLContextRef & inCtx, QObject * parent) :
	QAbstractVideoSurface(parent),
	uploader(CreateGLCPUToTexCopierRefUsing(inCtx)),
	swizzleScene(CreateISFSceneRefUsing(inCtx))
{
	generalInit();
}
void GLBufferQVideoSurface::generalInit()	{
	if (uploader!=nullptr)
		uploader->setQueueSize(1);
	if (swizzleScene != nullptr)	{
		QString			tmpString(":/resources/SwizzleISF-CbY0CrY1toRGB.fs");
		QFile			tmpFile(tmpString);
		if (!tmpFile.open(QFile::ReadOnly | QFile::Text))	{
			qDebug() << "\tERR: could not open SwizzleISF-CbY0CrY1toRGB.fs";
		}
		else	{
			QTextStream		tmpStream(&tmpFile);
			QString			fileContentsAsString = tmpStream.readAll();
			std::string		fileContents = fileContentsAsString.toStdString();
			tmpFile.close();
			
			ISFDocRef		tmpDoc = make_shared<ISFDoc>(fileContents, ISFVertPassthru_GL2, nullptr);
			swizzleScene->useDoc(tmpDoc);
		}
	}
}


bool GLBufferQVideoSurface::isFormatSupported(const QVideoSurfaceFormat & inFmt) const	{
	qDebug() << __PRETTY_FUNCTION__ << "... " << inFmt;
	
	//	check the handle type
	switch (inFmt.handleType())	{
	case QAbstractVideoBuffer::GLTextureHandle:
	case QAbstractVideoBuffer::XvShmImageHandle:
	case QAbstractVideoBuffer::CoreImageHandle:
	case QAbstractVideoBuffer::EGLImageHandle:
	case QAbstractVideoBuffer::UserHandle:
		//	not okay- bail and return false, we can't work with these
		cout << "ERR: " << __PRETTY_FUNCTION__ << ", bailing, incompatible handle type (" << inFmt.handleType() << ")\n";
		return false;
		break;
	case QAbstractVideoBuffer::NoHandle:
	case QAbstractVideoBuffer::QPixmapHandle:
		//	acceptable- continue to check the surface format's pixel format...
		break;
	}
	
	//	check the pixel format
	switch (inFmt.pixelFormat())	{
	case QVideoFrame::Format_Invalid:
		return false;
		break;
	case QVideoFrame::Format_BGRA32:
	case QVideoFrame::Format_BGR32:
	case QVideoFrame::Format_RGB32:
	case QVideoFrame::Format_UYVY:
		return true;
		break;
	
	case QVideoFrame::Format_ARGB32:
	case QVideoFrame::Format_BGRA32_Premultiplied:
	case QVideoFrame::Format_ARGB32_Premultiplied:
	case QVideoFrame::Format_YUYV:
	case QVideoFrame::Format_RGB24:
	case QVideoFrame::Format_BGR24:
	case QVideoFrame::Format_RGB565:
	case QVideoFrame::Format_RGB555:
	case QVideoFrame::Format_ARGB8565_Premultiplied:
	case QVideoFrame::Format_BGR565:
	case QVideoFrame::Format_BGR555:
	case QVideoFrame::Format_BGRA5658_Premultiplied:
	case QVideoFrame::Format_AYUV444:
	case QVideoFrame::Format_AYUV444_Premultiplied:
	case QVideoFrame::Format_YUV444:
	case QVideoFrame::Format_YUV420P:
	case QVideoFrame::Format_YV12:
	case QVideoFrame::Format_NV12:
	case QVideoFrame::Format_NV21:
	case QVideoFrame::Format_IMC1:
	case QVideoFrame::Format_IMC2:
	case QVideoFrame::Format_IMC3:
	case QVideoFrame::Format_IMC4:
	case QVideoFrame::Format_Y8:
	case QVideoFrame::Format_Y16:
	case QVideoFrame::Format_Jpeg:
	case QVideoFrame::Format_CameraRaw:
	case QVideoFrame::Format_AdobeDng:
	case QVideoFrame::NPixelFormats:
	case QVideoFrame::Format_User:
		//	we can't work with these pixel formats- unmap, delete, and bail
		cout << "\tERR: unsupported pixel format (" << inFmt.pixelFormat() << ") in " << __PRETTY_FUNCTION__ << "\n";
		return false;
		break;
	}
	
	return false;
	
}
/*
QVideoSurfaceFormat GLBufferQVideoSurface::nearestFormat(const QVideoSurfaceFormat & inFmt) const	{
	return inFmt;
}
*/
bool GLBufferQVideoSurface::present(const QVideoFrame & inFrame)	{
	//qDebug() << __PRETTY_FUNCTION__ << "... " << inFrame << endl;
	
	if (uploader == nullptr)	{
		cout << "ERR: no uploader, " << __PRETTY_FUNCTION__ << endl;
		return false;
	}
	auto			bp = GetGlobalBufferPool();
	if (bp == nullptr)	{
		cout << "ERR: no global buffer pool, " << __PRETTY_FUNCTION__ << endl;
		return false;
	}
	GLBufferRef		cpuBuffer = CreateCPUBufferForQVideoFrame(const_cast<QVideoFrame*>(&inFrame), bp);
	if (cpuBuffer == nullptr)	{
		cout << "ERR: cant make CPU buffer, " << __PRETTY_FUNCTION__ << endl;
		return false;
	}
	//cout << "\tcpuBuffer is " << *cpuBuffer << ", pixel format is " << cpuBuffer->desc.pixelFormat << endl;
	//cout << "\tPF_BGRA is " << GLBuffer::PF_BGRA << ", PF_RGBA is " << GLBuffer::PF_RGBA << ", PF_YCbCr_422 is " << GLBuffer::PF_YCbCr_422 << endl;
	
	//	BGRA/RGBA images from Qt are little-endian, so we need to swap bytes if that's what we're working with
	bool			textureNeedsToBeSwizzled = false;
	switch(cpuBuffer->desc.pixelFormat)	{
	case GLBuffer::PF_YCbCr_422:
		uploader->setSwapBytes(false);
		//	if we're uploading the YCbCr tex as RGBA and then swizzling it on the GPU, adjust the cpu buffer accordingly
		if (!native2vuySupport)	{
			textureNeedsToBeSwizzled = true;
			cpuBuffer->size = { ceil(cpuBuffer->size.width/2.), cpuBuffer->size.height };
			cpuBuffer->backingSize = { ceil(cpuBuffer->size.width/2.), cpuBuffer->size.height };
			cpuBuffer->srcRect = { 0, 0, ceil(cpuBuffer->srcRect.size.width/2.), cpuBuffer->size.height };
			cpuBuffer->desc.internalFormat = GLBuffer::IF_RGBA8;
			cpuBuffer->desc.pixelFormat = GLBuffer::PF_BGRA;
			cpuBuffer->desc.pixelType = GLBuffer::PT_UByte;
		}
		break;
	default:
		uploader->setSwapBytes(true);
		break;
	}
	
	GLBufferRef		freshTexture = uploader->streamCPUToTex(cpuBuffer,false);
	GLBufferRef		texToEmit = nullptr;
	if (freshTexture != nullptr)	{
		if (textureNeedsToBeSwizzled)	{
			swizzleScene->setFilterInputBuffer(freshTexture);
			//swizzleScene->setSize(Size(freshTexture->size.width*2.0, freshTexture->size.height));
			Size			tmpSize(freshTexture->size.width*2.0, freshTexture->size.height);
			GLBufferRef		swizzledTexture = swizzleScene->createAndRenderABuffer(tmpSize);
			if (swizzledTexture != nullptr)
				lastUploadedFrame = swizzledTexture;
		}
		else	{
			lastUploadedFrame = freshTexture;
		}
	}
	
	if (freshTexture == nullptr)	{
		//cout << "\tsuccessfully uploaded frame, nothing retrieved\n";
	}
	else	{
		//cout << "\tsuccessfully retrieved uploaded frame " << *lastUploadedFrame << endl;
		emit frameProduced(lastUploadedFrame);
	}
	
	return true;
}
//bool start(const QVideoSurfaceFormat & inFmt)	{ return true; }
//void stop()	{ }
QList<QVideoFrame::PixelFormat> GLBufferQVideoSurface::supportedPixelFormats(QAbstractVideoBuffer::HandleType inType) const	{
	Q_UNUSED(inType);
	QList<QVideoFrame::PixelFormat>		returnMe;
	returnMe.append(QVideoFrame::Format_UYVY);
	returnMe.append(QVideoFrame::Format_BGRA32);
	returnMe.append(QVideoFrame::Format_BGR32);
	returnMe.append(QVideoFrame::Format_RGB32);
	
	//returnMe.append(QVideoFrame::Format_ARGB32);
	//returnMe.append(QVideoFrame::Format_RGB24);
	//returnMe.append(QVideoFrame::Format_YUYV);
	//returnMe.append(QVideoFrame::Format_RGB32);
	//returnMe.append(QVideoFrame::Format_BGRA32_Premultiplied);
	//returnMe.append(QVideoFrame::Format_ARGB32_Premultiplied);
	return returnMe;
}

GLBufferRef GLBufferQVideoSurface::getLatestFrame() const	{
	return lastUploadedFrame;
}
