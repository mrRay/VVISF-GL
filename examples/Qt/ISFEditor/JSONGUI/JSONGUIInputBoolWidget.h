#ifndef JSONGUIINPUTBOOL_H
#define JSONGUIINPUTBOOL_H

#include <QWidget>

#include "VVISF.hpp"




namespace Ui {
	class JSONGUIInputBool;
}

using namespace VVISF;




class JSONGUIInputBoolWidget : public QWidget
{
	Q_OBJECT

public:
	explicit JSONGUIInputBoolWidget(const ISFAttrRef & inRef, QWidget *parent = nullptr);
	~JSONGUIInputBoolWidget();

private:
	Ui::JSONGUIInputBool *ui;
};

#endif // JSONGUIINPUTBOOL_H
