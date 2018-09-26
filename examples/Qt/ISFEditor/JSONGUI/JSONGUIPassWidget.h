#ifndef JSONGUIPASS_H
#define JSONGUIPASS_H

#include <QWidget>




namespace Ui {
	class JSONGUIPass;
}




class JSONGUIPassWidget : public QWidget
{
	Q_OBJECT

public:
	explicit JSONGUIPassWidget(QWidget *parent = nullptr);
	~JSONGUIPassWidget();

private:
	Ui::JSONGUIPass *ui;
};




#endif // JSONGUIPASS_H
