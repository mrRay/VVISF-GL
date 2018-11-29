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
		yuvSwizzleScene = nullptr;
		bgrSwizzleScene = nullptr;
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
	GLContextRef			ctxToUse = nullptr;	//	null by default.  if null, scenes can use their own contexts.  if non-null, scenes must use this context to do their rendering.
	GLCPUToTexCopierRef		uploader = nullptr;
	ISFSceneRef				yuvSwizzleScene = nullptr;	//	GL4 doesn't support YCbCr textures (FFFFFFUUUUUUU...) so we use this to swizzle the uploaded YCbCr data (packed in a half-width RGBA texture) to a full-width RGBA image
	ISFSceneRef				bgrSwizzleScene = nullptr;	//	Qt vends "RGB32" as "xRGB", which we upload as BGRA and then swizzle to RGBA using this scene
	GLBufferRef				lastUploadedFrame = nullptr;
	bool					native2vuySupport = false;	//	set this to 'true' to upload the frames to YCbCr textures.  if this is 'false', the YCbCr CPU data is uploaded as a half-width RGBA texture, and the YCbCr->RGB conversion is done in a shader
	
	void generalInit();
};




#endif // GLBUFFERQVIDEOSURFACE_H
