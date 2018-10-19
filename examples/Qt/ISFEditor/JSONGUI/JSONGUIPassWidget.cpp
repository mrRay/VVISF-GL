#include "JSONGUIPassWidget.h"
#include "ui_JSONGUIPassWidget.h"

#include <QDebug>




JSONGUIPassWidget::JSONGUIPassWidget(const JGMPassRef & inRef, QWidget *parent) :
	QWidget(parent),
	JSONGUIPass(inRef),
	ui(new Ui::JSONGUIPassWidget)
{
	ui->setupUi(this);
	
	if (_pass != nullptr)	{
		prepareUIItems();
		refreshUIItems();
	}
}

JSONGUIPassWidget::~JSONGUIPassWidget()
{
	delete ui;
}




void JSONGUIPassWidget::prepareUIItems()	{
	//qDebug() << __PRETTY_FUNCTION__;
	
	prepareDeleteLabel( *(ui->deleteLabel) );
	prepareBufferNameEdit( *(ui->targetBufferEdit) );
	preparePBufferCBox( *(ui->persistentCBox) );
	prepareFBufferCBox( *(ui->floatCBox) );
	prepareCustWidthEdit( *(ui->widthEdit) );
	prepareCustHeightEdit( *(ui->heightEdit) );
}
void JSONGUIPassWidget::refreshUIItems()	{
	//qDebug() << __PRETTY_FUNCTION__;
	
	refreshPassTitleLabel( *(ui->titleLabel) );
	refreshBufferNameEdit( *(ui->targetBufferEdit) );
	refreshPBufferCBox( *(ui->persistentCBox) );
	refreshFBufferCBox( *(ui->floatCBox) );
	refreshCustWidthEdit( *(ui->widthEdit) );
	refreshCustHeightEdit( *(ui->heightEdit) );
}

