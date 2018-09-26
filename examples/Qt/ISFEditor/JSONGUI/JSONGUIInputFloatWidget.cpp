#include "JSONGUIInputFloatWidget.h"
#include "ui_JSONGUIInputFloat.h"




JSONGUIInputFloatWidget::JSONGUIInputFloatWidget(const ISFAttrRef & inRef, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::JSONGUIInputFloat)
{
	ui->setupUi(this);
}

JSONGUIInputFloatWidget::~JSONGUIInputFloatWidget()
{
	delete ui;
}
