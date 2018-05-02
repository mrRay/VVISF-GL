#include <QGuiApplication>
#include <VVGL.hpp>
#include <VVISF.hpp>
#include <QDebug>
#include <QImage>
#include "GLBufferQWindow.h"
#include <QCoreApplication>
#include <QTime>
#include <QTimer>
#include <QFile>


int main(int argc, char *argv[])
{
	QGuiApplication a(argc, argv);

	using namespace VVGL;
	using namespace VVISF;

	//	figure out what version GL we're going to use
	QSurfaceFormat		sfcFmt = CreateDefaultSurfaceFormat();
	//QSurfaceFormat		sfcFmt = CreateGL4SurfaceFormat();
	
	//	make the shared context using the vsn of GL you need to target.  all GL contexts are going to share this so they can share textures/etc with one another
	GLContextRef		sharedContext = CreateNewGLContextRef(nullptr, nullptr, sfcFmt);
	
	//	make the global buffer pool, tell it to share the shared context.  you need a pool to release, allocate, and recycle GL resources.
	CreateGlobalBufferPool(sharedContext);
	
	//	make the window, open it
	GLBufferQWindow			window(sharedContext);
	window.setFormat(sfcFmt);
	window.resize(QSize(800,600));
	window.show();
	window.startRendering();
	
	//	make an ISF scene
	ISFSceneRef		renderScene = CreateISFSceneRef();
	
	//	tell the ISF scene to load the included ISF shader
	QString			tmpString(":/files/CellMod.fs");
	QFile			tmpFile(tmpString);
	if (!tmpFile.open(QFile::ReadOnly | QFile::Text))	{
        qDebug() << "ERR: could not open CellMod file, " << __PRETTY_FUNCTION__;
		return 0;
	}
	QTextStream		tmpStream(&tmpFile);
	QString			fileContentsString = tmpStream.readAll();
	std::string		fileContents = fileContentsString.toStdString();
	//cout << "fileContents are:\n" << fileContents << endl;
	ISFDocRef		tmpDoc = make_shared<ISFDoc>(fileContents, ISFVertPassthru_GL2, nullptr);
    //cout << "isf doc is " << *tmpDoc << endl;
	renderScene->useDoc(tmpDoc);
	
	
	
	
	//	the window has its own thread on which it drives rendering- it emits a signal after each frame, which we're going to use to drive rendering with this lambda.
	QObject::connect(&window, &GLBufferQWindow::renderedAFrame, [&window,renderScene](){
		//	size the target texture so it's the same size as the window
		double				ltbbm = window.devicePixelRatio();
		VVGL::Size			windowSize = VVGL::Size(window.width()*ltbbm, window.height()*ltbbm);
		GLBufferRef			newBuffer = CreateRGBATex(windowSize,true);
		//	tell the scene to render to the target texture
		renderScene->renderToBuffer(newBuffer);
		//	tell the window to draw the texture we just rendered!
		window.drawBuffer(newBuffer);
	});

	return a.exec();
}
