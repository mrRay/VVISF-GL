#include "MainWindow.h"
#include <QApplication>

#include "LoadingWindow.h"
#include "OutputWindow.h"
#include "DocWindow.h"
#include "DynamicVideoSource.h"
#include "ISFController.h"




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
	
	DynamicVideoSource		dvs;
	
	MainWindow w;
	w.show();
	w.hide();
	
	OutputWindow		ow;
	ow.show();
	
	//	ISF controller has to be created after OutputWindow (it connects to a signal from the window)
	GetISFController();
	
	LoadingWindow		lw;
	lw.show();
	
	DocWindow			dw;
	dw.show();
	/*
	lw.on_createNewFile();
	*/
	
	QTimer::singleShot(750, [&]()	{
		lw.on_createNewFile();
	});
	
	return a.exec();
}
