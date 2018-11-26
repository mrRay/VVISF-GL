#ifndef OUTPUTWINDOW_H
#define OUTPUTWINDOW_H

#include <QWidget>

#include "ISFGLBufferQWidget.h"
#include <VVGL.hpp>
#include <VVISF.hpp>

#include "InterAppOutput.h"




namespace Ui {
	class OutputWindow;
}




class OutputWindow : public QWidget
{
	Q_OBJECT

public:
	explicit OutputWindow(QWidget *parent = nullptr);
	~OutputWindow();
	
	ISFGLBufferQWidget * bufferView();
	void drawBuffer(const VVGL::GLBufferRef & n);
	void updateContentsFromISFController();
	int selectedIndexToDisplay();
	bool getFreezeOutputFlag() { return freezeOutputFlag; }
	
signals:
	Q_SIGNAL void outputWindowMouseMoved(VVGL::Point normMouseEventLoc, VVGL::Point absMouseEventLoc);
	
protected:
	void closeEvent(QCloseEvent * event);
	void showEvent(QShowEvent * event);

private slots:
	void on_freezeOutputToggle_stateChanged(int arg1);
	void on_displayAlphaToggle_stateChanged(int arg1);
	//void widgetDrewItsFirstFrame();
	void aboutToQuit();

private:
	Ui::OutputWindow			*ui;
	bool			freezeOutputFlag = false;
	InterAppOutput		interAppOutput;
};




//	gets the global singleton for this class, which is created in main()
OutputWindow * GetOutputWindow();




#endif // OUTPUTWINDOW_H
