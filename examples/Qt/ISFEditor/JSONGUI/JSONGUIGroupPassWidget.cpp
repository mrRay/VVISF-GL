#include "JSONGUIGroupPassWidget.h"
#include "ui_JSONGUIGroupPass.h"




JSONGUIGroupPassWidget::JSONGUIGroupPassWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::JSONGUIGroupPass)
{
	ui->setupUi(this);
	
	QPalette			p = palette();
	p.setColor(QPalette::Background, p.color(QPalette::Background).darker(110));
	setAutoFillBackground(true);
	setPalette(p);
}

JSONGUIGroupPassWidget::~JSONGUIGroupPassWidget()
{
	delete ui;
}
