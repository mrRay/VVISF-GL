#include "JSONGUIInputPoint2DWidget.h"
#include "ui_JSONGUIInputPoint2D.h"




JSONGUIInputPoint2DWidget::JSONGUIInputPoint2DWidget(const ISFAttrRef & inRef, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::JSONGUIInputPoint2D)
{
	ui->setupUi(this);
}

JSONGUIInputPoint2DWidget::~JSONGUIInputPoint2DWidget()
{
	delete ui;
}
