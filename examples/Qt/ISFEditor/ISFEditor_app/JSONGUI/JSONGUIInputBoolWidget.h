#ifndef JSONGUIINPUTBOOL_H
#define JSONGUIINPUTBOOL_H

#include "JSONGUIInputWidget.h"




namespace Ui {
	class JSONGUIInputBool;
}




class JSONGUIInputBoolWidget : public JSONGUIInputWidget
{
	Q_OBJECT

public:
	explicit JSONGUIInputBoolWidget(const JGMInputRef & inInput, JSONScrollWidget * inScrollWidget, QWidget *parent = nullptr);
	~JSONGUIInputBoolWidget();
	
	virtual void prepareToBeDeleted() override;
	
	virtual void prepareUIItems() override;
	virtual void refreshUIItems() override;

private:
	Ui::JSONGUIInputBool *ui;
};

#endif // JSONGUIINPUTBOOL_H
