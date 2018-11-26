#include "FindDialog.h"
#include "ui_FindDialog.h"

#include <QDebug>
#include <QRegularExpression>
#include <QMessageBox>




namespace SimpleSourceCodeEdit	{




FindDialog::FindDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::FindDialog)
{
	ui->setupUi(this);
	//setWindowModality(Qt::ApplicationModal);
	setWindowModality(Qt::WindowModal);
}
FindDialog::FindDialog(const FindOpts & f, QWidget * parent) :
	QDialog(parent),
	ui(new Ui::FindDialog)
{
	ui->setupUi(this);
	//setWindowModality(Qt::ApplicationModal);
	setWindowModality(Qt::WindowModal);
	
	_findOpts = f;
	
	pushOptsToUI();
}
FindDialog::~FindDialog()
{
	delete ui;
}




void FindDialog::cancelClicked()	{
	//qDebug() << __PRETTY_FUNCTION__;
	pushUIToOpts();
	done(1);
}
void FindDialog::searchClicked()	{
	//qDebug() << __PRETTY_FUNCTION__;
	
	//	make sure that the search string is "valid" (more than 0 chars long)
	QString		tmpString = ui->findEdit->text();
	if (tmpString.length() < 1)	{
		QMessageBox::warning(this, "", QString("Please enter a valid search string!"), QMessageBox::Ok);
		return;
	}
	
	//	if regex is enabled, we need to make sure that the regex pattern is valid
	if (ui->regexCBox->isChecked())	{
		QRegularExpression		tmpExpr(tmpString);
		if (!tmpExpr.isValid())	{
			QMessageBox::warning(this, "", QString("Sorry, but there's an error with your regex string:\n(%1)").arg(tmpExpr.errorString()), QMessageBox::Ok);
			return;
		}
	}
	
	//	...if we're here, then the search parameters are valid and we're ready to close the dialog and proceed
	
	pushUIToOpts();
	done(0);
}




void FindDialog::pushUIToOpts()	{
	_findOpts.searchString = ui->findEdit->text();
	_findOpts.caseSensitive = ui->caseSensitiveCBox->isChecked();
	_findOpts.entireWord = ui->entireWordCBox->isChecked();
	_findOpts.regex = ui->regexCBox->isChecked();
}
void FindDialog::pushOptsToUI()	{
	ui->findEdit->setText(_findOpts.searchString);
	ui->caseSensitiveCBox->setChecked(_findOpts.caseSensitive);
	ui->entireWordCBox->setChecked(_findOpts.entireWord);
	ui->regexCBox->setChecked(_findOpts.regex);
}




}	//	namespace SimpleSourceCodeEdit
