#ifndef JSONGUIINPUTCOLOR_H
#define JSONGUIINPUTCOLOR_H

#include <QWidget>

#include "VVISF.hpp"




namespace Ui {
	class JSONGUIInputColor;
}

using namespace VVISF;




class JSONGUIInputColorWidget : public QWidget
{
	Q_OBJECT

public:
	explicit JSONGUIInputColorWidget(const ISFAttrRef & inRef, QWidget *parent = nullptr);
	~JSONGUIInputColorWidget();

private:
	Ui::JSONGUIInputColor *ui;
};

#endif // JSONGUIINPUTCOLOR_H
