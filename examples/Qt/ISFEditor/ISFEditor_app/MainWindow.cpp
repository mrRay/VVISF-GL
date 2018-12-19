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
	cout << "global buffer pool is " << GetGlobalBufferPool() << endl;
	
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
	
	ISFController	*isfc = GetISFController();
	VVGLRenderQThread		*renderThread = isfc->renderThread();
	GLBufferPoolRef			renderPool = isfc->renderThreadBufferPool();
	GLTexToTexCopierRef		renderTexCopier = isfc->renderThreadTexCopier();
	
	DynamicVideoSource		*dvs = new DynamicVideoSource();
	//	move the dynamic video source to the render thread, so it loads files on the render thread (use signals to load files)
	dvs->moveToThread(renderThread);
	
	AudioController		*ac = GetAudioController();
	if (ac != nullptr)
		ac->moveToThread(renderThread, renderPool, renderTexCopier);
	
	OutputWindow		*ow = new OutputWindow();
	
	DocWindow			*dw = new DocWindow();;
	
	LoadingWindow		*lw = new LoadingWindow();
	
	dw->show();
	lw->show();
	lw->on_createNewFile();
	
	
	//	connect the output window's buffer view's signal to the ISFController's redraw slot
	ISFGLBufferQWidget		*bufferView = (ow==nullptr) ? nullptr : ow->bufferView();
	if (bufferView == nullptr)
		qDebug() << "ERR: bufferView nil in " << __PRETTY_FUNCTION__;
	else	{
		QObject::connect(bufferView, &QOpenGLWidget::frameSwapped, GetISFController(), &ISFController::widgetRedrawSlot);
	}
	
	
	ow->show();
	
}




MainWindow * GetMainWindow()	{
	return globalMainWindow;
}

