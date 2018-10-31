#ifndef JSONSCROLLWIDGET_H
#define JSONSCROLLWIDGET_H

#include <mutex>

#include <QScrollArea>
#include <QVector>
#include <QPointer>
#include <QSpacerItem>
//#include <QMap>
#include <QJsonObject>
#include <QJsonDocument>
#include <QSharedPointer>

#include "VVISF.hpp"
#include "JSONScrollEventFilter.h"
class JSONScrollEventFilter;
class JGMTop;




using namespace VVISF;




class JSONScrollWidget : public QScrollArea
{
	Q_OBJECT
	
public:
	JSONScrollWidget(QWidget * inParent=nullptr);
	~JSONScrollWidget();
	
	void loadDocFromISFController();
	void recreateJSONAndExport();
	
	void startScrolling(const Qt::Edge & inScrollDirection);
	void stopScrolling();
	
private slots:
	void scrollTimerCallback();
	
protected:
	//virtual bool eventFilter(QObject * watched, QEvent * event) Q_DECL_OVERRIDE;
	
private:
	std::recursive_mutex	itemLock;
	ISFDocRef				doc = nullptr;
	QSharedPointer<JGMTop>	top = nullptr;	//	JSON GUI model- top
	//JGMTopRef				top = nullptr;
	QTimer					*scrollTimer = nullptr;
	Qt::Edge				scrollDirection = Qt::BottomEdge;
	
	QVector<QPointer<QWidget>>		items;	//	weak refs 'cause the layout owns the widgets...
	QSpacerItem				*spacerItem = nullptr;
	JSONScrollEventFilter	*eventFilter = nullptr;
	
	void clearItems();
	void repopulateUI();
	
	
	int indexBasicInfo();
	int indexInputsGroupItem();
	int indexInputByIndex(const int & n);
	int indexInputByName(const std::string & n);
	int indexPassesGroupItem();
	int indexPassByIndex(const int & n);
};




//	calls recreateJSONAndExport() on the global singleton of JSONScrollWidget
void RecreateJSONAndExport();




#endif // JSONSCROLLWIDGET_H