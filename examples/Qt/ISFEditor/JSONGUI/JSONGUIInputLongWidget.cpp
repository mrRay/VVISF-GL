#include "JSONGUIInputLongWidget.h"
#include "ui_JSONGUIInputLong.h"




JSONGUIInputLongWidget::JSONGUIInputLongWidget(const ISFAttrRef & inRef, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::JSONGUIInputLong)
{
	ui->setupUi(this);
}

JSONGUIInputLongWidget::~JSONGUIInputLongWidget()
{
	delete ui;
}
