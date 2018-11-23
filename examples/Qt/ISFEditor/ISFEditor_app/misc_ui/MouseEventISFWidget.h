#ifndef MOUSEEVENTISFWIDGET_H
#define MOUSEEVENTISFWIDGET_H

#include "ISFGLBufferQWidget.h"
#include "VVGL.hpp"




using namespace VVGL;




class MouseEventISFWidget : public ISFGLBufferQWidget
{
	Q_OBJECT
	
public:
	MouseEventISFWidget(QWidget * inParent=nullptr) : ISFGLBufferQWidget(inParent) {}
	
	VVGL::Point normClickLoc() { return _normClickLoc; }
	VVGL::Point absClickLoc() { return _absClickLoc; }
	
signals:
	Q_SIGNAL void mouseMoved(VVGL::Point normMouseEventLoc, VVGL::Point absMouseEventLoc);
	
protected:
	virtual void mousePressEvent(QMouseEvent * event) Q_DECL_OVERRIDE	{
		
		//	if there's presently no buffer, bail- nothing's being displayed/rendered...
		std::lock_guard<recursive_mutex>		lock(ctxLock);
		if (buffer == nullptr)
			return;
		//	get the size of the widget, and the size of the frame displayed in the widget
		QSize			tmpQSize = frameSize();
		Size			canvasSize(tmpQSize.width(), tmpQSize.height());
		Size			frameSize = buffer->srcRect.size;
		//	determine the rect which the video frame will occupy within the widget
		Rect			frameRect = ResizeRect(
			Rect(0,0,frameSize.width,frameSize.height),
			Rect(0,0,canvasSize.width,canvasSize.height),
			SizingMode_Fit);
		//	get the local coords of the event location
		Point			tmpPoint(event->x(), event->y());
		//	calculate the event's coords relative to the video frame rect, convert to normalized vals
		Point			framePoint(tmpPoint.x-frameRect.origin.x, tmpPoint.y-frameRect.origin.y);
		Point			normFramePoint(framePoint.x/frameRect.size.width, framePoint.y/frameRect.size.height);
		
		//	invert the 'y'
		normFramePoint.y = 1.0 - normFramePoint.y;
		framePoint.y = normFramePoint.y * frameRect.size.height;
		
		_normClickLoc = normFramePoint;
		_absClickLoc = Point(normFramePoint.x * frameSize.width, normFramePoint.y * frameSize.height);
		
		emit mouseMoved(_normClickLoc, _absClickLoc);
	}
	virtual void mouseMoveEvent(QMouseEvent * event) Q_DECL_OVERRIDE	{
		
		//	if there's presently no buffer, bail- nothing's being displayed/rendered...
		std::lock_guard<recursive_mutex>		lock(ctxLock);
		if (buffer == nullptr)
			return;
		//	get the size of the widget, and the size of the frame displayed in the widget
		QSize			tmpQSize = frameSize();
		Size			canvasSize(tmpQSize.width(), tmpQSize.height());
		Size			frameSize = buffer->srcRect.size;
		//	determine the rect which the video frame will occupy within the widget
		Rect			frameRect = ResizeRect(
			Rect(0,0,frameSize.width,frameSize.height),
			Rect(0,0,canvasSize.width,canvasSize.height),
			SizingMode_Fit);
		//	get the local coords of the event location
		Point			tmpPoint(event->x(), event->y());
		//	calculate the event's coords relative to the video frame rect, convert to normalized vals
		Point			framePoint(tmpPoint.x-frameRect.origin.x, tmpPoint.y-frameRect.origin.y);
		Point			normFramePoint(framePoint.x/frameRect.size.width, framePoint.y/frameRect.size.height);
		
		//	invert the 'y'
		normFramePoint.y = 1.0 - normFramePoint.y;
		framePoint.y = normFramePoint.y * frameRect.size.height;
		
		_normClickLoc = normFramePoint;
		_absClickLoc = Point(normFramePoint.x * frameSize.width, normFramePoint.y * frameSize.height);
		
		emit mouseMoved(_normClickLoc, _absClickLoc);
	}
	
private:
	VVGL::Point			_normClickLoc;
	VVGL::Point			_absClickLoc;
};




#endif // MOUSEEVENTISFWIDGET_H