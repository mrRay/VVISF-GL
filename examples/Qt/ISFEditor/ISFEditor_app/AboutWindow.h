#ifndef ABOUTWINDOW_H
#define ABOUTWINDOW_H

#include <QDialog>




namespace Ui {
	class AboutWindow;
}




class AboutWindow : public QDialog
{
	Q_OBJECT

public:
	explicit AboutWindow(QWidget *parent = nullptr);
	~AboutWindow();
	
protected:
	virtual void showEvent(QShowEvent * event) Q_DECL_OVERRIDE;

private:
	Ui::AboutWindow		*ui;
};




AboutWindow * GetAboutWindow();




#endif // ABOUTWINDOW_H
