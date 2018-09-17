#ifndef OUTPUTWINDOW_H
#define OUTPUTWINDOW_H

#include <QWidget>

#include "GLBufferQWidget.h"
#include <VVGL.hpp>
#include <VVISF.hpp>




namespace Ui {
	class OutputWindow;
}

class OutputWindow : public QWidget
{
	Q_OBJECT

public:
	explicit OutputWindow(QWidget *parent = nullptr);
	~OutputWindow();

private slots:
	void on_renderPassComboBox_currentIndexChanged(int index);
	void on_freezeOutputToggle_stateChanged(int arg1);
	void on_displayAlphaToggle_stateChanged(int arg1);
	void widgetDrewItsFirstFrame();
	void aboutToQuit();

private:
	Ui::OutputWindow *ui;
};

#endif // OUTPUTWINDOW_H
