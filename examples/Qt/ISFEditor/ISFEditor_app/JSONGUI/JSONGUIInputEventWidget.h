#ifndef JSONGUIINPUTEVENT_H
#define JSONGUIINPUTEVENT_H

#include "JSONGUIInputWidget.h"




namespace Ui {
	class JSONGUIInputEvent;
}




class JSONGUIInputEventWidget : public JSONGUIInputWidget
{
	Q_OBJECT

public:
	explicit JSONGUIInputEventWidget(const JGMInputRef & inInput, JSONScrollWidget * inScrollWidget, QWidget *parent = nullptr);
	~JSONGUIInputEventWidget();
	
	virtual void prepareToBeDeleted() override;
	
	virtual void prepareUIItems() override;
	virtual void refreshUIItems() override;

private:
	Ui::JSONGUIInputEvent *ui;
};

#endif // JSONGUIINPUTEVENT_H
