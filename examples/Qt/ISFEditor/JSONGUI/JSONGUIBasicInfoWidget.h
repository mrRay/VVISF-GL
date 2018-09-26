#ifndef JSONGUIBASICINFO_H
#define JSONGUIBASICINFO_H

#include <QWidget>

#include "VVISF.hpp"




namespace Ui {
	class JSONGUIBasicInfo;
}




class JSONGUIBasicInfoWidget : public QWidget
{
	Q_OBJECT

public:
	explicit JSONGUIBasicInfoWidget(const VVISF::ISFDocRef & inDoc, QWidget *parent = nullptr);
	~JSONGUIBasicInfoWidget();

private:
	Ui::JSONGUIBasicInfo *ui;
	
	VVISF::ISFDocRef		doc = nullptr;
};




#endif // JSONGUIBASICINFO_H
