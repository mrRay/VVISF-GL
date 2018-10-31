#include "JSONScrollWidget.h"

#include <QDebug>
#include <QLayout>
#include <QFile>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDragLeaveEvent>

#include "ISFController.h"

#include "JGMTop.h"

#include "JSONGUIGroupInputWidget.h"
#include "JSONGUIBasicInfoWidget.h"
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




JSONScrollWidget::JSONScrollWidget(QWidget * inParent) :
	QScrollArea(inParent)
{
	/*
	QPalette			p = palette();
	p.setColor(QPalette::Background, p.color(QPalette::Background).darker(105));
	setAutoFillBackground(true);
	setPalette(p);
	*/
	
	globalScrollWidget = this;
	
	//setViewport(new JSONScrollViewportWidget(this));
	//setViewport(new QWidget(this));
	
	//qDebug() << "viewport is " << viewport();
	viewport()->setAcceptDrops(true);
	
	eventFilter = new JSONScrollEventFilter(this, this);
	viewport()->installEventFilter(eventFilter);
	//qApp->installEventFilter(eventFilter);
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
	
	viewport()->removeEventFilter(eventFilter);
	//qApp->removeEventFilter(eventFilter);
	delete eventFilter;
	eventFilter = nullptr;
}


void JSONScrollWidget::loadDocFromISFController()	{
	//qDebug() << __PRETTY_FUNCTION__;
	//	clear the items
	clearItems();
	
	//	update the ISFDocRef from the ISF controller
	ISFController		*isfc = GetISFController();
	//ISFSceneRef			scene = (isfc==nullptr) ? nullptr : isfc->getScene();
	//doc = (scene==nullptr) ? nullptr : scene->doc();
	doc = (isfc==nullptr) ? nullptr : isfc->getCurrentDoc();

	if (doc == nullptr)
		return;
	
	QJsonObject			isfDict = QJsonObject();
	string				*jsonStr = doc->jsonString();
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
	QJsonDocument	exportJSONDoc(exportObj);
	
	string			tmpPath = doc->path();
	QFile			tmpFile(QString::fromStdString(tmpPath));
	if (!tmpFile.open(QFile::WriteOnly))	{
		qDebug() << "ERR: cannot open file to write (" << QString::fromStdString(tmpPath) << ")";
		return;
	}
	
	QString			jsonString = QString(exportJSONDoc.toJson());
	
	string			*fsStringPtr = doc->fragShaderSource();
	QString			fsString = (fsStringPtr==nullptr) ? QString() : QString::fromStdString(*fsStringPtr);
	QString			exportString;
	if (fsStringPtr == nullptr)
		exportString = QString("/*%1*/").arg(jsonString);
	else
		exportString = QString("/*%1*/\n%2").arg(jsonString).arg(fsString);
	
	//qDebug().noquote() << "*****************\n" << exportString << "\n*****************";
	
	tmpFile.write(exportString.toUtf8());
	tmpFile.close();
	
	
	/*
	QJsonObject		exportObj = top->createJSONExport();
	QJsonDocument	exportJSONDoc(exportObj);
	QFile			tmpFile("/Users/testadmin/Desktop/tmpFile.txt");
	tmpFile.open(QFile::WriteOnly);
	tmpFile.write(exportJSONDoc.toJson());
	tmpFile.close();
	*/
}
void JSONScrollWidget::startScrolling(const Qt::Edge & inScrollDirection)	{
	//qDebug() << __PRETTY_FUNCTION__ << ", " << this;
	
	lock_guard<recursive_mutex>		tmpLock(itemLock);
	scrollDirection = inScrollDirection;
	if (scrollTimer == nullptr)	{
		scrollTimer = new QTimer(this);
		scrollTimer->setInterval(50);
		scrollTimer->setSingleShot(false);
		connect(scrollTimer, &QTimer::timeout, this, &JSONScrollWidget::scrollTimerCallback);
		scrollTimer->start();
	}
}
void JSONScrollWidget::stopScrolling()	{
	//qDebug() << __PRETTY_FUNCTION__ << ", " << this;
	
	lock_guard<recursive_mutex>		tmpLock(itemLock);
	if (scrollTimer != nullptr)	{
		scrollTimer->stop();
		delete scrollTimer;
		scrollTimer = nullptr;
	}
}




