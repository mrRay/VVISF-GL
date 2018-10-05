#include "JSONGUIInputLongWidget.h"
#include "ui_JSONGUIInputLongWidget.h"




JSONGUIInputLongWidget::JSONGUIInputLongWidget(const JGMInputRef & inRef, QWidget *parent) :
	QWidget(parent),
	JSONGUIInput(inRef),
	ui(new Ui::JSONGUIInputLong)
{
	ui->setupUi(this);
	
	if (_input != nullptr)	{
		prepareUIItems();
		refreshUIItems();
	}
}

JSONGUIInputLongWidget::~JSONGUIInputLongWidget()
{
	delete ui;
}




void JSONGUIInputLongWidget::prepareUIItems() {
	//	have my super prepare the UI items common to all of these
	prepareInputNameEdit( *(ui->inputNameEdit) );
	prepareLabelField( *(ui->labelField) );
	prepareTypeCBox( *(ui->typePUB) );
	
	//	prepare the UI items specific to this input
}
void JSONGUIInputLongWidget::refreshUIItems() {
	//	have my super refresh the UI items common to all of these
	refreshInputNameEdit( *(ui->inputNameEdit) );
	refreshLabelField( *(ui->labelField) );
	refreshTypeCBox( *(ui->typePUB) );
	
	//	refresh the UI items specific to this input
}
