#include "JSONGUIGroupInputWidget.h"
#include "ui_JSONGUIGroupInput.h"




JSONGUIGroupInputWidget::JSONGUIGroupInputWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::JSONGUIGroupInput)
{
	ui->setupUi(this);
	
	QPalette			p = palette();
	p.setColor(QPalette::Background, p.color(QPalette::Background).darker(110));
	setAutoFillBackground(true);
	setPalette(p);
}

JSONGUIGroupInputWidget::~JSONGUIGroupInputWidget()
{
	delete ui;
}
