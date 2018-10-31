#include "JSONGUIInputAudioWidget.h"
#include "ui_JSONGUIInputAudioWidget.h"




JSONGUIInputAudioWidget::JSONGUIInputAudioWidget(const JGMInputRef & inRef, JSONScrollWidget * inScrollWidget, QWidget *parent) :
	JSONGUIInputWidget(inRef, inScrollWidget, parent),
	ui(new Ui::JSONGUIInputAudio)
{
	ui->setupUi(this);
	
	if (_input != nullptr)	{
		prepareUIItems();
		refreshUIItems();
	}
}

JSONGUIInputAudioWidget::~JSONGUIInputAudioWidget()
{
	delete ui;
}




void JSONGUIInputAudioWidget::prepareUIItems() {
	//	have my super prepare the UI items common to all of these
	prepareDragLabel( (ui->dragLabel) );
	prepareInputNameEdit( (ui->inputNameEdit) );
	prepareLabelField( (ui->labelField) );
	prepareTypeCBox( (ui->typePUB) );
	prepareDeleteLabel( (ui->deleteLabel) );
	
	//	prepare the UI items specific to this input
}
void JSONGUIInputAudioWidget::refreshUIItems() {
	//	have my super refresh the UI items common to all of these
	refreshInputNameEdit( (ui->inputNameEdit) );
	refreshLabelField( (ui->labelField) );
	refreshTypeCBox( (ui->typePUB) );
	prepareDeleteLabel( (ui->deleteLabel) );
	
	//	refresh the UI items specific to this input
}
