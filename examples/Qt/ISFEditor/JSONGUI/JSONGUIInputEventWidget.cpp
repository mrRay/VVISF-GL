#include "JSONGUIInputEventWidget.h"
#include "ui_JSONGUIInputEventWidget.h"

JSONGUIInputEventWidget::JSONGUIInputEventWidget(const JGMInputRef & inRef, QWidget *parent) :
	QWidget(parent),
	JSONGUIInput(inRef),
	ui(new Ui::JSONGUIInputEvent)
{
	ui->setupUi(this);
	
	if (_input != nullptr)	{
		prepareUIItems();
		refreshUIItems();
	}
}

JSONGUIInputEventWidget::~JSONGUIInputEventWidget()
{
	delete ui;
}




void JSONGUIInputEventWidget::prepareUIItems() {
	//	have my super prepare the UI items common to all of these
	prepareInputNameEdit( *(ui->inputNameEdit) );
	prepareLabelField( *(ui->labelField) );
	prepareTypeCBox( *(ui->typePUB) );
	prepareDeleteLabel( *(ui->deleteLabel) );
	
	//	prepare the UI items specific to this input
}
void JSONGUIInputEventWidget::refreshUIItems() {
	//	have my super refresh the UI items common to all of these
	refreshInputNameEdit( *(ui->inputNameEdit) );
	refreshLabelField( *(ui->labelField) );
	refreshTypeCBox( *(ui->typePUB) );
	prepareDeleteLabel( *(ui->deleteLabel) );
	
	//	refresh the UI items specific to this input
}
