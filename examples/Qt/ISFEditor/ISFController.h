#ifndef ISFCONTROLLER_H
#define ISFCONTROLLER_H

#include <mutex>
#include <vector>
#include <utility>

#include "VVISF.hpp"
#include "ISFUIItem.h"
#include <QSpacerItem>




using namespace VVGL;
using namespace VVISF;




class ISFController : public QObject
{
	Q_OBJECT
public:
	ISFController();
	~ISFController();
	
	void loadFile(const QString & inPathToLoad);
	void setRenderSize(const Size & inSize) { std::lock_guard<std::recursive_mutex> lockGuard(sceneLock); renderSize=inSize; }
	Size getRenderSize() { std::lock_guard<std::recursive_mutex> lockGuard(sceneLock); return renderSize; }
	ISFSceneRef getScene() { std::lock_guard<std::recursive_mutex> lockGuard(sceneLock); return scene; }
	vector<pair<int,string>> getSceneVertErrors() { std::lock_guard<std::recursive_mutex> lockGuard(sceneLock); return sceneVertErrors; }
	vector<pair<int,string>> getSceneFragErrors() { std::lock_guard<std::recursive_mutex> lockGuard(sceneLock); return sceneFragErrors; }
	
private:
	Size			renderSize = Size(640.0,480.0);
	
	std::recursive_mutex	sceneLock;	//	used to lock the 'scene'-related vars below
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