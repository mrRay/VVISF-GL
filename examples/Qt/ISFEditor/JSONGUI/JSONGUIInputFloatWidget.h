#ifndef JSONGUIINPUTFLOAT_H
#define JSONGUIINPUTFLOAT_H

#include "JSONGUIInputWidget.h"




namespace Ui {
	class JSONGUIInputFloat;
}




class JSONGUIInputFloatWidget : public JSONGUIInputWidget
{
	Q_OBJECT

public:
	explicit JSONGUIInputFloatWidget(const JGMInputRef & inRef, QWidget *parent = nullptr);
	~JSONGUIInputFloatWidget();
	
	virtual void prepareUIItems() override;
	virtual void refreshUIItems() override;

private:
	Ui::JSONGUIInputFloat *ui;
};

#endif // JSONGUIINPUTFLOAT_H
