#include "TexUploadBenchmarkMainWindow.h"
#include <QApplication>
#include <VVGL.hpp>

#include <QOpenGLWidget>


using namespace std;
using namespace VVGL;


int main(int argc, char *argv[])
{
	//	we want all the widgets to share contexts, and we need to make a widget to get that shard context (there's no way to tell a widget to use a given context)
	QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts, true);
	QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL, true);
	QCoreApplication::setAttribute(Qt::AA_DontCheckOpenGLContextThreadAffinity, true);

	QSurfaceFormat::setDefaultFormat(CreateCompatibilityGLSurfaceFormat());
	//QSurfaceFormat::setDefaultFormat(CreateGL4SurfaceFormat());
	
	QApplication a(argc, argv);
	
	
	
	/*
	QOpenGLWidget			myWidget;
	myWidget.makeCurrent();
	if (myWidget.context() == nullptr)
		cout << "no context yet!\n";
	else
		cout << "made a context!\n";
	*/
	
	
	using namespace VVGL;
	
	
	//	make the window, which contains all the moving parts for the app
	TexUploadBenchmarkMainWindow w;
	w.show();

	return a.exec();
}
