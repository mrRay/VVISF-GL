#ifndef GLBUFFERQVIDEOSURFACE_H
#define GLBUFFERQVIDEOSURFACE_H

//#include <stdio.h>

#include <QAbstractVideoSurface>
#include <QVideoSurfaceFormat>
#include <QVideoFrame>
#include <QList>

#include "VVGL.hpp"
#include "VVISF.hpp"




using namespace VVGL;
using namespace VVISF;




class GLBufferQVideoSurface : public QAbstractVideoSurface
{
	Q_OBJECT
	
public:
	GLBufferQVideoSurface(QObject * parent=nullptr);
	GLBufferQVideoSurface(const GLContextRef & inCtx, QObject * parent=nullptr);
	~GLBufferQVideoSurface() {
		uploader = nullptr;
		swizzleScene = nullptr;
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
	ISFSceneRef				swizzleScene = nullptr;	//	GL4 doesn't support YCbCr textures (FFFFFFUUUUUUU...) so we use this to swizzle the uploaded YCbCr data (packed in a half-width RGBA texture) to a full-width RGBA image
	GLBufferRef				lastUploadedFrame = nullptr;
	bool					native2vuySupport = false;	//	set this to 'true' to upload the frames to YCbCr textures.  if this is 'false', the YCbCr CPU data is uploaded as a half-width RGBA texture, and the YCbCr->RGB conversion is done in a shader
	
	void generalInit();
};




#endif // GLBUFFERQVIDEOSURFACE_H
