#include "JSONScrollEventFilter.h"

#include <QDragEnterEvent>




JSONScrollEventFilter::JSONScrollEventFilter(JSONScrollWidget * inParentScroll, QObject *parent) :
	QObject(parent),
	_parentScroll(inParentScroll)
{

}


bool JSONScrollEventFilter::eventFilter(QObject * watched, QEvent * event)	{
	//return true;	//	'false' means "don't filter"
	
	//qDebug() << __PRETTY_FUNCTION__ << ", " << event;
	//qDebug() << __PRETTY_FUNCTION__;
	
	switch (event->type())	{
	
	case QEvent::DragEnter:
		{
			JSONScrollWidget		*scrollWidget = _parentScroll.data();
			if (scrollWidget != nullptr)	{
				QDragEnterEvent			*dragEvent = static_cast<QDragEnterEvent*>(event);
				QPoint					dragLoc = dragEvent->pos();
				QWidget					*viewportWidget = qobject_cast<QWidget*>(watched);
				if (viewportWidget != nullptr)	{
					QSize					viewportSize = viewportWidget->frameSize();
					if (dragLoc.y() < 50)	{
						scrollWidget->startScrolling(Qt::TopEdge);
					}
					else if ((viewportSize.height()-dragLoc.y())<50)	{
						scrollWidget->startScrolling(Qt::BottomEdge);
					}
					else	{
						scrollWidget->stopScrolling();
					}
				}
				else	{
					scrollWidget->stopScrolling();
				}
			}
			else	{
				scrollWidget->stopScrolling();
			}
			
			//	accept the event so we get move events
			static_cast<QDragEnterEvent*>(event)->acceptProposedAction();
			return false;
		}
	case QEvent::DragMove:
		{
			JSONScrollWidget		*scrollWidget = _parentScroll.data();
			if (scrollWidget != nullptr)	{
				QDragMoveEvent			*dragEvent = static_cast<QDragMoveEvent*>(event);
				QPoint					dragLoc = dragEvent->pos();
				QWidget					*viewportWidget = qobject_cast<QWidget*>(watched);
				if (viewportWidget != nullptr)	{
					QSize					viewportSize = viewportWidget->frameSize();
					if (dragLoc.y() < 50)	{
						scrollWidget->startScrolling(Qt::TopEdge);
					}
					else if ((viewportSize.height()-dragLoc.y())<50)	{
						scrollWidget->startScrolling(Qt::BottomEdge);
					}
					else	{
						scrollWidget->stopScrolling();
					}
				}
				else	{
					scrollWidget->stopScrolling();
				}
			}
			else	{
				scrollWidget->stopScrolling();
			}
			
			static_cast<QDragEnterEvent*>(event)->accept();
			return false;
		}
	case QEvent::DragLeave:
	case QEvent::Drop:
		{
			JSONScrollWidget		*scrollWidget = _parentScroll.data();
			if (scrollWidget != nullptr)	{
				scrollWidget->stopScrolling();
			}
			
			static_cast<QDragEnterEvent*>(event)->accept();
			return false;
		}
	
	default:
		return false;
	
	}
	
	return false;
}

