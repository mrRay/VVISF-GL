#include "OutputWindow.h"
#include "ui_OutputWindow.h"

#include <QDebug>
#include <QSettings>

#include "ISFPassTarget.hpp"




OutputWindow::OutputWindow(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::OutputWindow)
{
	qDebug() << __PRETTY_FUNCTION__;
	
	ui->setupUi(this);
	
	//	we want to know when the widget draws its first frame because we can't create the global shared context/buffer pool until we can get a base ctx from the widget
	connect(ui->bufferView, SIGNAL(frameSwapped()), this, SLOT(widgetDrewItsFirstFrame()));
	//	we need to shut stuff down and delete contexts gracefull on quit
	connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(aboutToQuit()));
	
	//	restore the window position
	QSettings		settings;
	if (settings.contains("OutputWindowGeometry"))	{
		restoreGeometry(settings.value("OutputWindowGeometry").toByteArray());
	}
}

OutputWindow::~OutputWindow()	{
	delete ui;
}

void OutputWindow::closeEvent(QCloseEvent * event)	{
	QSettings		settings;
	settings.setValue("OutputWindowGeometry", saveGeometry());
	
	QWidget::closeEvent(event);
}

void OutputWindow::on_renderPassComboBox_currentIndexChanged(int index)	{

}

void OutputWindow::on_freezeOutputToggle_stateChanged(int arg1)	{

}

void OutputWindow::on_displayAlphaToggle_stateChanged(int arg1)	{

}

void OutputWindow::widgetDrewItsFirstFrame()	{
	qDebug() << __PRETTY_FUNCTION__;
	/*
	if (QThread::currentThread() == qApp->thread())
		qDebug() << "\tcurrent thread in widget draw first frame is main thread";
	else
		qDebug() << "\tcurrent thread in widget draw first frame is NOT main thread!";
	*/
	//	get the widget's context- if it's null, the widget's context doesn't exist yet and this method shouldn't have been called!
	GLContextRef		widgetCtx = ui->bufferView->getContext();
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
}
void OutputWindow::aboutToQuit()	{
	//	we want to clear all the rendering resources- if we don't, Qt will crash on quit, which is harmless but tacky and should be avoided
	
	//	set the global buffer pool to null
	SetGlobalBufferPool(nullptr);
	//	the ISFPassTarget class creates a tex-to-tex copier for copying textures: delete it
	VVISF::ISFPassTarget::cleanup();
	
	//	...other GL resources owned by other classes will be freed using the same signal
}
