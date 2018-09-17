#include "ISFController.h"

#include <QMessageBox>
#include <QDebug>
#include <QSharedPointer>
#include <QScrollArea>
#include <QLayout>

#include <vector>

#include "LoadingWindow.h"
#include "DocWindow.h"




static ISFController * globalISFController = nullptr;




ISFController::ISFController()	{
	globalISFController = this;
	connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(aboutToQuit()));
}
ISFController::~ISFController()	{
	//	we have to explicitly free the spacer item
	if (spacerItem != nullptr)	{
		LoadingWindow			*lw = GetLoadingWindow();
		QScrollArea				*scrollArea = (lw==nullptr) ? nullptr : lw->getScrollArea();
		QWidget					*scrollWidget = (scrollArea==nullptr) ? nullptr : scrollArea->widget();
		QLayout					*scrollLayout = (scrollWidget==nullptr) ? nullptr : scrollWidget->layout();
		if (scrollLayout != nullptr)	{
			scrollLayout->removeItem(spacerItem);
		}
		
		delete spacerItem;
		spacerItem = nullptr;
	}
}


void ISFController::aboutToQuit()	{
	std::lock_guard<std::recursive_mutex>		tmpLock(sceneLock);
	scene = nullptr;
}
void ISFController::loadFile(const QString & inPathToLoad)	{
	qDebug() << __PRETTY_FUNCTION__;
	
	if (GetGlobalBufferPool() == nullptr)
		return;
	
	std::lock_guard<std::recursive_mutex>		tmpLock(sceneLock);
	if (scene == nullptr)
		scene = CreateISFSceneRef();
	if (scene == nullptr)
		return;

	scene->setThrowExceptions(true);
	//	tell the scene to load the file, catch exceptions so we can throw stuff
	try	{
		scene->useFile(inPathToLoad.toStdString());
	}
	catch (ISFErr & exc)	{
		QString		errString = QString("%1, %2").arg(QString::fromStdString(exc.general), QString::fromStdString(exc.specific));
		QMessageBox::warning(GetLoadingWindow(), "", errString, QMessageBox::Ok);
	}
	catch (...)	{
	}
	
	
	
	/*
	if (QThread::currentThread() == qApp->thread())
		qDebug() << "\tcurrent thread in load file is main thread";
	else
		qDebug() << "\tcurrent thread in load file is NOT main thread!";
	*/
	//	tell the scene to render a frame, so the ISFController can pull its compiled shaders and populate its UI items
	try	{
		scene->createAndRenderABuffer();
	}
	catch (ISFErr & exc)	{
		//QString		errString = QString("%1, %2").arg(QString::fromStdString(exc.general), QString::fromStdString(exc.specific));
		//QMessageBox::warning(GetLoadingWindow(), "", errString, QMessageBox::Ok);
		qDebug() << "\tERR: caught exception rendering first frame, " << __PRETTY_FUNCTION__;
	}
	catch (...)	{
	}
	
	
	
	
	populateUI();
}


void ISFController::pushRenderingResolutionToUI()	{
}
void ISFController::populateUI()	{
	//	get the loading window, bail if we can't
	LoadingWindow			*lw = GetLoadingWindow();
	if (lw == nullptr)
		return;
	
	//	get the scroll area's widget and its layout, which is where we're going to be adding stuff
	QScrollArea				*scrollArea = lw->getScrollArea();
	QWidget					*scrollWidget = (scrollArea==nullptr) ? nullptr : scrollArea->widget();
	QLayout					*scrollLayout = (scrollWidget==nullptr) ? nullptr : scrollWidget->layout();
	if (scrollWidget == nullptr)
		return;
	
	//	get a lock
	lock_guard<recursive_mutex>		tmpLock(sceneLock);
	for (const QPointer<ISFUIItem> sceneItemPtr : sceneItemArray)	{
		if (sceneItemPtr.isNull())
			continue;
		//	remove the widget from the layout and hide it
		scrollLayout->removeWidget(sceneItemPtr.data());
		sceneItemPtr->hide();
		//	remove the widget's layout item from the layout
		QLayoutItem			*itemLayoutItem = scrollLayout->itemAt(scrollLayout->indexOf(sceneItemPtr.data()));
		if (itemLayoutItem != nullptr)
			scrollLayout->removeItem(itemLayoutItem);
		//	delete the widget
		delete sceneItemPtr.data();
	}
	//	clear the array of items
	sceneItemArray.clear();
	//	if there's a spacer item, remove it from the layout
	if (spacerItem != nullptr)	{
		scrollLayout->removeItem(spacerItem);
	}
	//	else there's no spacer item- create one
	else	{
		spacerItem = new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Expanding);
	}
	
	//	if something's wrong and there's no scene, bail here
	if (scene == nullptr)
		return;
	
	//	run through the scene's inputs- we want to make a UI item for each...
	vector<ISFAttrRef>		sceneInputs = scene->getInputs();
	for (auto it=sceneInputs.rbegin(); it!=sceneInputs.rend(); ++it)	{
		ISFAttrRef		attrib = *it;
		if (attrib == nullptr)
			continue;
		//	if the attrib ISN'T the image filter's input image...
		if (!attrib->getIsFilterInputImage())	{
			//	make the UI item, store a weak ptr to it in the vector, add the UI item to the layout
			ISFUIItem		*newWidget = new ISFUIItem(attrib);
			QPointer<ISFUIItem>			newWidgetPtr(newWidget);
			sceneItemArray.append(newWidgetPtr);
			scrollLayout->addWidget(newWidgetPtr);
		}
	}
	
	//	add the spacer at the bottom so the UI items are pushed to the top of the scroll area
	if (spacerItem != nullptr)	{
		scrollLayout->addItem(spacerItem);
	}
}
void ISFController::pushNormalizedMouseClickToPoints(const Size & inSize)	{
}
void ISFController::reloadTargetFile()	{
}




ISFController * GetISFController()	{
	if (globalISFController == nullptr)
		globalISFController = new ISFController();
	return globalISFController;
}
