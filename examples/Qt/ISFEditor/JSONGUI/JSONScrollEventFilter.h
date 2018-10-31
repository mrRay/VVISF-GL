#ifndef JSONSCROLLEVENTFILTER_H
#define JSONSCROLLEVENTFILTER_H

#include <QObject>
#include <QPointer>

#include "JSONScrollWidget.h"
class JSONScrollWidget;




/*		this event filter is owned by the JSON scroll view widget- it monitors drag events picked up 
by the viewport, when drags are occurring outside of the input/pass widgets, and converts those drag 
events into scroll messages sent back to the scroll view widget (which is why it has a weak ptr to 
the scroll widget that created it).	*/




class JSONScrollEventFilter : public QObject
{
	Q_OBJECT
public:
	explicit JSONScrollEventFilter(JSONScrollWidget * inParentScroll, QObject *parent = nullptr);
	
	virtual bool eventFilter(QObject * watched, QEvent * event) override;
	
private:
	QPointer<JSONScrollWidget>		_parentScroll = nullptr;
};




#endif // JSONSCROLLEVENTFILTER_H