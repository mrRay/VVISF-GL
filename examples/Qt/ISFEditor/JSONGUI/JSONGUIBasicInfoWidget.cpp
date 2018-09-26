#include "JSONGUIBasicInfoWidget.h"
#include "ui_JSONGUIBasicInfo.h"

#include <QDebug>
#include <QLayout>




using namespace std;




JSONGUIBasicInfoWidget::JSONGUIBasicInfoWidget(const VVISF::ISFDocRef & inDoc, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::JSONGUIBasicInfo)
{
	ui->setupUi(this);
	
	doc = inDoc;
	
	QPalette			p = palette();
	p.setColor(QPalette::Background, p.color(QPalette::Background).darker(110));
	setAutoFillBackground(true);
	setPalette(p);
	
	if (doc != nullptr)	{
		QString				tmpString;
		
		tmpString = QString::fromStdString(doc->getDescription());
		ui->descriptionEdit->setText(tmpString);
		
		tmpString = QString::fromStdString(doc->getCredit());
		ui->creditEdit->setText(tmpString);
		
		tmpString = QString("");
		for (const string category : doc->getCategories())	{
			if (tmpString.length() < 1)
				tmpString = QString::fromStdString(category);
			else
				tmpString.append( QString(", %1").arg(QString::fromStdString(category)) );
		}
		ui->categoriesEdit->setText(tmpString);
		
		tmpString = QString::fromStdString(doc->getVsn());
		ui->vsnEdit->setText(tmpString);
	}
}

JSONGUIBasicInfoWidget::~JSONGUIBasicInfoWidget()
{
	delete ui;
}
