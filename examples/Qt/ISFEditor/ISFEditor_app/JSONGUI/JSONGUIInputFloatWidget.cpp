#include "JSONGUIInputFloatWidget.h"
#include "ui_JSONGUIInputFloatWidget.h"

#include <limits>

#include <QDebug>

#include "JSONScrollWidget.h"




JSONGUIInputFloatWidget::JSONGUIInputFloatWidget(const JGMInputRef & inRef, JSONScrollWidget * inScrollWidget, QWidget *parent) :
	JSONGUIInputWidget(inRef, inScrollWidget, parent),
	ui(new Ui::JSONGUIInputFloat)
{
	ui->setupUi(this);
	
	if (_input != nullptr)	{
		prepareUIItems();
		refreshUIItems();
	}
}
JSONGUIInputFloatWidget::~JSONGUIInputFloatWidget()
{
	delete ui;
}
void JSONGUIInputFloatWidget::prepareToBeDeleted()	{
	QObject::disconnect(ui->defaultSBox, 0, 0, 0);
	QObject::disconnect(ui->defaultCBox, 0, 0, 0);
	QObject::disconnect(ui->identitySBox, 0, 0, 0);
	QObject::disconnect(ui->identityCBox, 0, 0, 0);
	QObject::disconnect(ui->minSBox, 0, 0, 0);
	QObject::disconnect(ui->minCBox, 0, 0, 0);
	QObject::disconnect(ui->maxSBox, 0, 0, 0);
	QObject::disconnect(ui->maxCBox, 0, 0, 0);
	
	QObject::disconnect((ui->dragLabel), 0, 0, 0);
	QObject::disconnect((ui->inputNameEdit), 0, 0, 0);
	QObject::disconnect((ui->labelField), 0, 0, 0);
	QObject::disconnect((ui->typePUB), 0, 0, 0);
	QObject::disconnect((ui->deleteLabel), 0, 0, 0);
}




