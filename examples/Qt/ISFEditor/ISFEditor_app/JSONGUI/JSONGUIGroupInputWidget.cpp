#include "JSONGUIGroupInputWidget.h"
#include "ui_JSONGUIGroupInputWidget.h"

#include "JSONScrollWidget.h"




JSONGUIGroupInputWidget::JSONGUIGroupInputWidget(const JGMTopRef & inTop, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::JSONGUIGroupInput)
{
	ui->setupUi(this);
	
	_top = inTop;
	
	QPalette			p = palette();
	p.setColor(QPalette::Background, p.color(QPalette::Background).darker(110));
	setAutoFillBackground(true);
	setPalette(p);
	
	connect(ui->pushButton, &QAbstractButton::clicked, this, &JSONGUIGroupInputWidget::newInputClicked);
}
JSONGUIGroupInputWidget::~JSONGUIGroupInputWidget()
{
	delete ui;
}
void JSONGUIGroupInputWidget::prepareToBeDeleted()	{
	QObject::disconnect(ui->pushButton, 0, 0, 0);
}




void JSONGUIGroupInputWidget::newInputClicked()	{
	//	if there's no top or the top doesn't have a JGMCInputArray, bail immediately
	if (_top == nullptr)
		return;
	JGMCInputArray		&inputs = _top->inputsContainer();
	//	tell the top object to create a new (unique) input name
	QString			uniqueInputName = _top->createNewInputName();
	//	make a QJsonObject that describes a float-type input with the new input name
	QJsonObject		tmpDict;
	tmpDict["NAME"] = QJsonValue(uniqueInputName);
	tmpDict["TYPE"] = QJsonValue(QString("float"));
	//	make an input-type JGMInputRef from the QJsonObject
	JGMInputRef		tmpInput(new JGMInput(tmpDict, &(*_top)));
	if (tmpInput == nullptr)
		return;
	//	add the new input ref we created to the inputs ref
	inputs.contents().append(tmpInput);
	//	tell the global scroll widget that it has to recreate the JSON and export
	RecreateJSONAndExport();
}
