#include "JSONGUIInputColorWidget.h"
#include "ui_JSONGUIInputColor.h"




JSONGUIInputColorWidget::JSONGUIInputColorWidget(const ISFAttrRef & inRef, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::JSONGUIInputColor)
{
	ui->setupUi(this);
}

JSONGUIInputColorWidget::~JSONGUIInputColorWidget()
{
	delete ui;
}
