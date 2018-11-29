#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>




namespace Ui {
	class MainWindow;
}

class GLBufferQWidget;




class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = nullptr);
	~MainWindow();

	GLBufferQWidget * bufferView();

private slots:
	void on_actionNew_triggered();

	void on_actionOpen_triggered();

	void on_actionSave_triggered();
	
	void on_actionQuit_triggered();
	
	void widgetDrewItsFirstFrame();

	void on_actionImport_from_GLSLSandbox_triggered();

	void on_actionImport_from_Shadertoy_triggered();

	void on_actionFind_triggered();
	void on_actionFind_Previous_triggered();
	void on_actionFind_Next_triggered();
	void on_actionUse_selection_for_search_triggered();

private:
	Ui::MainWindow *ui;
};




//	gets the global singleton for this class
MainWindow * GetMainWindow();

//	this function is called when the GL environment has been created and the app is ready to finish launching
void FinishLaunching();




#endif // MAINWINDOW_H
