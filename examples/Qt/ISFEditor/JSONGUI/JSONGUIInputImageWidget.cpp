#include "JSONGUIInputImageWidget.h"
#include "ui_JSONGUIInputImage.h"




JSONGUIInputImageWidget::JSONGUIInputImageWidget(const ISFAttrRef & inRef, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::JSONGUIInputImage)
{
	ui->setupUi(this);
}

JSONGUIInputImageWidget::~JSONGUIInputImageWidget()
{
	delete ui;
}
