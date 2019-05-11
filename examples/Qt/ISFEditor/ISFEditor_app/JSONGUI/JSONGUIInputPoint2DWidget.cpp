#include "JSONGUIInputPoint2DWidget.h"
#include "ui_JSONGUIInputPoint2DWidget.h"




JSONGUIInputPoint2DWidget::JSONGUIInputPoint2DWidget(const JGMInputRef & inRef, JSONScrollWidget * inScrollWidget, QWidget *parent) :
	JSONGUIInputWidget(inRef, inScrollWidget, parent),
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
void JSONGUIInputPoint2DWidget::prepareToBeDeleted()	{
	QObject::disconnect((ui->defaultField), 0, 0, 0);
	QObject::disconnect((ui->minField), 0, 0, 0);
	QObject::disconnect((ui->maxField), 0, 0, 0);
	QObject::disconnect((ui->identityField), 0, 0, 0);
	
	QObject::disconnect((ui->dragLabel), 0, 0, 0);
	QObject::disconnect((ui->inputNameEdit), 0, 0, 0);
	QObject::disconnect((ui->labelField), 0, 0, 0);
	QObject::disconnect((ui->typePUB), 0, 0, 0);
	QObject::disconnect((ui->deleteLabel), 0, 0, 0);
}




void JSONGUIInputPoint2DWidget::prepareUIItems() {
	//	have my super prepare the UI items common to all of these
	prepareDragLabel( (ui->dragLabel) );
	prepareInputNameEdit( (ui->inputNameEdit) );
	prepareLabelField( (ui->labelField) );
	prepareTypeCBox( (ui->typePUB) );
	prepareDeleteLabel( (ui->deleteLabel) );
	
	//	prepare the UI items specific to this input
	
	//	default
	QObject::disconnect((ui->defaultField), 0, 0, 0);
	QObject::connect(ui->defaultField, &QLineEdit::editingFinished, [&]()	{
		//	get text as string, break up to array of strings
		QString			tmpStr = ui->defaultField->text();
		QStringList		tmpStrList = tmpStr.split(QRegularExpression("[^-0-9\\.]+"));
		//	run through array of strings, creating an array of doubles
		QVariantList		tmpDoubleList;
		for (const QString & listStr : tmpStrList)	{
			if (listStr.length() < 1)
				continue;
			tmpDoubleList.append( QVariant(listStr.toDouble()) );
			if (tmpDoubleList.length() == 2)
				break;
		}
		if (tmpDoubleList.length() != 2 && tmpDoubleList.length() != 0)	{
			qDebug() << "default tmpDoubleList is " << tmpDoubleList.size() << ", " << tmpDoubleList;
			refreshUIItems();
			return;
		}
		//	update the input ref, export the file
		if (tmpDoubleList.size() == 0)
			_input->setValue("DEFAULT", QJsonValue::Undefined);
		else
			_input->setValue("DEFAULT", QJsonArray::fromVariantList(tmpDoubleList));
		RecreateJSONAndExport();
	});
	//	identity
	QObject::disconnect((ui->identityField), 0, 0, 0);
	QObject::connect(ui->identityField, &QLineEdit::editingFinished, [&]()	{
		//	get text as string, break up to array of strings
		QString			tmpStr = ui->identityField->text();
		QStringList		tmpStrList = tmpStr.split(QRegularExpression("[^-0-9\\.]+"));
		//	run through array of strings, creating an array of doubles
		QVariantList		tmpDoubleList;
		for (const QString & listStr : tmpStrList)	{
			if (listStr.length() < 1)
				continue;
			tmpDoubleList.append( QVariant(listStr.toDouble()) );
			if (tmpDoubleList.length() == 2)
				break;
		}
		if (tmpDoubleList.length() != 2 && tmpDoubleList.length() != 0)	{
			refreshUIItems();
			return;
		}
		//	update the input ref, export the file
		if (tmpDoubleList.size() == 0)
			_input->setValue("IDENTITY", QJsonValue::Undefined);
		else
			_input->setValue("IDENTITY", QJsonArray::fromVariantList(tmpDoubleList));
		RecreateJSONAndExport();
	});
	//	min
	QObject::disconnect((ui->minField), 0, 0, 0);
	QObject::connect(ui->minField, &QLineEdit::editingFinished, [&]()	{
		//	get text as string, break up to array of strings
		QString			tmpStr = ui->minField->text();
		QStringList		tmpStrList = tmpStr.split(QRegularExpression("[^-0-9\\.]+"));
		//	run through array of strings, creating an array of doubles
		QVariantList		tmpDoubleList;
		for (const QString & listStr : tmpStrList)	{
			if (listStr.length() < 1)
				continue;
			tmpDoubleList.append( QVariant(listStr.toDouble()) );
			if (tmpDoubleList.length() == 2)
				break;
		}
		if (tmpDoubleList.length() != 2 && tmpDoubleList.length() != 0)	{
			refreshUIItems();
			return;
		}
		//	update the input ref, export the file
		if (tmpDoubleList.size() == 0)
			_input->setValue("MIN", QJsonValue::Undefined);
		else
			_input->setValue("MIN", QJsonArray::fromVariantList(tmpDoubleList));
		RecreateJSONAndExport();
	});
	//	max
	QObject::disconnect((ui->maxField), 0, 0, 0);
	QObject::connect(ui->maxField, &QLineEdit::editingFinished, [&]()	{
		//	get text as string, break up to array of strings
		QString			tmpStr = ui->maxField->text();
		QStringList		tmpStrList = tmpStr.split(QRegularExpression("[^-0-9\\.]+"));
		//	run through array of strings, creating an array of doubles
		QVariantList		tmpDoubleList;
		for (const QString & listStr : tmpStrList)	{
			if (listStr.length() < 1)
				continue;
			tmpDoubleList.append( QVariant(listStr.toDouble()) );
			if (tmpDoubleList.length() == 2)
				break;
		}
		if (tmpDoubleList.length() != 2 && tmpDoubleList.length() != 0)	{
			refreshUIItems();
			return;
		}
		//	update the input ref, export the file
		if (tmpDoubleList.size() == 0)
			_input->setValue("MAX", QJsonValue::Undefined);
		else
			_input->setValue("MAX", QJsonArray::fromVariantList(tmpDoubleList));
		RecreateJSONAndExport();
	});
}
void JSONGUIInputPoint2DWidget::refreshUIItems() {
	//	have my super refresh the UI items common to all of these
	refreshInputNameEdit( (ui->inputNameEdit) );
	refreshLabelField( (ui->labelField) );
	refreshTypeCBox( (ui->typePUB) );
	prepareDeleteLabel( (ui->deleteLabel) );
	
	//	refresh the UI items specific to this input
	
	QJsonValue		tmpArrayVal;
	QJsonArray		tmpArray;
	//	default
	tmpArrayVal = _input->value("DEFAULT");
	tmpArray = tmpArrayVal.toArray();
	if (!tmpArrayVal.isArray() || tmpArray.size()!=2)
		ui->defaultField->setText("");
	else	{
		QStringList		tmpList;
		for (const QJsonValue & tmpVal : tmpArray)	{
			tmpList.append( QString("%1").arg(tmpVal.toDouble()) );
		}
		ui->defaultField->setText( tmpList.join(", ") );
	}
	//	identity
	tmpArrayVal = _input->value("IDENTITY");
	tmpArray = tmpArrayVal.toArray();
	if (!tmpArrayVal.isArray() || tmpArray.size()!=2)
		ui->identityField->setText("");
	else	{
		QStringList		tmpList;
		for (const QJsonValue & tmpVal : tmpArray)	{
			tmpList.append( QString("%1").arg(tmpVal.toDouble()) );
		}
		ui->identityField->setText( tmpList.join(", ") );
	}
	//	min
	tmpArrayVal = _input->value("MIN");
	tmpArray = tmpArrayVal.toArray();
	if (!tmpArrayVal.isArray() || tmpArray.size()!=2)
		ui->minField->setText("");
	else	{
		QStringList		tmpList;
		for (const QJsonValue & tmpVal : tmpArray)	{
			tmpList.append( QString("%1").arg(tmpVal.toDouble()) );
		}
		ui->minField->setText( tmpList.join(", ") );
	}
	//	max
	tmpArrayVal = _input->value("MAX");
	tmpArray = tmpArrayVal.toArray();
	if (!tmpArrayVal.isArray() || tmpArray.size()!=2)
		ui->maxField->setText("");
	else	{
		QStringList		tmpList;
		for (const QJsonValue & tmpVal : tmpArray)	{
			tmpList.append( QString("%1").arg(tmpVal.toDouble()) );
		}
		ui->maxField->setText( tmpList.join(", ") );
	}
}



