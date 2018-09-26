#ifndef JSONGUIINPUTEVENT_H
#define JSONGUIINPUTEVENT_H

#include <QWidget>

#include "VVISF.hpp"




namespace Ui {
	class JSONGUIInputEvent;
}

using namespace VVISF;




class JSONGUIInputEventWidget : public QWidget
{
	Q_OBJECT

public:
	explicit JSONGUIInputEventWidget(const ISFAttrRef & inRef, QWidget *parent = nullptr);
	~JSONGUIInputEventWidget();

private:
	Ui::JSONGUIInputEvent *ui;
};

#endif // JSONGUIINPUTEVENT_H
