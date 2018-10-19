#include "JSONGUIInputBoolWidget.h"
#include "ui_JSONGUIInputBoolWidget.h"

#include "JSONScrollWidget.h"




JSONGUIInputBoolWidget::JSONGUIInputBoolWidget(const JGMInputRef & inRef, QWidget *parent) :
	QWidget(parent),
	JSONGUIInput(inRef),
	ui(new Ui::JSONGUIInputBool)
{
	ui->setupUi(this);
	
	if (_input != nullptr)	{
		prepareUIItems();
		refreshUIItems();
	}
}

JSONGUIInputBoolWidget::~JSONGUIInputBoolWidget()
{
	delete ui;
}




void JSONGUIInputBoolWidget::prepareUIItems() {
	//	have my super prepare the UI items common to all of these
	prepareInputNameEdit( *(ui->inputNameEdit) );
	prepareLabelField( *(ui->labelField) );
	prepareTypeCBox( *(ui->typePUB) );
	prepareDeleteLabel( *(ui->deleteLabel) );
	
	//	prepare the UI items specific to this input
	QObject::disconnect(ui->defaultCBox, 0, 0, 0);
	QObject::connect(ui->defaultCBox, &QCheckBox::clicked, [&](bool checked)	{
		if (_input == nullptr)
			return;
		if (checked)
			_input->setValue("DEFAULT", QJsonValue(checked));
		else
			_input->setValue("DEFAULT", QJsonValue::Undefined);
		RecreateJSONAndExport();
	});
}
void JSONGUIInputBoolWidget::refreshUIItems() {
	//	have my super refresh the UI items common to all of these
	refreshInputNameEdit( *(ui->inputNameEdit) );
	refreshLabelField( *(ui->labelField) );
	refreshTypeCBox( *(ui->typePUB) );
	prepareDeleteLabel( *(ui->deleteLabel) );
	
	//	refresh the UI items specific to this input
	QJsonValue		defVal = (!_input->contains("DEFAULT")) ? QJsonValue(false) : _input->value("DEFAULT");
	bool			def = false;
	if (defVal.isBool())
		def = defVal.toBool();
	else
		def = ((defVal.isBool()&&defVal.toBool()) || (defVal.isDouble()&&defVal.toDouble()>0.0)) ? true : false;
	ui->defaultCBox->blockSignals(true);
	ui->defaultCBox->setChecked(def);
	ui->defaultCBox->blockSignals(false);
	
	
	QJsonValue		idenVal = (!_input->contains("IDENTITY")) ? QJsonValue(false) : _input->value("DEFAULT");
	bool			iden = false;
	if (idenVal.isBool())
		iden = idenVal.toBool();
	else
		iden = ((idenVal.isBool()&&idenVal.toBool()) || (idenVal.isDouble()&&idenVal.toDouble()>0.0)) ? true  :false;
	ui->identityCBox->blockSignals(true);
	ui->identityCBox->setChecked(iden);
	ui->identityCBox->blockSignals(false);
}
