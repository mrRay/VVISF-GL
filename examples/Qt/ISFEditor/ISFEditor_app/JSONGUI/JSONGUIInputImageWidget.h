#ifndef JSONGUIINPUTIMAGE_H
#define JSONGUIINPUTIMAGE_H

#include "JSONGUIInputWidget.h"




namespace Ui {
	class JSONGUIInputImage;
}




class JSONGUIInputImageWidget : public JSONGUIInputWidget
{
	Q_OBJECT

public:
	explicit JSONGUIInputImageWidget(const JGMInputRef & inInput, JSONScrollWidget * inScrollWidget, QWidget *parent = nullptr);
	~JSONGUIInputImageWidget();
	
	virtual void prepareToBeDeleted() override;
	
	virtual void prepareUIItems() override;
	virtual void refreshUIItems() override;

private:
	Ui::JSONGUIInputImage *ui;
};

#endif // JSONGUIINPUTIMAGE_H
