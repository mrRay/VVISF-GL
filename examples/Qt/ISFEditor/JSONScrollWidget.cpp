#include "JSONScrollWidget.h"

#include <QDebug>
#include <QLayout>

#include "ISFController.h"

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




JSONScrollWidget::JSONScrollWidget(QWidget * inParent) : QScrollArea(inParent)	{
	/*
	QPalette			p = palette();
	p.setColor(QPalette::Background, p.color(QPalette::Background).darker(105));
	setAutoFillBackground(true);
	setPalette(p);
	*/
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
	
	QWidget		*scrollWidget = widget();
	QLayout		*scrollLayout = (scrollWidget==nullptr) ? nullptr : scrollWidget->layout();
	if (scrollLayout == nullptr)
		return;
	
	//	create objects from the ISFDocRef
	lock_guard<recursive_mutex>		tmpLock(itemLock);
	
	//	make the basic info widget
	JSONGUIBasicInfoWidget		*basicInfo = new JSONGUIBasicInfoWidget(doc);
	QPointer<QWidget>		basicInfoPtr(basicInfo);
	items.append(basicInfoPtr);
	scrollLayout->addWidget(basicInfo);
	
	//	make the inputs widget
	JSONGUIGroupInputWidget		*inputsWidget = new JSONGUIGroupInputWidget();
	QPointer<QWidget>		inputsWidgetPtr(inputsWidget);
	items.append(inputsWidgetPtr);
	scrollLayout->addWidget(inputsWidget);
	
	//	make objects for each of the inputs
	vector<ISFAttrRef>		&docInputs = doc->getInputs();
	for (ISFAttrRef docInput : docInputs)	{
		QWidget		*newWidget = nullptr;
		switch (docInput->getType())	{
		case ISFValType_None:
			break;
		case ISFValType_Event:
			newWidget = new JSONGUIInputEventWidget(docInput);
			break;
		case ISFValType_Bool:
			newWidget = new JSONGUIInputBoolWidget(docInput);
			break;
		case ISFValType_Long:
			newWidget = new JSONGUIInputLongWidget(docInput);
			break;
		case ISFValType_Float:
			newWidget = new JSONGUIInputFloatWidget(docInput);
			break;
		case ISFValType_Point2D:
			newWidget = new JSONGUIInputPoint2DWidget(docInput);
			break;
		case ISFValType_Color:
			newWidget = new JSONGUIInputColorWidget(docInput);
			break;
		case ISFValType_Cube:
			break;
		case ISFValType_Image:
			newWidget = new JSONGUIInputImageWidget(docInput);
			break;
		case ISFValType_Audio:
			newWidget = new JSONGUIInputAudioWidget(docInput);
			break;
		case ISFValType_AudioFFT:
			newWidget = new JSONGUIInputAudioFFTWidget(docInput);
			break;
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

