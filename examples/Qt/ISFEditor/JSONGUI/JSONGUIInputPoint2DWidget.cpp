#include "JSONGUIInputPoint2DWidget.h"
#include "ui_JSONGUIInputPoint2DWidget.h"




JSONGUIInputPoint2DWidget::JSONGUIInputPoint2DWidget(const JGMInputRef & inRef, QWidget *parent) :
	JSONGUIInputWidget(inRef, parent),
	ui(new Ui::JSONGUIInputPoint2D)
{
	ui->setupUi(this);
	
	if (_input != nullptr)	{
		prepareUIItems();
		refreshUIItems();
	}
}

JSONGUIInputPoint2DWidget::~JSONGUIInputPoint2DWidget()
{
	delete ui;
}




void JSONGUIInputPoint2DWidget::prepareUIItems() {
	//	have my super prepare the UI items common to all of these
	prepareDragLabel( (ui->dragLabel) );
	prepareInputNameEdit( (ui->inputNameEdit) );
	prepareLabelField( (ui->labelField) );
	prepareTypeCBox( (ui->typePUB) );
	prepareDeleteLabel( (ui->deleteLabel) );
	
	//	prepare the UI items specific to this input
}
void JSONGUIInputPoint2DWidget::refreshUIItems() {
	//	have my super refresh the UI items common to all of these
	refreshInputNameEdit( (ui->inputNameEdit) );
	refreshLabelField( (ui->labelField) );
	refreshTypeCBox( (ui->typePUB) );
	prepareDeleteLabel( (ui->deleteLabel) );
	
	//	refresh the UI items specific to this input
}
