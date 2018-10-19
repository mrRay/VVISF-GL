#include "JSONGUIGroupPassWidget.h"
#include "ui_JSONGUIGroupPassWidget.h"

#include "JSONScrollWidget.h"




JSONGUIGroupPassWidget::JSONGUIGroupPassWidget(const JGMTopRef & inTop, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::JSONGUIGroupPass)
{
	ui->setupUi(this);
	
	_top = inTop;
	
	QPalette			p = palette();
	p.setColor(QPalette::Background, p.color(QPalette::Background).darker(110));
	setAutoFillBackground(true);
	setPalette(p);
	
	connect(ui->pushButton, &QAbstractButton::clicked, this, &JSONGUIGroupPassWidget::newInputClicked);
}

JSONGUIGroupPassWidget::~JSONGUIGroupPassWidget()
{
	delete ui;
}


void JSONGUIGroupPassWidget::newInputClicked()	{
	//	if there's a top or the top doesn't have a JGMCPassArray, bail immediately
	if (_top == nullptr)
		return;
	//	make a new pass, add it to the top-level array of passes
	JGMPassRef			newPass(new JGMPass(QJsonObject(), &(*_top)));
	if (newPass == nullptr)
		return;
	JGMCPassArray		&passes = _top->passesContainer();
	passes.contents().append(newPass);
	//	tell the global scroll widget that it has to recreate the JSON and export
	RecreateJSONAndExport();
}
