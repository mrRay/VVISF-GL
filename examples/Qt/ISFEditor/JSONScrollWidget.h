#ifndef JSONSCROLLWIDGET_H
#define JSONSCROLLWIDGET_H

#include <mutex>

#include <QScrollArea>
#include <QVector>
#include <QPointer>
#include <QSpacerItem>

#include "VVISF.hpp"




using namespace VVISF;




class JSONScrollWidget : public QScrollArea
{
	Q_OBJECT
	
public:
	JSONScrollWidget(QWidget * inParent=nullptr);
	~JSONScrollWidget();
	
	void loadDocFromISFController();
	
private:
	std::recursive_mutex	itemLock;
	ISFDocRef				doc = nullptr;
	QVector<QPointer<QWidget>>		items;	//	weak refs 'cause the layout owns the widgets...
	QSpacerItem				*spacerItem = nullptr;
	
	void clearItems();
	
	int indexBasicInfo();
	int indexInputsGroupItem();
	int indexInputByIndex(const int & n);
	int indexInputByName(const std::string & n);
	int indexPassesGroupItem();
	int indexPassByIndex(const int & n);
};




#endif // JSONSCROLLWIDGET_H