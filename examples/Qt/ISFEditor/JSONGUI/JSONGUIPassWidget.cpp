#include "JSONGUIPassWidget.h"
#include "ui_JSONGUIPass.h"




JSONGUIPassWidget::JSONGUIPassWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::JSONGUIPass)
{
	ui->setupUi(this);
}

JSONGUIPassWidget::~JSONGUIPassWidget()
{
	delete ui;
}
