#include "JSONGUIInputLongWidget.h"
#include "ui_JSONGUIInputLongWidget.h"




JSONGUIInputLongWidget::JSONGUIInputLongWidget(const JGMInputRef & inRef, JSONScrollWidget * inScrollWidget, QWidget *parent) :
	JSONGUIInputWidget(inRef, inScrollWidget, parent),
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
void JSONGUIInputLongWidget::prepareToBeDeleted()	{
	QObject::disconnect((ui->defaultField), 0, 0, 0);
	QObject::disconnect((ui->identityField), 0, 0, 0);
	QObject::disconnect((ui->valuesField), 0, 0, 0);
	QObject::disconnect((ui->labelsField), 0, 0, 0);
	
	QObject::disconnect((ui->dragLabel), 0, 0, 0);
	QObject::disconnect((ui->inputNameEdit), 0, 0, 0);
	QObject::disconnect((ui->labelField), 0, 0, 0);
	QObject::disconnect((ui->typePUB), 0, 0, 0);
	QObject::disconnect((ui->deleteLabel), 0, 0, 0);
}




void JSONGUIInputLongWidget::prepareUIItems() {
	//	have my super prepare the UI items common to all of these
	prepareDragLabel( (ui->dragLabel) );
	prepareInputNameEdit( (ui->inputNameEdit) );
	prepareLabelField( (ui->labelField) );
	prepareTypeCBox( (ui->typePUB) );
	prepareDeleteLabel( (ui->deleteLabel) );
	
	//	prepare the UI items specific to this input
	
	//	default field
	QObject::disconnect(ui->defaultField, 0, 0, 0);
	QObject::connect(ui->defaultField, &QLineEdit::editingFinished, [&]()	{
		//	get text as string, convert to int
		QString			tmpStr = ui->defaultField->text();
		bool			success = true;
		int				tmpInt = tmpStr.toInt(&success);
		if (!success)	{
			ui->defaultField->setText("");
			return;
		}
		if (tmpStr.length() < 1)
			_input->setValue("DEFAULT", QJsonValue::Undefined);
		else
			_input->setValue("DEFAULT", QJsonValue(tmpInt));
		RecreateJSONAndExport();
	});
	//	identity field
	QObject::disconnect(ui->identityField, 0, 0, 0);
	QObject::connect(ui->identityField, &QLineEdit::editingFinished, [&]()	{
		//	get text as string, convert to int
		QString			tmpStr = ui->identityField->text();
		bool			success = true;
		int				tmpInt = tmpStr.toInt(&success);
		if (!success)	{
			ui->identityField->setText("");
			return;
		}
		if (tmpStr.length() < 1)
			_input->setValue("IDENTITY", QJsonValue::Undefined);
		else
			_input->setValue("IDENTITY", QJsonValue(tmpInt));
		RecreateJSONAndExport();
	});
	//	values field
	QObject::disconnect(ui->valuesField, 0, 0, 0);
	QObject::connect(ui->valuesField, &QLineEdit::editingFinished, [&]()	{
		//	get text as string, break up to array of strings
		QString			tmpStr = ui->valuesField->text();
		QStringList		tmpStrList = tmpStr.split(QRegularExpression("[^-0-9\\.]+"));
		//	run through array of strings, converting to int and making a new array of ints
		QList<int>		tmpIntList;
		for (const QString & listStr : tmpStrList)	{
			if (listStr.length() < 1)
				continue;
			bool			success = true;
			int				tmpInt = listStr.toInt(&success);
			if (success)
				tmpIntList.append(tmpInt);
		}
		//	make a new string from the list of ints, use it to populate the sender, also make an array of ints, use it to populate the input ref
		QString			newStr;
		QJsonArray		newArray;
		for (const int & tmpInt : tmpIntList)	{
			if (newStr.length() < 1)
				newStr = QString("%1").arg(tmpInt);
			else	{
				newStr = newStr.append(", %1").arg(tmpInt);
			}
			newArray.append(QJsonValue(tmpInt));
		}
		if (newStr != tmpStr)
			ui->valuesField->setText(newStr);
		//	update the input ref, export the file
		if (newArray.size() == 0)
			_input->setValue("VALUES", QJsonValue::Undefined);
		else
			_input->setValue("VALUES", newArray);
		RecreateJSONAndExport();
	});
	//	labels field
	QObject::disconnect(ui->labelsField, 0, 0, 0);
	QObject::connect(ui->labelsField, &QLineEdit::editingFinished, [&]()	{
		//	get text as string, break up to array of strings, convert to QJsonArray of strs
		QString			tmpStr = ui->labelsField->text();
		QStringList		tmpStrList = tmpStr.split(QRegularExpression("[^\\w]+"));
		if (tmpStrList.size() == 0)
			_input->setValue("LABELS", QJsonValue::Undefined);
		else
			_input->setValue("LABELS", QJsonArray::fromStringList(tmpStrList));
		RecreateJSONAndExport();
	});
}
void JSONGUIInputLongWidget::refreshUIItems() {
	//	have my super refresh the UI items common to all of these
	refreshInputNameEdit( (ui->inputNameEdit) );
	refreshLabelField( (ui->labelField) );
	refreshTypeCBox( (ui->typePUB) );
	prepareDeleteLabel( (ui->deleteLabel) );
	
	//	refresh the UI items specific to this input
	
	QJsonValue		tmpVal;
	//	default field
	tmpVal = _input->value("DEFAULT");
	if (!tmpVal.isDouble())
		ui->defaultField->setText("");
	else
		ui->defaultField->setText( QString("%1").arg(tmpVal.toInt()) );
	//	identity field
	tmpVal = _input->value("IDENTITY");
	if (!tmpVal.isDouble())
		ui->identityField->setText("");
	else
		ui->identityField->setText( QString("%1").arg(tmpVal.toInt()) );
	//	values field
	tmpVal = _input->value("VALUES");
	if (!tmpVal.isArray())
		ui->valuesField->setText("");
	else	{
		QVariantList		tmpList = tmpVal.toArray().toVariantList();
		QStringList			newList;
		for (const QVariant & tmpVar : tmpList)	{
			newList.append( QString("%1").arg(tmpVar.toInt()) );
		}
		ui->valuesField->setText( newList.join(", ") );
	}
	//	labels field
	tmpVal = _input->value("LABELS");
	if (!tmpVal.isArray())
		ui->labelsField->setText("");
	else	{
		QVariantList		tmpList = tmpVal.toArray().toVariantList();
		QStringList			newList;
		for (const QVariant & tmpVar : tmpList)	{
			newList.append( tmpVar.toString() );
		}
		QString				newStr = newList.join(", ");
		ui->labelsField->setText(newStr);
	}
}
