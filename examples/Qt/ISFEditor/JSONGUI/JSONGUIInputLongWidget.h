#ifndef JSONGUIINPUTLONG_H
#define JSONGUIINPUTLONG_H

#include <QWidget>

#include "VVISF.hpp"




namespace Ui {
	class JSONGUIInputLong;
}

using namespace VVISF;




class JSONGUIInputLongWidget : public QWidget
{
	Q_OBJECT

public:
	explicit JSONGUIInputLongWidget(const ISFAttrRef & inRef, QWidget *parent = nullptr);
	~JSONGUIInputLongWidget();

private:
	Ui::JSONGUIInputLong *ui;
};

#endif // JSONGUIINPUTLONG_H
