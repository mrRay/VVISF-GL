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
	void widgetDrewItsFirstFrame();

private:
	Ui::MainWindow *ui;
};




//	gets the global singleton for this class
MainWindow * GetMainWindow();

//	this function is called when the GL environment has been created and the app is ready to finish launching
void FinishLaunching();




#endif // MAINWINDOW_H
