#include "ReportProblemDialog.h"
#include "ui_ReportProblemDialog.h"




static ReportProblemDialog			*_globalReportProblemDialog = nullptr;




ReportProblemDialog::ReportProblemDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::ReportProblemDialog)
{
	ui->setupUi(this);
	setModal(true);
}

ReportProblemDialog::~ReportProblemDialog()
{
	delete ui;
}








ReportProblemDialog * GetReportProblemDialog()	{
	if (_globalReportProblemDialog == nullptr)
		_globalReportProblemDialog = new ReportProblemDialog();
	return _globalReportProblemDialog;
}
