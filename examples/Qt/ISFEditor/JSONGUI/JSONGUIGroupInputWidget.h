#ifndef JSONGUIGROUPINPUT_H
#define JSONGUIGROUPINPUT_H

#include <QWidget>




namespace Ui {
	class JSONGUIGroupInput;
}




class JSONGUIGroupInputWidget : public QWidget
{
	Q_OBJECT

public:
	explicit JSONGUIGroupInputWidget(QWidget *parent = nullptr);
	~JSONGUIGroupInputWidget();

private:
	Ui::JSONGUIGroupInput *ui;
};




#endif // JSONGUIGROUPINPUT_H
