#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QDebug>

#include "GLBufferQWidget.h"
#include "ISFGLBufferQWidget.h"
#include "AudioController.h"
#include "OutputWindow.h"
#include "DynamicVideoSource.h"
#include "ISFController.h"
#include "DocWindow.h"
#include "LoadingWindow.h"
#include "GLSLSandboxConverter.h"
#include "ShadertoyConverter.h"




static MainWindow * globalMainWindow = nullptr;




MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	//qDebug() << __PRETTY_FUNCTION__;
	
	globalMainWindow = this;

	ui->setupUi(this);
	
	//	we want to know when the widget draws its first frame because we can't create the global shared context/buffer pool until we can get a base ctx from the widget
	connect(ui->bufferView, SIGNAL(frameSwapped()), this, SLOT(widgetDrewItsFirstFrame()));
}

MainWindow::~MainWindow()	{
	delete ui;
}

GLBufferQWidget * MainWindow::bufferView()	{
	return ui->bufferView;
}

void MainWindow::on_actionNew_triggered()	{
	qDebug() << __PRETTY_FUNCTION__;
	LoadingWindow		*lw = GetLoadingWindow();
	if (lw != nullptr)
		lw->on_createNewFile();
}

void MainWindow::on_actionOpen_triggered()	{
	qDebug() << __PRETTY_FUNCTION__;
}

void MainWindow::on_actionSave_triggered()	{
	//qDebug() << __PRETTY_FUNCTION__;
	DocWindow		*dw = GetDocWindow();
	if (dw != nullptr)
		dw->saveOpenFile();
}

void MainWindow::on_actionImport_from_GLSLSandbox_triggered()	{
	qDebug() << __PRETTY_FUNCTION__;
	
	GLSLSandboxConverter		*conv = new GLSLSandboxConverter(GetLoadingWindow());
	int				returnCode = conv->exec();
	qDebug() << "returnCode is " << returnCode;
	if (!returnCode)	{
		LoadingWindow		*lw = GetLoadingWindow();
		if (lw != nullptr)	{
			lw->finishedConversionDisplayFile(conv->exportedISFPath());
		}
	}
	delete conv;
}

void MainWindow::on_actionImport_from_Shadertoy_triggered()	{
	qDebug() << __PRETTY_FUNCTION__;
	
	ShadertoyConverter		*conv = new ShadertoyConverter(GetLoadingWindow());
	int				returnCode = conv->exec();
	qDebug() << "returnCode is " << returnCode;
	if (!returnCode)	{
		LoadingWindow		*lw = GetLoadingWindow();
		if (lw != nullptr)	{
			lw->finishedConversionDisplayFile(conv->exportedISFPath());
		}
	}
	delete conv;
}

void MainWindow::on_actionQuit_triggered()	{
	qDebug() << __PRETTY_FUNCTION__;
	QCoreApplication::quit();
}


void MainWindow::widgetDrewItsFirstFrame()	{
	//qDebug() << __PRETTY_FUNCTION__;
	
	//	get the widget's context- if it's null, the widget's context doesn't exist yet and this method shouldn't have been called!
	GLContextRef		widgetCtx = ui->bufferView->glContextRef();
	if (widgetCtx == nullptr)
		return;
	
	//cout << "\tversion is " << GLVersionToString(widgetCtx->version) << endl;
	
	//	disconnect immediately- we're only doing this because we need to create the shared context from the widget's context
	disconnect(ui->bufferView, SIGNAL(frameSwapped()), this, SLOT(widgetDrewItsFirstFrame()));
	
	//	make the global buffer pool, using a newly-created context that shares the base global context
	CreateGlobalBufferPool(widgetCtx->newContextSharingMe());
	
	//	don't tell the widget to start rendering- doing so will cause it to start rendering at 60fps
	//ui->bufferView->startRendering();
	
	//	tell the widget to draw a single frame.  for some reason, GL widgets on os x don't have their internal sizes set properly when they draw their first frame.
	ui->bufferView->drawBuffer(nullptr);
	
	//	hide myself, i don't need to be visible any more
	hide();
	
	//	finish launching
	FinishLaunching();
}




void FinishLaunching()	{
	//qDebug() << __PRETTY_FUNCTION__;
	
	DynamicVideoSource		*dvs = new DynamicVideoSource();
	Q_UNUSED(dvs);
	
	GetAudioController();
	
	OutputWindow		*ow = new OutputWindow();;
	
	//	ISF controller has to be created after OutputWindow (it connects to a signal from the window)
	GetISFController();
	
	DocWindow			*dw = new DocWindow();;
	
	LoadingWindow		*lw = new LoadingWindow();
	
	dw->show();
	lw->show();
	lw->on_createNewFile();
	
	ow->show();
	
}




void MainWindow::on_actionFind_triggered()	{
	DocWindow		*dw = GetDocWindow();
	if (dw == nullptr)
		return;
	dw->on_actionFind_triggered();
}
void MainWindow::on_actionFind_Previous_triggered()	{
	DocWindow		*dw = GetDocWindow();
	if (dw == nullptr)
		return;
	dw->on_actionFind_Previous_triggered();
}
void MainWindow::on_actionFind_Next_triggered()	{
	DocWindow		*dw = GetDocWindow();
	if (dw == nullptr)
		return;
	dw->on_actionFind_Next_triggered();
}
void MainWindow::on_actionUse_selection_for_search_triggered()	{
	DocWindow		*dw = GetDocWindow();
	if (dw == nullptr)
		return;
	dw->on_actionUse_selection_for_search_triggered();
}




MainWindow * GetMainWindow()	{
	return globalMainWindow;
}

