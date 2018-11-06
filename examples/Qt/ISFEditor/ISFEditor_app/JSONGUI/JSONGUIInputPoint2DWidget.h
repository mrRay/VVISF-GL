#ifndef JSONGUIINPUTPOINT2D_H
#define JSONGUIINPUTPOINT2D_H

#include "JSONGUIInputWidget.h"




namespace Ui {
	class JSONGUIInputPoint2D;
}




class JSONGUIInputPoint2DWidget : public JSONGUIInputWidget
{
	Q_OBJECT

public:
	explicit JSONGUIInputPoint2DWidget(const JGMInputRef & inInput, JSONScrollWidget * inScrollWidget, QWidget *parent = nullptr);
	~JSONGUIInputPoint2DWidget();
	
	virtual void prepareUIItems() override;
	virtual void refreshUIItems() override;

private:
	Ui::JSONGUIInputPoint2D *ui;
};

#endif // JSONGUIINPUTPOINT2D_H
