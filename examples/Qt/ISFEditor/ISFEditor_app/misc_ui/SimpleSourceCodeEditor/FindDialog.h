#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <QDialog>

#include "FindOpts.h"




namespace Ui {
	class FindDialog;
}




namespace SimpleSourceCodeEdit	{




class FindDialog : public QDialog
{
	Q_OBJECT

public:
	explicit FindDialog(QWidget *parent = nullptr);
	explicit FindDialog(const FindOpts & f, QWidget * parent=nullptr);
	~FindDialog();
	
	FindOpts findOpts() { return _findOpts; }

private slots:
	Q_SLOT void cancelClicked();
	Q_SLOT void searchClicked();

private:
	Ui::FindDialog		*ui;
	FindOpts			_findOpts;
	
	void pushUIToOpts();
	void pushOptsToUI();
};




}	//	namespace SimpleSourceCodeEdit




#endif // FINDDIALOG_H
