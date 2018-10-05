#ifndef JSONGUIINPUTPOINT2D_H
#define JSONGUIINPUTPOINT2D_H

#include <QWidget>

#include "JSONGUIInput.h"




namespace Ui {
	class JSONGUIInputPoint2D;
}




class JSONGUIInputPoint2DWidget : public QWidget, public JSONGUIInput
{
	Q_OBJECT

public:
	explicit JSONGUIInputPoint2DWidget(const JGMInputRef & inRef, QWidget *parent = nullptr);
	~JSONGUIInputPoint2DWidget();
	
	virtual void prepareUIItems() override;
	virtual void refreshUIItems() override;

private:
	Ui::JSONGUIInputPoint2D *ui;
};

#endif // JSONGUIINPUTPOINT2D_H
