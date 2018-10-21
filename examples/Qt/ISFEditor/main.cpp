#include "MainWindow.h"
#include <QApplication>
#include <QDebug>

#include "LoadingWindow.h"
#include "OutputWindow.h"
#include "DocWindow.h"
#include "DynamicVideoSource.h"
#include "ISFController.h"



void LaunchingBufferPoolCheck()	{
	qDebug() << __PRETTY_FUNCTION__;
	
	//	if there's no global buffer pool, wait a bit and call this block again
	if (GetGlobalBufferPool() == nullptr)	{
		QThread		*mainThread = QApplication::instance()->thread();
		QTimer		*tmpTimer = new QTimer();
		tmpTimer->setInterval(50);
		tmpTimer->setSingleShot(true);
		tmpTimer->moveToThread(mainThread);
		QObject::connect(tmpTimer, &QTimer::timeout, [&]()	{
			LaunchingBufferPoolCheck();
		});
		tmpTimer->start();
		qDebug() << "no buffer pool, checking again on thread " << mainThread;
	}
	//	else there's a global buffer pool- we can proceed with loading!
	else	{
		qDebug() << "buffer pool found- proceeding with app launch";
		//	tell the loading window to create a new file
		GetLoadingWindow()->on_createNewFile();
		//	open the loading window- this seems to cause the longest delay (populating the list of cameras is done here)
		GetLoadingWindow()->show();
	}
}

int main(int argc, char *argv[])
{
	//	we want all the widgets to share contexts, and we need to make a widget to get that shard context (there's no way to tell a widget to use a given context)
	QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts, true);
	QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL, true);
	//QCoreApplication::setAttribute(Qt::AA_DontCheckOpenGLContextThreadAffinity, true);
	
	//	let's set up some basic info about the app so QSettings will be easier to use
	QCoreApplication::setOrganizationName("yourcompanyname");
	QCoreApplication::setOrganizationDomain("com.yourcompanyname");
	QCoreApplication::setApplicationName("ISFEditor");
	
	QSurfaceFormat::setDefaultFormat(CreateCompatibilityGLSurfaceFormat());
	//QSurfaceFormat::setDefaultFormat(CreateGL4SurfaceFormat());
	
	QApplication a(argc, argv);
	//QThread		*firstThread = a.thread();
	QThread			*firstThread = QApplication::instance()->thread();
	if (firstThread == nullptr)
		qDebug() << "firstThread is null on launch";
	else
		qDebug() << "there's a firstThread: " << firstThread;
	
	
	MainWindow w;
	w.show();
	w.hide();
	
	DynamicVideoSource		dvs;
	OutputWindow		ow;
	ow.show();
	
	//	ISF controller has to be created after OutputWindow (it connects to a signal from the window)
	GetISFController();
	
	DocWindow			dw;
	dw.show();
	
	LoadingWindow		lw;
	//lw.show();	//	NO, not yet- this loads webcam stuff, which causes problems right now
	//lw.on_createNewFile();	//	NO, not yet- there's no GL buffer pool, so the scene can't compile anything
	
	
	LaunchingBufferPoolCheck();

	return a.exec();
}
