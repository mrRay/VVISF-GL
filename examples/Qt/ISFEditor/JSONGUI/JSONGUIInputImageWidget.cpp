#include "JSONGUIInputImageWidget.h"
#include "ui_JSONGUIInputImageWidget.h"




JSONGUIInputImageWidget::JSONGUIInputImageWidget(const JGMInputRef & inRef, QWidget *parent) :
	QWidget(parent),
	JSONGUIInput(inRef),
	ui(new Ui::JSONGUIInputImage)
{
	ui->setupUi(this);
	
	if (_input != nullptr)	{
		prepareUIItems();
		refreshUIItems();
	}
}

JSONGUIInputImageWidget::~JSONGUIInputImageWidget()
{
	delete ui;
}




void JSONGUIInputImageWidget::prepareUIItems() {
	//	have my super prepare the UI items common to all of these
	prepareInputNameEdit( *(ui->inputNameEdit) );
	prepareLabelField( *(ui->labelField) );
	prepareTypeCBox( *(ui->typePUB) );
	prepareDeleteLabel( *(ui->deleteLabel) );
	
	//	prepare the UI items specific to this input
}
void JSONGUIInputImageWidget::refreshUIItems() {
	//	have my super refresh the UI items common to all of these
	refreshInputNameEdit( *(ui->inputNameEdit) );
	refreshLabelField( *(ui->labelField) );
	refreshTypeCBox( *(ui->typePUB) );
	prepareDeleteLabel( *(ui->deleteLabel) );
	
	//	refresh the UI items specific to this input
}
