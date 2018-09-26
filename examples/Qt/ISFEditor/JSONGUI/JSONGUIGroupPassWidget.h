#ifndef JSONGUIGROUPPASS_H
#define JSONGUIGROUPPASS_H

#include <QWidget>




namespace Ui {
	class JSONGUIGroupPass;
}




class JSONGUIGroupPassWidget : public QWidget
{
	Q_OBJECT

public:
	explicit JSONGUIGroupPassWidget(QWidget *parent = nullptr);
	~JSONGUIGroupPassWidget();

private:
	Ui::JSONGUIGroupPass *ui;
};




#endif // JSONGUIGROUPPASS_H
