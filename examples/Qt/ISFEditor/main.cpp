#include "MainWindow.h"
#include <QApplication>

#include "LoadingWindow.h"
#include "OutputWindow.h"
#include "DocWindow.h"




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
	
	QApplication a(argc, argv);
	
	MainWindow w;
	w.show();
	w.hide();
	
	OutputWindow		ow;
	ow.show();
	
	LoadingWindow		lw;
	lw.show();
	
	DocWindow			dw;
	dw.show();
	
	lw.on_createNewFile();

	return a.exec();
}
