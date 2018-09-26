#ifndef JSONGUIINPUTPOINT2D_H
#define JSONGUIINPUTPOINT2D_H

#include <QWidget>

#include "VVISF.hpp"




namespace Ui {
	class JSONGUIInputPoint2D;
}

using namespace VVISF;




class JSONGUIInputPoint2DWidget : public QWidget
{
	Q_OBJECT

public:
	explicit JSONGUIInputPoint2DWidget(const ISFAttrRef & inRef, QWidget *parent = nullptr);
	~JSONGUIInputPoint2DWidget();

private:
	Ui::JSONGUIInputPoint2D *ui;
};

#endif // JSONGUIINPUTPOINT2D_H
