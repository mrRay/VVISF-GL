#ifndef GLBUFFERQVIDEOSURFACE_H
#define GLBUFFERQVIDEOSURFACE_H

//#include <stdio.h>

#include <QAbstractVideoSurface>
#include <QVideoSurfaceFormat>
#include <QVideoFrame>
#include <QList>

#include "VVGL.hpp"




using namespace VVGL;




class GLBufferQVideoSurface : public QAbstractVideoSurface
{
	Q_OBJECT
	
public:
	GLBufferQVideoSurface(QObject * parent=nullptr) : QAbstractVideoSurface(parent), uploader(CreateGLCPUToTexCopierRef()) {}
	GLBufferQVideoSurface(const GLContextRef & inCtx, QObject * parent=nullptr) : QAbstractVideoSurface(parent), uploader(CreateGLCPUToTexCopierRefUsing(inCtx)) {}
	~GLBufferQVideoSurface() {
		uploader = nullptr;
		lastUploadedFrame = nullptr;
	}
	
	bool isFormatSupported(const QVideoSurfaceFormat & inFmt) const;
	/*
	QVideoSurfaceFormat nearestFormat(const QVideoSurfaceFormat & inFmt) const;
	*/
	bool present(const QVideoFrame & inFrame);
	//bool start(const QVideoSurfaceFormat & inFmt)	{ return true; }
	//void stop()	{ }
	QList<QVideoFrame::PixelFormat> supportedPixelFormats(QAbstractVideoBuffer::HandleType inType=QAbstractVideoBuffer::NoHandle) const;
	
	GLBufferRef getLatestFrame() const;
	
signals:
	Q_SIGNAL void frameProduced(GLBufferRef n);
	
private:
	GLCPUToTexCopierRef		uploader = nullptr;
	GLBufferRef				lastUploadedFrame = nullptr;
};




#endif // GLBUFFERQVIDEOSURFACE_H
