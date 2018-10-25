#include "JSONGUIInputColorWidget.h"
#include "ui_JSONGUIInputColorWidget.h"

#include <QColorDialog>
#include <QJsonArray>
#include <QDebug>

#include "JSONScrollWidget.h"
#include "QLabelClickable.h"




QColor ConvertArrayJSONToColor(const QJsonValue & inVal)	{
	if (!inVal.isArray())
		return QColor(255,255,255,255);
	QColor		tmpColor;
	int			tmpInt = 0;
	for (const QJsonValue & arrayVal : inVal.toArray())	{
		switch(tmpInt)	{
		case 0:		tmpColor.setRedF(arrayVal.toDouble());		break;
		case 1:		tmpColor.setGreenF(arrayVal.toDouble());		break;
		case 2:		tmpColor.setBlueF(arrayVal.toDouble());		break;
		case 3:		tmpColor.setAlphaF(arrayVal.toDouble());		break;
		}
		++tmpInt;
	}
	return tmpColor;
}








JSONGUIInputColorWidget::JSONGUIInputColorWidget(const JGMInputRef & inRef, QWidget *parent) :
	JSONGUIInputWidget(inRef, parent),
	ui(new Ui::JSONGUIInputColor)
{
	ui->setupUi(this);
	
	if (_input != nullptr)	{
		prepareUIItems();
		refreshUIItems();
	}
}

JSONGUIInputColorWidget::~JSONGUIInputColorWidget()
{
	delete ui;
}




