#ifndef JSONGUIGROUPINPUT_H
#define JSONGUIGROUPINPUT_H

#include <QWidget>
#include "JGMTop.h"




namespace Ui {
	class JSONGUIGroupInput;
}




class JSONGUIGroupInputWidget : public QWidget
{
	Q_OBJECT

public:
	explicit JSONGUIGroupInputWidget(const JGMTopRef & inTop, QWidget *parent = nullptr);
	~JSONGUIGroupInputWidget();
	
	void prepareToBeDeleted();

public slots:
	void newInputClicked();

private:
	Ui::JSONGUIGroupInput *ui;
	JGMTopRef			_top = nullptr;
};




#endif // JSONGUIGROUPINPUT_H
