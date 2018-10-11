#include "JSONScrollWidget.h"

#include <QDebug>
#include <QLayout>
#include <QFile>

#include "ISFController.h"

#include "JGMTop.h"

#include "JSONGUIBasicInfoWidget.h"
#include "JSONGUIGroupInputWidget.h"
#include "JSONGUIGroupPassWidget.h"
#include "JSONGUIInputAudioWidget.h"
#include "JSONGUIInputAudioFFTWidget.h"
#include "JSONGUIInputBoolWidget.h"
#include "JSONGUIInputColorWidget.h"
#include "JSONGUIInputEventWidget.h"
#include "JSONGUIInputFloatWidget.h"
#include "JSONGUIInputImageWidget.h"
#include "JSONGUIInputLongWidget.h"
#include "JSONGUIInputPoint2DWidget.h"
#include "JSONGUIPassWidget.h"




using namespace std;

static JSONScrollWidget * globalScrollWidget = nullptr;




JSONScrollWidget::JSONScrollWidget(QWidget * inParent) : QScrollArea(inParent)	{
	/*
	QPalette			p = palette();
	p.setColor(QPalette::Background, p.color(QPalette::Background).darker(105));
	setAutoFillBackground(true);
	setPalette(p);
	*/
	
	globalScrollWidget = this;
}
JSONScrollWidget::~JSONScrollWidget()	{
	clearItems();
	
	if (spacerItem != nullptr)	{
		QWidget			*scrollWidget = widget();
		QLayout			*scrollLayout = (scrollWidget==nullptr) ? nullptr : scrollWidget->layout();
		if (scrollLayout != nullptr)	{
			scrollLayout->removeItem(spacerItem);
		}
		
		delete spacerItem;
		spacerItem = nullptr;
	}
	
	if (top != nullptr)
		top = nullptr;
	
}


void JSONScrollWidget::loadDocFromISFController()	{
	qDebug() << __PRETTY_FUNCTION__;
	//	clear the items
	clearItems();
	
	//	update the ISFDocRef from the ISF controller
	ISFController		*isfc = GetISFController();
	ISFSceneRef			scene = (isfc==nullptr) ? nullptr : isfc->getScene();
	doc = (scene==nullptr) ? nullptr : scene->getDoc();

	if (doc == nullptr)
		return;
	
	QJsonObject			isfDict = QJsonObject();
	string				*jsonStr = doc->getJSONString();
	if (jsonStr != nullptr)	{
		QJsonDocument		tmpDoc = QJsonDocument::fromJson( jsonStr->c_str() );
		if (!tmpDoc.isEmpty() && tmpDoc.isObject())	{
			isfDict = QJsonObject(tmpDoc.object());
		}
	}
	
	//	update the JGMTop instance
	if (top != nullptr)
		top = nullptr;
	//top = QSharedPointer<JGMTop>(new JGMTop(isfDict));
	top = JGMTopRef(new JGMTop(isfDict));
	
	
	if (doc == nullptr)
		return;
	
	//	repopulate the UI
	repopulateUI();
}
void JSONScrollWidget::recreateJSONAndExport()	{
	qDebug() << __PRETTY_FUNCTION__;
	if (top==nullptr || doc==nullptr)
		return;
	
	QJsonObject		exportObj = top->createJSONExport();
	QJsonDocument	exportDoc(exportObj);
	QFile			tmpFile("/Users/testadmin/Desktop/tmpFile.txt");
	tmpFile.open(QFile::WriteOnly);
	tmpFile.write(exportDoc.toJson());
	tmpFile.close();
	
}




