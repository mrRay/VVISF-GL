#ifndef JSONGUIGROUPPASS_H
#define JSONGUIGROUPPASS_H

#include <QWidget>
#include "JGMTop.h"




namespace Ui {
	class JSONGUIGroupPass;
}




class JSONGUIGroupPassWidget : public QWidget
{
	Q_OBJECT

public:
	explicit JSONGUIGroupPassWidget(const JGMTopRef & inTop, QWidget *parent = nullptr);
	~JSONGUIGroupPassWidget();

public slots:
	void newInputClicked();

private:
	Ui::JSONGUIGroupPass *ui;
	JGMTopRef			_top = nullptr;
};




#endif // JSONGUIGROUPPASS_H
