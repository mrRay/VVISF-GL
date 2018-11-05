#ifndef ISFCONTROLLER_H
#define ISFCONTROLLER_H

#include <mutex>
#include <vector>
#include <utility>

#include <QSpacerItem>
#include <QFileSystemWatcher>

#include "VVISF.hpp"
#include "ISFUIItem.h"
#include "ISFGLBufferQWidget.h"




using namespace VVGL;
using namespace VVISF;




class ISFController : public QObject
{
	Q_OBJECT
public:
	ISFController();
	~ISFController();
	
	void loadFile(const QString & inPathToLoad);
	void setRenderSize(const Size & inSize) { std::lock_guard<std::recursive_mutex> lockGuard(sceneLock); _renderSize=inSize; }
	Size renderSize() { std::lock_guard<std::recursive_mutex> lockGuard(sceneLock); return _renderSize; }
	//ISFSceneRef getScene() { std::lock_guard<std::recursive_mutex> lockGuard(sceneLock); return scene; }
	//ISFDocRef getCurrentDoc() { std::lock_guard<std::recursive_mutex> lockGuard(sceneLock); return (scene==nullptr)?nullptr:scene->doc(); }
	ISFDocRef getCurrentDoc() { std::lock_guard<std::recursive_mutex> lockGuard(sceneLock); return currentDoc; }
	QString getCompiledVertexShaderString() { std::lock_guard<std::recursive_mutex> lockGuard(sceneLock); return (scene==nullptr) ? QString() : QString::fromStdString( scene->vertexShaderString() ); }
	QString getCompiledFragmentShaderString() { std::lock_guard<std::recursive_mutex> lockGuard(sceneLock); return (scene==nullptr) ? QString() : QString::fromStdString( scene->fragmentShaderString() ); }
	vector<pair<int,string>> getSceneVertErrors() { std::lock_guard<std::recursive_mutex> lockGuard(sceneLock); return sceneVertErrors; }
	vector<pair<int,string>> getSceneFragErrors() { std::lock_guard<std::recursive_mutex> lockGuard(sceneLock); return sceneFragErrors; }

public slots:
	//	the widget sends a signal to this slot every time it's about to redraw
	Q_SLOT void widgetRedrawSlot(ISFGLBufferQWidget * n);
	
private:
	Size			_renderSize = Size(640.0,480.0);
	
	std::recursive_mutex	sceneLock;	//	used to lock the 'scene'-related vars below
	QFileSystemWatcher		*sceneFileWatcher = nullptr;
	ISFDocRef				currentDoc = nullptr;
	ISFSceneRef				scene = nullptr;	//	this is the main scene doing all the rendering!
	bool					sceneIsFilter = false;
	vector<pair<int,string>>		sceneVertErrors;
	vector<pair<int,string>>		sceneFragErrors;
	
	QString					targetFile;
	
	QList<QPointer<ISFUIItem>>		sceneItemArray;
	
	QSpacerItem				*spacerItem = nullptr;	//	must be explicitly freed!

private:
	void populateLoadingWindowUI();
	void pushNormalizedMouseClickToPoints(const Size & inSize);
	void reloadTargetFile();
private slots:
	void aboutToQuit();
};




//	gets the global singleton for this class, which is created in main()
ISFController * GetISFController();




#endif // ISFCONTROLLER_H