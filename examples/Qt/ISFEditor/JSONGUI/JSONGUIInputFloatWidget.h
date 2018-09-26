#ifndef JSONGUIINPUTFLOAT_H
#define JSONGUIINPUTFLOAT_H

#include <QWidget>

#include "VVISF.hpp"




namespace Ui {
	class JSONGUIInputFloat;
}

using namespace VVISF;




class JSONGUIInputFloatWidget : public QWidget
{
	Q_OBJECT

public:
	explicit JSONGUIInputFloatWidget(const ISFAttrRef & inRef, QWidget *parent = nullptr);
	~JSONGUIInputFloatWidget();

private:
	Ui::JSONGUIInputFloat *ui;
};

#endif // JSONGUIINPUTFLOAT_H
