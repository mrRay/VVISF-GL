#ifndef JSONGUIINPUTIMAGE_H
#define JSONGUIINPUTIMAGE_H

#include <QWidget>

#include "JSONGUIInput.h"




namespace Ui {
	class JSONGUIInputImage;
}




class JSONGUIInputImageWidget : public QWidget, public JSONGUIInput
{
	Q_OBJECT

public:
	explicit JSONGUIInputImageWidget(const JGMInputRef & inRef, QWidget *parent = nullptr);
	~JSONGUIInputImageWidget();
	
	virtual void prepareUIItems() override;
	virtual void refreshUIItems() override;

private:
	Ui::JSONGUIInputImage *ui;
};

#endif // JSONGUIINPUTIMAGE_H
