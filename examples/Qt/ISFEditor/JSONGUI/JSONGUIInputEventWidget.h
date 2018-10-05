#ifndef JSONGUIINPUTEVENT_H
#define JSONGUIINPUTEVENT_H

#include <QWidget>

#include "JSONGUIInput.h"




namespace Ui {
	class JSONGUIInputEvent;
}




class JSONGUIInputEventWidget : public QWidget, public JSONGUIInput
{
	Q_OBJECT

public:
	explicit JSONGUIInputEventWidget(const JGMInputRef & inRef, QWidget *parent = nullptr);
	~JSONGUIInputEventWidget();
	
	virtual void prepareUIItems() override;
	virtual void refreshUIItems() override;

private:
	Ui::JSONGUIInputEvent *ui;
};

#endif // JSONGUIINPUTEVENT_H
