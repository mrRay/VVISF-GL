#ifndef JSONGUIPASSWIDGET_H
#define JSONGUIPASSWIDGET_H

#include <QWidget>

#include "JSONGUIPass.h"




namespace Ui {
	class JSONGUIPassWidget;
}




class JSONGUIPassWidget : public QWidget, public JSONGUIPass
{
	Q_OBJECT

public:
	explicit JSONGUIPassWidget(const JGMPassRef & inRef, QWidget *parent = nullptr);
	~JSONGUIPassWidget();
	
	virtual void prepareUIItems() override;
	virtual void refreshUIItems() override;

private:
	Ui::JSONGUIPassWidget *ui;
};




#endif // JSONGUIPASSWIDGET_H
