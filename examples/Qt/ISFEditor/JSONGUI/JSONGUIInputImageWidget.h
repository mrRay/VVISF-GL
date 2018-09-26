#ifndef JSONGUIINPUTIMAGE_H
#define JSONGUIINPUTIMAGE_H

#include <QWidget>

#include "VVISF.hpp"




namespace Ui {
	class JSONGUIInputImage;
}

using namespace VVISF;




class JSONGUIInputImageWidget : public QWidget
{
	Q_OBJECT

public:
	explicit JSONGUIInputImageWidget(const ISFAttrRef & inRef, QWidget *parent = nullptr);
	~JSONGUIInputImageWidget();

private:
	Ui::JSONGUIInputImage *ui;
};

#endif // JSONGUIINPUTIMAGE_H
