#include "JSONGUIInputBoolWidget.h"
#include "ui_JSONGUIInputBool.h"




JSONGUIInputBoolWidget::JSONGUIInputBoolWidget(const ISFAttrRef & inRef, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::JSONGUIInputBool)
{
	ui->setupUi(this);
}

JSONGUIInputBoolWidget::~JSONGUIInputBoolWidget()
{
	delete ui;
}
