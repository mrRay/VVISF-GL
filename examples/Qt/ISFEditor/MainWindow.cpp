#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QDebug>

#include "LoadingWindow.h"
#include "DocWindow.h"




MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::on_actionNew_triggered()	{
	qDebug() << __PRETTY_FUNCTION__;
	GetLoadingWindow()->on_createNewFile();
}

void MainWindow::on_actionOpen_triggered()	{
	qDebug() << __PRETTY_FUNCTION__;
}

void MainWindow::on_actionSave_triggered()	{
	//qDebug() << __PRETTY_FUNCTION__;
	GetDocWindow()->saveOpenFile();
}