void JSONScrollWidget::clearItems()	{
	QWidget		*scrollWidget = widget();
	QLayout		*scrollLayout = (scrollWidget==nullptr) ? nullptr : scrollWidget->layout();
	if (scrollLayout == nullptr)
		return;
	
	lock_guard<recursive_mutex>		tmpLock(itemLock);
	for (const QPointer<QWidget> itemPtr : items)	{
		if (itemPtr.isNull())
			continue;
		//	remove the widget from the layout and hide it
		scrollLayout->removeWidget(itemPtr.data());
		itemPtr->hide();
		//	remove the widget's layout item from the layout
		QLayoutItem		*itemLayoutItem = scrollLayout->itemAt( scrollLayout->indexOf(itemPtr.data()) );
		if (itemLayoutItem != nullptr)
			scrollLayout->removeItem(itemLayoutItem);
		//	delete the widget
		delete itemPtr.data();
	}
	//	clear the array of items
	items.clear();
	//	if there's a spacer item, remove it from the layout
	if (spacerItem != nullptr)
		scrollLayout->removeItem(spacerItem);
	
	
}
void JSONScrollWidget::repopulateUI()	{
	QWidget		*scrollWidget = widget();
	QLayout		*scrollLayout = (scrollWidget==nullptr) ? nullptr : scrollWidget->layout();
	if (scrollLayout == nullptr || top==nullptr)
		return;
	
	//	create objects from the ISFDocRef
	lock_guard<recursive_mutex>		tmpLock(itemLock);
	
	//	make the basic info widget
	JSONGUIBasicInfoWidget		*basicInfo = new JSONGUIBasicInfoWidget(top);
	QPointer<QWidget>		basicInfoPtr(basicInfo);
	items.append(basicInfoPtr);
	scrollLayout->addWidget(basicInfo);
	
	//	make the inputs widget
	JSONGUIGroupInputWidget		*inputsWidget = new JSONGUIGroupInputWidget(top);
	QPointer<QWidget>		inputsWidgetPtr(inputsWidget);
	items.append(inputsWidgetPtr);
	scrollLayout->addWidget(inputsWidget);
	
	//	make objects for each of the inputs
	JGMCInputArray			&inputsContainer = top->inputsContainer();
	QVector<JGMInputRef>	&inputs = inputsContainer.contents();
	for (const JGMInputRef & input : inputs)	{
		if (input == nullptr)
			continue;
		QWidget		*newWidget = nullptr;
		QString		inputTypeString = input->value("TYPE").toString();
		if (inputTypeString == "event")	{
			newWidget = new JSONGUIInputEventWidget(input);
		}
		else if (inputTypeString == "bool")	{
			newWidget = new JSONGUIInputBoolWidget(input);
		}
		else if (inputTypeString == "long")	{
			newWidget = new JSONGUIInputLongWidget(input);
		}
		else if (inputTypeString == "float")	{
			newWidget = new JSONGUIInputFloatWidget(input);
		}
		else if (inputTypeString == "point2D")	{
			newWidget = new JSONGUIInputPoint2DWidget(input);
		}
		else if (inputTypeString == "color")	{
			newWidget = new JSONGUIInputColorWidget(input);
		}
		else if (inputTypeString == "cube")	{
			//	intentionally blank, no UI should be shown?
		}
		else if (inputTypeString == "image")	{
			newWidget = new JSONGUIInputImageWidget(input);
		}
		else if (inputTypeString == "audio")	{
			newWidget = new JSONGUIInputAudioWidget(input);
		}
		else if (inputTypeString == "audioFFT")	{
			newWidget = new JSONGUIInputAudioFFTWidget(input);
		}
		
		if (newWidget == nullptr)
			continue;
		
		QPointer<QWidget>		newWidgetPtr(newWidget);
		items.append(newWidgetPtr);
		scrollLayout->addWidget(newWidget);
	}
	
	//	make the passes widget
	JSONGUIGroupPassWidget		*passesWidget = new JSONGUIGroupPassWidget();
	QPointer<QWidget>		passesWidgetPtr(passesWidget);
	items.append(passesWidgetPtr);
	scrollLayout->addWidget(passesWidget);
	
	//	make objects for each of the passes
	
	//	add the spacer at the bottom so the UI items are pushed to the top of the scroll area
	if (spacerItem == nullptr)
		spacerItem = new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Expanding);
	if (spacerItem != nullptr)
		scrollLayout->addItem(spacerItem);
	
}
int JSONScrollWidget::indexBasicInfo()	{
	return 0;
}
int JSONScrollWidget::indexInputsGroupItem()	{
	return 0;
}
int JSONScrollWidget::indexInputByIndex(const int & n)	{
	return 0;
}
int JSONScrollWidget::indexInputByName(const std::string & n)	{
	return 0;
}
int JSONScrollWidget::indexPassesGroupItem()	{
	return 0;
}
int JSONScrollWidget::indexPassByIndex(const int & n)	{
	return 0;
}




void RecreateJSONAndExport()	{
	if (globalScrollWidget == nullptr)
		return;
	globalScrollWidget->recreateJSONAndExport();
}