void JSONGUIInputColorWidget::prepareUIItems() {
	//	have my super prepare the UI items common to all of these
	prepareDragLabel( (ui->dragLabel) );
	prepareInputNameEdit( (ui->inputNameEdit) );
	prepareLabelField( (ui->labelField) );
	prepareTypeCBox( (ui->typePUB) );
	prepareDeleteLabel( (ui->deleteLabel) );
	
	//	prepare the UI items specific to this input- there are a lot...
	
	//	default UI items (color & checkbox)
	QObject::disconnect(ui->defaultLabel, 0, 0, 0);
	QObject::connect(ui->defaultLabel, &QLabelClickable::clicked, [&]()	{
		//	make a color dialog
		QColorDialog		*colorDialog = new QColorDialog(this);
		colorDialog->setOptions( QColorDialog::DontUseNativeDialog );
		colorDialog->setAttribute( Qt::WA_DeleteOnClose );
		//	the color dialog updates the attr & saves the file when it closed by the user clicking "OK"
		connect(colorDialog, &QColorDialog::colorSelected, [&](const QColor & inColor)	{
			QJsonArray		tmpArray = {
				double(inColor.red())/255.0,
				double(inColor.green())/255.0,
				double(inColor.blue())/255.0,
				double(inColor.alpha())/255.0
			};
			_input->setValue("DEFAULT", QJsonValue(tmpArray));
			RecreateJSONAndExport();
		});
		//	open the color dialog
		colorDialog->open();
	});
	QObject::disconnect(ui->defaultCBox, 0, 0, 0);
	QObject::connect(ui->defaultCBox, &QCheckBox::clicked, [&](bool checked)	{
		if (checked)	{
			QJsonArray		tmpArray = { 1.0, 1.0, 1.0, 1.0 };
			_input->setValue("DEFAULT", QJsonValue(tmpArray));
		}
		else
			_input->setValue("DEFAULT", QJsonValue::Undefined);
		RecreateJSONAndExport();
	});
	
	
	//	identity UI items (color & checkbox)
	QObject::disconnect(ui->identityLabel, 0, 0, 0);
	QObject::connect(ui->identityLabel, &QLabelClickable::clicked, [&]()	{
		//	make a color dialog
		QColorDialog		*colorDialog = new QColorDialog(this);
		colorDialog->setOptions( QColorDialog::DontUseNativeDialog );
		colorDialog->setAttribute( Qt::WA_DeleteOnClose );
		//	the color dialog updates the attr & saves the file when it closed by the user clicking "OK"
		connect(colorDialog, &QColorDialog::colorSelected, [&](const QColor & inColor)	{
			QJsonArray		tmpArray = {
				double(inColor.red())/255.0,
				double(inColor.green())/255.0,
				double(inColor.blue())/255.0,
				double(inColor.alpha())/255.0
			};
			_input->setValue("IDENTITY", QJsonValue(tmpArray));
			RecreateJSONAndExport();
		});
		//	open the color dialog
		colorDialog->open();
	});
	QObject::disconnect(ui->identityCBox, 0, 0, 0);
	QObject::connect(ui->identityCBox, &QCheckBox::clicked, [&](bool checked)	{
		if (checked)	{
			QJsonArray		tmpArray = { 1.0, 1.0, 1.0, 1.0 };
			_input->setValue("IDENTITY", QJsonValue(tmpArray));
		}
		else
			_input->setValue("IDENTITY", QJsonValue::Undefined);
		RecreateJSONAndExport();
	});
	
	
	//	minimum UI items (color & checkbox)
	QObject::disconnect(ui->minLabel, 0, 0, 0);
	QObject::connect(ui->minLabel, &QLabelClickable::clicked, [&]()	{
		//	make a color dialog
		QColorDialog		*colorDialog = new QColorDialog(this);
		colorDialog->setOptions( QColorDialog::DontUseNativeDialog );
		colorDialog->setAttribute( Qt::WA_DeleteOnClose );
		//	the color dialog updates the attr & saves the file when it closed by the user clicking "OK"
		connect(colorDialog, &QColorDialog::colorSelected, [&](const QColor & inColor)	{
			QJsonArray		tmpArray = {
				double(inColor.red())/255.0,
				double(inColor.green())/255.0,
				double(inColor.blue())/255.0,
				double(inColor.alpha())/255.0
			};
			_input->setValue("MIN", QJsonValue(tmpArray));
			RecreateJSONAndExport();
		});
		//	open the color dialog
		colorDialog->open();
	});
	QObject::disconnect(ui->minCBox, 0, 0, 0);
	QObject::connect(ui->minCBox, &QCheckBox::clicked, [&](bool checked)	{
		if (checked)	{
			QJsonArray		tmpArray = { 1.0, 1.0, 1.0, 1.0 };
			_input->setValue("MIN", QJsonValue(tmpArray));
		}
		else
			_input->setValue("MIN", QJsonValue::Undefined);
		RecreateJSONAndExport();
	});
	
	
	//	maximum UI items (color & checkbox)
	QObject::disconnect(ui->maxLabel, 0, 0, 0);
	QObject::connect(ui->maxLabel, &QLabelClickable::clicked, [&]()	{
		//	make a color dialog
		QColorDialog		*colorDialog = new QColorDialog(this);
		colorDialog->setOptions( QColorDialog::DontUseNativeDialog );
		colorDialog->setAttribute( Qt::WA_DeleteOnClose );
		//	the color dialog updates the attr & saves the file when it closed by the user clicking "OK"
		connect(colorDialog, &QColorDialog::colorSelected, [&](const QColor & inColor)	{
			QJsonArray		tmpArray = {
				double(inColor.red())/255.0,
				double(inColor.green())/255.0,
				double(inColor.blue())/255.0,
				double(inColor.alpha())/255.0
			};
			_input->setValue("MAX", QJsonValue(tmpArray));
			RecreateJSONAndExport();
		});
		//	open the color dialog
		colorDialog->open();
	});
	QObject::disconnect(ui->maxCBox, 0, 0, 0);
	QObject::connect(ui->maxCBox, &QCheckBox::clicked, [&](bool checked)	{
		if (checked)	{
			QJsonArray		tmpArray = { 1.0, 1.0, 1.0, 1.0 };
			_input->setValue("MAX", QJsonValue(tmpArray));
		}
		else
			_input->setValue("MAX", QJsonValue::Undefined);
		RecreateJSONAndExport();
	});
}
void JSONGUIInputColorWidget::refreshUIItems() {
	//	have my super refresh the UI items common to all of these
	refreshInputNameEdit( (ui->inputNameEdit) );
	refreshLabelField( (ui->labelField) );
	refreshTypeCBox( (ui->typePUB) );
	prepareDeleteLabel( (ui->deleteLabel) );
	
	//	refresh the UI items specific to this input- there are a lot...
	
	QPalette			tmpPalette;
	QColor				tmpColor;
	bool				tmpBool;
	
	//	default UI items (color & checkbox)
	if (_input->contains("DEFAULT"))	{
		tmpColor = ConvertArrayJSONToColor(_input->value("DEFAULT"));
		tmpBool = true;
	}
	else	{
		tmpColor = QColor(255,255,255,255);
		tmpBool = false;
	}
	tmpPalette = ui->defaultLabel->palette();
	tmpPalette.setColor(ui->defaultLabel->backgroundRole(), tmpColor);
	ui->defaultLabel->setPalette(tmpPalette);
	ui->defaultCBox->blockSignals(true);
	ui->defaultCBox->setChecked(tmpBool);
	ui->defaultCBox->blockSignals(false);
	//	identity UI items (color & checkbox)
	if (_input->contains("IDENTITY"))	{
		tmpColor = ConvertArrayJSONToColor(_input->value("IDENTITY"));
		tmpBool = true;
	}
	else	{
		tmpColor = QColor(255,255,255,255);
		tmpBool = false;
	}
	tmpPalette = ui->identityLabel->palette();
	tmpPalette.setColor(ui->identityLabel->backgroundRole(), tmpColor);
	ui->identityLabel->setPalette(tmpPalette);
	ui->identityCBox->blockSignals(true);
	ui->identityCBox->setChecked(tmpBool);
	ui->identityCBox->blockSignals(false);
	//	minimum UI items (color & checkbox)
	if (_input->contains("MIN"))	{
		tmpColor = ConvertArrayJSONToColor(_input->value("MIN"));
		tmpBool = true;
	}
	else	{
		tmpColor = QColor(255,255,255,255);
		tmpBool = false;
	}
	tmpPalette = ui->minLabel->palette();
	tmpPalette.setColor(ui->minLabel->backgroundRole(), tmpColor);
	ui->minLabel->setPalette(tmpPalette);
	ui->minCBox->blockSignals(true);
	ui->minCBox->setChecked(tmpBool);
	ui->minCBox->blockSignals(false);
	//	maximum UI items (color & checkbox)
	if (_input->contains("MAX"))	{
		tmpColor = ConvertArrayJSONToColor(_input->value("MAX"));
		tmpBool = true;
	}
	else	{
		tmpColor = QColor(255,255,255,255);
		tmpBool = false;
	}
	tmpPalette = ui->maxLabel->palette();
	tmpPalette.setColor(ui->maxLabel->backgroundRole(), tmpColor);
	ui->maxLabel->setPalette(tmpPalette);
	ui->maxCBox->blockSignals(true);
	ui->maxCBox->setChecked(tmpBool);
	ui->maxCBox->blockSignals(false);
}
