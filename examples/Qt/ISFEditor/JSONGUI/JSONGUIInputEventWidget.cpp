#include "JSONGUIInputEventWidget.h"
#include "ui_JSONGUIInputEvent.h"

JSONGUIInputEventWidget::JSONGUIInputEventWidget(const ISFAttrRef & inRef, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::JSONGUIInputEvent)
{
	ui->setupUi(this);
}

JSONGUIInputEventWidget::~JSONGUIInputEventWidget()
{
	delete ui;
}
