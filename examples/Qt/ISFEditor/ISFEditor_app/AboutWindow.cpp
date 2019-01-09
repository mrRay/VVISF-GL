#include "AboutWindow.h"
#include "ui_AboutWindow.h"

#include "VVGL.hpp"




static AboutWindow		*_globalAboutWindow = nullptr;




AboutWindow::AboutWindow(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::AboutWindow)
{
	ui->setupUi(this);
	
	//	load the .html license file
	QFile			aboutFile(":/resources/About.html");
	QString			aboutFileString;
	if (aboutFile.open(QFile::ReadOnly))	{
		QTextStream		rStream(&aboutFile);
		aboutFileString = aboutFile.readAll();
		aboutFile.close();
	}
	else
		aboutFileString = QString("");
	
	ui->textEdit->setHtml(aboutFileString);
}

AboutWindow::~AboutWindow()
{
	delete ui;
}

void AboutWindow::showEvent(QShowEvent * event)	{
	qDebug() << __PRETTY_FUNCTION__;
	
	//Q_UNUSED(event);
	QWidget::showEvent(event);
	
	
}





AboutWindow * GetAboutWindow()	{
	if (_globalAboutWindow == nullptr)	{
		_globalAboutWindow = new AboutWindow();
	}
	return _globalAboutWindow;
}