void JSONScrollWidget::clearItems()	{
	QWidget		*scrollWidget = widget();
	QLayout		*scrollLayout = (scrollWidget==nullptr) ? nullptr : scrollWidget->layout();
	if (scrollLayout == nullptr)
		return;
	
	lock_guard<recursive_mutex>		tmpLock(itemLock);
	for (const QPointer<QWidget> & itemPtr : items)	{
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
			newWidget = new JSONGUIInputEventWidget(input, this);
		}
		else if (inputTypeString == "bool")	{
			newWidget = new JSONGUIInputBoolWidget(input, this);
		}
		else if (inputTypeString == "long")	{
			newWidget = new JSONGUIInputLongWidget(input, this);
		}
		else if (inputTypeString == "float")	{
			newWidget = new JSONGUIInputFloatWidget(input, this);
		}
		else if (inputTypeString == "point2D")	{
			newWidget = new JSONGUIInputPoint2DWidget(input, this);
		}
		else if (inputTypeString == "color")	{
			newWidget = new JSONGUIInputColorWidget(input, this);
		}
		else if (inputTypeString == "cube")	{
			//	intentionally blank, no UI should be shown?
		}
		else if (inputTypeString == "image")	{
			newWidget = new JSONGUIInputImageWidget(input, this);
		}
		else if (inputTypeString == "audio")	{
			newWidget = new JSONGUIInputAudioWidget(input, this);
		}
		else if (inputTypeString == "audioFFT")	{
			newWidget = new JSONGUIInputAudioFFTWidget(input, this);
		}
		
		if (newWidget == nullptr)
			continue;
		
		QPointer<QWidget>		newWidgetPtr(newWidget);
		items.append(newWidgetPtr);
		scrollLayout->addWidget(newWidget);
	}
	
	//	make the passes widget
	JSONGUIGroupPassWidget		*passesWidget = new JSONGUIGroupPassWidget(top);
	QPointer<QWidget>		passesWidgetPtr(passesWidget);
	items.append(passesWidgetPtr);
	scrollLayout->addWidget(passesWidget);
	
	//	make objects for each of the passes
	JGMCPassArray			&passesContainer = top->passesContainer();
	QVector<JGMPassRef>		&passes = passesContainer.contents();
	for (const JGMPassRef & pass : passes)	{
		if (pass == nullptr)
			continue;
		QWidget				*newWidget = new JSONGUIPassWidget(pass, this);
		if (newWidget == nullptr)
			continue;
		QPointer<QWidget>	newWidgetPtr(newWidget);
		items.append(newWidgetPtr);
		scrollLayout->addWidget(newWidget);
	}
	
	//	add the spacer at the bottom so the UI items are pushed to the top of the scroll area
	if (spacerItem == nullptr)
		spacerItem = new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Expanding);
	if (spacerItem != nullptr)
		scrollLayout->addItem(spacerItem);
	
}


void JSONScrollWidget::scrollTimerCallback()	{
	//qDebug() << __PRETTY_FUNCTION__;
	
	lock_guard<recursive_mutex>		tmpLock(itemLock);
	switch (scrollDirection)	{
	case Qt::TopEdge:
		//qDebug() << "\tscrolling content up";
		//scrollContentsBy(0, 50);
		verticalScrollBar()->setValue( verticalScrollBar()->value() - 25 );
		break;
	case Qt::BottomEdge:
		//qDebug() << "\tscrolling content down";
		verticalScrollBar()->setValue( verticalScrollBar()->value() + 25 );
		//scrollContentsBy(0, -50);
		break;
	default:
		break;
	}
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


/*
bool JSONScrollWidget::eventFilter(QObject * watched, QEvent * event)	{
	//return true;	//	'false' means "don't filter"
	
	if (!this->isAncestorOf(qobject_cast<QWidget*>(watched)))
		return false;
	
	switch (event->type())	{
	case QEvent::Paint:
		return false;
	
	case QEvent::DragEnter:
	case QEvent::DragLeave:
	case QEvent::DragMove:
	case QEvent::Drop:
		qDebug() << __PRETTY_FUNCTION__;
		qDebug() << "\twatched: " << watched;
		qDebug() << "\tevent: " << event;
		return false;
	
	default:
		return false;
	
	//default:
	//	return false;
	}
	
	return false;
}
*/



void RecreateJSONAndExport()	{
	if (globalScrollWidget == nullptr)
		return;
	globalScrollWidget->recreateJSONAndExport();
}

