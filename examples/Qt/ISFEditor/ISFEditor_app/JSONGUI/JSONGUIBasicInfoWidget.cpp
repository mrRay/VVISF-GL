#include "JSONGUIBasicInfoWidget.h"
#include "ui_JSONGUIBasicInfoWidget.h"

#include <QDebug>
#include <QLayout>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>

#include "JSONScrollWidget.h"




using namespace std;




JSONGUIBasicInfoWidget::JSONGUIBasicInfoWidget(const JGMTopRef & inTop, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::JSONGUIBasicInfo),
	_top(inTop)
{
	ui->setupUi(this);
	
	//doc = inDoc;
	
	QPalette			p = palette();
	p.setColor(QPalette::Background, p.color(QPalette::Background).darker(110));
	setAutoFillBackground(true);
	setPalette(p);
	
	QJsonObject			defObj;
	QJsonObject			&isfDict = (_top==nullptr) ? defObj : _top->isfDict();
	QJsonValue			tmpVal;
	QString				tmpString;
	
	if (isfDict.contains("DESCRIPTION"))	{
		tmpVal = isfDict["DESCRIPTION"];
		tmpString = (tmpVal.isString()) ? tmpVal.toString() : QString("");
	}
	else
		tmpString = QString("");
	ui->descriptionEdit->setText(tmpString);
	
	if (isfDict.contains("CREDIT"))	{
		tmpVal = isfDict["CREDIT"];
		tmpString = (tmpVal.isString()) ? tmpVal.toString() : QString("");
	}
	else
		tmpString = QString("");
	ui->creditEdit->setText(tmpString);
	
	tmpString = QString("");
	if (isfDict.contains("CATEGORIES"))	{
		tmpVal = isfDict["CATEGORIES"];
		if (tmpVal.isArray())	{
			for (const auto & catVal : tmpVal.toArray())	{
				if (!catVal.isString())
					continue;
				if (tmpString.length() < 1)
					tmpString = catVal.toString();
				else
					tmpString.append( QString(", %1").arg(catVal.toString()) );
			}
		}
	}
	else
		tmpString = QString("");
	ui->categoriesEdit->setText(tmpString);
	
	if (isfDict.contains("VSN"))	{
		tmpVal = isfDict["VSN"];
		tmpString = (tmpVal.isString()) ? tmpVal.toString() : QString("");
	}
	else
		tmpString = QString("");
	ui->vsnEdit->setText(tmpString);
	
	connect(ui->descriptionEdit, &QLineEdit::editingFinished, this, &JSONGUIBasicInfoWidget::descriptionFieldUsed);
	connect(ui->creditEdit, &QLineEdit::editingFinished, this, &JSONGUIBasicInfoWidget::creditFieldUsed);
	connect(ui->categoriesEdit, &QLineEdit::editingFinished, this, &JSONGUIBasicInfoWidget::categoriesFieldUsed);
	connect(ui->vsnEdit, &QLineEdit::editingFinished, this, &JSONGUIBasicInfoWidget::vsnFieldUsed);
}
JSONGUIBasicInfoWidget::~JSONGUIBasicInfoWidget()
{
	delete ui;
}
void JSONGUIBasicInfoWidget::prepareToBeDeleted()	{
	QObject::disconnect(ui->descriptionEdit, 0, 0, 0);
	QObject::disconnect(ui->creditEdit, 0, 0, 0);
	QObject::disconnect(ui->categoriesEdit, 0, 0, 0);
	QObject::disconnect(ui->vsnEdit, 0, 0, 0);
}




void JSONGUIBasicInfoWidget::descriptionFieldUsed()	{
	qDebug() << __PRETTY_FUNCTION__;
	if (_top == nullptr)
		return;
	QJsonObject			&isfDict = _top->isfDict();
	isfDict["DESCRIPTION"] = QJsonValue(ui->descriptionEdit->text());
	
	//	tell the global scroll widget that it has to recreate the JSON and export
	RecreateJSONAndExport();
}
void JSONGUIBasicInfoWidget::creditFieldUsed()	{
	qDebug() << __PRETTY_FUNCTION__;
	if (_top == nullptr)
		return;
	QJsonObject			&isfDict = _top->isfDict();
	isfDict["CREDIT"] = QJsonValue(ui->creditEdit->text());
	
	//	tell the global scroll widget that it has to recreate the JSON and export
	RecreateJSONAndExport();
}
void JSONGUIBasicInfoWidget::categoriesFieldUsed()	{
	qDebug() << __PRETTY_FUNCTION__;
	if (_top == nullptr)
		return;
	
	QStringList			catList = ui->categoriesEdit->text().split(",", QString::SkipEmptyParts);
	QJsonArray			catArray;
	for (const auto cat : catList)	{
		catArray.append( QJsonValue( cat.trimmed() ) );
	}
	
	QJsonObject			&isfDict = _top->isfDict();
	if (catArray.size() < 1)
		isfDict.remove("CATEGORIES");
	else
		isfDict["CATEGORIES"] = QJsonValue(catArray);
	
	//	tell the global scroll widget that it has to recreate the JSON and export
	RecreateJSONAndExport();
}
void JSONGUIBasicInfoWidget::vsnFieldUsed()	{
	qDebug() << __PRETTY_FUNCTION__;
	if (_top == nullptr)
		return;
	QJsonObject			&isfDict = _top->isfDict();
	isfDict["VSN"] = ui->vsnEdit->text();
	
	//	tell the global scroll widget that it has to recreate the JSON and export
	RecreateJSONAndExport();
}

