#ifndef REPORTPROBLEMDIALOG_H
#define REPORTPROBLEMDIALOG_H

#include <QDialog>




namespace Ui {
	class ReportProblemDialog;
}




class ReportProblemDialog : public QDialog
{
	Q_OBJECT

public:
	explicit ReportProblemDialog(QWidget *parent = nullptr);
	~ReportProblemDialog();

private:
	Ui::ReportProblemDialog *ui;
};




ReportProblemDialog * GetReportProblemDialog();




#endif // REPORTPROBLEMDIALOG_H