void JSONGUIInputFloatWidget::prepareUIItems() {
	//	have my super prepare the UI items common to all of these
	prepareDragLabel( (ui->dragLabel) );
	prepareInputNameEdit( (ui->inputNameEdit) );
	prepareLabelField( (ui->labelField) );
	prepareTypeCBox( (ui->typePUB) );
	prepareDeleteLabel( (ui->deleteLabel) );
	
	//	prepare the UI items specific to this input- there are a lot...
	
	//	default UI items
	QObject::disconnect(ui->defaultSBox, 0, 0, 0);
	QObject::connect(ui->defaultSBox, &QAbstractSpinBox::editingFinished, [&]()	{
		_input->setValue("DEFAULT", QJsonValue(ui->defaultSBox->value()));
		RecreateJSONAndExport();
	});
	QObject::disconnect(ui->defaultCBox, 0, 0, 0);
	QObject::connect(ui->defaultCBox, &QCheckBox::clicked, [&](bool checked)	{
		_input->setValue("DEFAULT", (checked) ? QJsonValue(ui->defaultSBox->minimum()) : QJsonValue::Undefined);
		RecreateJSONAndExport();
	});
	//	identity UI items
	QObject::disconnect(ui->identitySBox, 0, 0, 0);
	QObject::connect(ui->identitySBox,  &QAbstractSpinBox::editingFinished, [&]()	{
		_input->setValue("IDENTITY", QJsonValue(ui->identitySBox->value()));
		RecreateJSONAndExport();
	});
	QObject::disconnect(ui->identityCBox, 0, 0, 0);
	QObject::connect(ui->identityCBox, &QCheckBox::clicked, [&](bool checked)	{
		_input->setValue("IDENTITY", (checked) ? QJsonValue(ui->identitySBox->minimum()) : QJsonValue::Undefined);
		RecreateJSONAndExport();
	});
	//	minimum UI items
	QObject::disconnect(ui->minSBox, 0, 0, 0);
	QObject::connect(ui->minSBox, &QAbstractSpinBox::editingFinished, [&]()	{
		_input->setValue("MIN", QJsonValue(ui->minSBox->value()));
		RecreateJSONAndExport();
	});
	QObject::disconnect(ui->minCBox, 0, 0, 0);
	QObject::connect(ui->minCBox, &QCheckBox::clicked, [&](bool checked)	{
		_input->setValue("MIN", (checked) ? QJsonValue(ui->maxSBox->value()-1.0) : QJsonValue::Undefined);
		RecreateJSONAndExport();
	});
	//	maximum UI items
	QObject::disconnect(ui->maxSBox, 0, 0, 0);
	QObject::connect(ui->maxSBox, &QAbstractSpinBox::editingFinished, [&]()	{
		_input->setValue("MAX", QJsonValue(ui->maxSBox->value()));
		RecreateJSONAndExport();
	});
	QObject::disconnect(ui->maxCBox, 0, 0, 0);
	QObject::connect(ui->maxCBox, &QCheckBox::clicked, [&](bool checked)	{
		_input->setValue("MAX", (checked) ? QJsonValue(ui->minSBox->value()+1.0) : QJsonValue::Undefined);
		RecreateJSONAndExport();
	});
	
	
	
}
void JSONGUIInputFloatWidget::refreshUIItems() {
	//	have my super refresh the UI items common to all of these
	refreshInputNameEdit( (ui->inputNameEdit) );
	refreshLabelField( (ui->labelField) );
	refreshTypeCBox( (ui->typePUB) );
	prepareDeleteLabel( (ui->deleteLabel) );
	
	//	refresh the UI items specific to this input- there are a lot...
	
	double		tmpMin = 0.0;
	double		tmpMax = 1.0;
	double		tmpDef = 0.5;
	double		tmpIden = 0.5;
	bool		hasMin = false;
	bool		hasMax = false;
	bool		hasDef = false;
	bool		hasIden = false;
	QJsonValue	tmpVal;
	
	tmpVal = _input->value("MIN");
	if (tmpVal.isDouble())	{
		hasMin = true;
		tmpMin = tmpVal.toDouble();
	}
	tmpVal = _input->value("MAX");
	if (tmpVal.isDouble())	{
		hasMax = true;
		tmpMax = tmpVal.toDouble();
	}
	
	if (hasMin && hasMax && tmpMin >= tmpMax)
		tmpMin = tmpMax - 1.0;
	
	tmpVal = _input->value("DEFAULT");
	if (tmpVal.isDouble())	{
		hasDef = true;
		tmpDef = tmpVal.toDouble();
	}
	
	if (hasMin && tmpDef < tmpMin)
		tmpDef = tmpMin;
	if (hasMax && tmpDef > tmpMax)
		tmpDef = tmpMax;
	
	tmpVal = _input->value("IDENTITY");
	if (tmpVal.isDouble())	{
		hasIden = true;
		tmpIden = tmpVal.toDouble();
	}
	
	if (hasMin && tmpDef < tmpMin)
		tmpIden = tmpMin;
	if (hasMax && tmpDef > tmpMax)
		tmpIden = tmpMax;
	
	
	
	
	
	
	float		maxPossibleFloat = std::numeric_limits<float>::max();
	float		minPossibleFloat = std::numeric_limits<float>::min();
	
	
	
	
	
	
	
	ui->defaultSBox->blockSignals(true);
	ui->defaultCBox->blockSignals(true);
	ui->identitySBox->blockSignals(true);
	ui->identityCBox->blockSignals(true);
	ui->minSBox->blockSignals(true);
	ui->minCBox->blockSignals(true);
	ui->maxSBox->blockSignals(true);
	ui->maxCBox->blockSignals(true);
	
	//	default UI items
	ui->defaultSBox->setMinimum(minPossibleFloat);
	ui->defaultSBox->setMaximum( (hasMax) ? tmpMax : maxPossibleFloat );
	ui->defaultSBox->setMinimum( (hasMin) ? tmpMin : minPossibleFloat );
	ui->defaultSBox->setValue(tmpDef);
	ui->defaultSBox->setDisabled(!hasDef);
	ui->defaultCBox->setChecked(hasDef);
	
	//	identity UI items
	ui->identitySBox->setMinimum(minPossibleFloat);
	ui->identitySBox->setMaximum( (hasMax) ? tmpMax : maxPossibleFloat );
	ui->identitySBox->setMinimum( (hasMin) ? tmpMin : minPossibleFloat );
	ui->identitySBox->setValue(tmpIden);
	ui->identitySBox->setDisabled(!hasIden);
	ui->identityCBox->setChecked(hasIden);
	
	//	minimum UI items
	ui->minSBox->setMinimum(minPossibleFloat);
	ui->minSBox->setMaximum( (hasMax) ? tmpMax : maxPossibleFloat );
	ui->minSBox->setValue(tmpMin);
	ui->minSBox->setDisabled(!hasMin);
	ui->minCBox->setChecked(hasMin);
	
	//	maximum UI items
	ui->maxSBox->setMaximum(maxPossibleFloat);
	ui->maxSBox->setMinimum( (hasMin) ? tmpMin : minPossibleFloat );
	ui->maxSBox->setValue(tmpMax);
	ui->maxSBox->setDisabled(!hasMax);
	ui->maxCBox->setChecked(hasMax);
	
	ui->defaultSBox->blockSignals(false);
	ui->defaultCBox->blockSignals(false);
	ui->identitySBox->blockSignals(false);
	ui->identityCBox->blockSignals(false);
	ui->minSBox->blockSignals(false);
	ui->minCBox->blockSignals(false);
	ui->maxSBox->blockSignals(false);
	ui->maxCBox->blockSignals(false);
}
