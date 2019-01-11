#include "JSONGUIInputAudioFFTWidget.h"
#include "ui_JSONGUIInputAudioFFTWidget.h"




JSONGUIInputAudioFFTWidget::JSONGUIInputAudioFFTWidget(const JGMInputRef & inRef, JSONScrollWidget * inScrollWidget, QWidget *parent) :
	JSONGUIInputWidget(inRef, inScrollWidget, parent),
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
void JSONGUIInputAudioFFTWidget::prepareToBeDeleted()	{
	QObject::disconnect((ui->maxSBox), 0, 0, 0);
	QObject::disconnect((ui->maxCBox), 0, 0, 0);
	
	QObject::disconnect((ui->dragLabel), 0, 0, 0);
	QObject::disconnect((ui->inputNameEdit), 0, 0, 0);
	QObject::disconnect((ui->labelField), 0, 0, 0);
	QObject::disconnect((ui->typePUB), 0, 0, 0);
	QObject::disconnect((ui->deleteLabel), 0, 0, 0);
}




void JSONGUIInputAudioFFTWidget::prepareUIItems() {
	//	have my super prepare the UI items common to all of these
	prepareDragLabel( (ui->dragLabel) );
	prepareInputNameEdit( (ui->inputNameEdit) );
	prepareLabelField( (ui->labelField) );
	prepareTypeCBox( (ui->typePUB) );
	prepareDeleteLabel( (ui->deleteLabel) );
	
	//	prepare the UI items specific to this input
	
	//	maximum UI items
	QObject::disconnect(ui->maxSBox, 0, 0, 0);
	QObject::connect(ui->maxSBox, &QAbstractSpinBox::editingFinished, [&]()	{
		_input->setValue("MAX", QJsonValue(ui->maxSBox->value()));
		RecreateJSONAndExport();
	});
	QObject::disconnect(ui->maxCBox, 0, 0, 0);
	QObject::connect(ui->maxCBox, &QCheckBox::clicked, [&](bool checked)	{
		_input->setValue("MAX", (checked) ? QJsonValue(100.0) : QJsonValue::Undefined);
		RecreateJSONAndExport();
	});
}
void JSONGUIInputAudioFFTWidget::refreshUIItems() {
	//	have my super refresh the UI items common to all of these
	refreshInputNameEdit( (ui->inputNameEdit) );
	refreshLabelField( (ui->labelField) );
	refreshTypeCBox( (ui->typePUB) );
	prepareDeleteLabel( (ui->deleteLabel) );
	
	//	refresh the UI items specific to this input
	
	
	int			tmpMax = 0;
	bool		hasMax = false;
	QJsonValue	tmpVal;
	
	tmpVal = _input->value("MAX");
	if (tmpVal.isDouble())	{
		hasMax = true;
		tmpMax = tmpVal.toInt();
	}
	
	int		maxPossibleInt = std::numeric_limits<int>::max();
	
	ui->maxSBox->blockSignals(true);
	ui->maxCBox->blockSignals(true);
	
	//	maximum UI items
	ui->maxSBox->setMaximum(maxPossibleInt);
	ui->maxSBox->setMinimum( 1 );
	ui->maxSBox->setValue(tmpMax);
	ui->maxSBox->setDisabled(!hasMax);
	ui->maxCBox->setChecked(hasMax);
	
	ui->maxSBox->blockSignals(false);
	ui->maxCBox->blockSignals(false);
}
