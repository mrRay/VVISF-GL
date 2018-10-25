#ifndef JSONGUIINPUTCOLOR_H
#define JSONGUIINPUTCOLOR_H

#include "JSONGUIInputWidget.h"




namespace Ui {
	class JSONGUIInputColor;
}




class JSONGUIInputColorWidget : public JSONGUIInputWidget
{
	Q_OBJECT

public:
	explicit JSONGUIInputColorWidget(const JGMInputRef & inRef, QWidget *parent = nullptr);
	~JSONGUIInputColorWidget();
	
	virtual void prepareUIItems() override;
	virtual void refreshUIItems() override;

private:
	Ui::JSONGUIInputColor *ui;
};

#endif // JSONGUIINPUTCOLOR_H
