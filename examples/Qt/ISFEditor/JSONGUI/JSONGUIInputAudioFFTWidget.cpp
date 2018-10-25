#include "JSONGUIInputAudioFFTWidget.h"
#include "ui_JSONGUIInputAudioFFTWidget.h"




JSONGUIInputAudioFFTWidget::JSONGUIInputAudioFFTWidget(const JGMInputRef & inRef, QWidget *parent) :
	JSONGUIInputWidget(inRef, parent),
	ui(new Ui::JSONGUIInputAudioFFT)
{
	ui->setupUi(this);
	
	if (_input != nullptr)	{
		prepareUIItems();
		refreshUIItems();
	}
}

JSONGUIInputAudioFFTWidget::~JSONGUIInputAudioFFTWidget()
{
	delete ui;
}




void JSONGUIInputAudioFFTWidget::prepareUIItems() {
	//	have my super prepare the UI items common to all of these
	prepareDragLabel( (ui->dragLabel) );
	prepareInputNameEdit( (ui->inputNameEdit) );
	prepareLabelField( (ui->labelField) );
	prepareTypeCBox( (ui->typePUB) );
	prepareDeleteLabel( (ui->deleteLabel) );
	
	//	prepare the UI items specific to this input
}
void JSONGUIInputAudioFFTWidget::refreshUIItems() {
	//	have my super refresh the UI items common to all of these
	refreshInputNameEdit( (ui->inputNameEdit) );
	refreshLabelField( (ui->labelField) );
	refreshTypeCBox( (ui->typePUB) );
	prepareDeleteLabel( (ui->deleteLabel) );
	
	//	refresh the UI items specific to this input
}
