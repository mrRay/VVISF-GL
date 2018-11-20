#ifndef JSONGUIBASICINFO_H
#define JSONGUIBASICINFO_H

#include <QWidget>

#include "JGMTop.h"




namespace Ui {
	class JSONGUIBasicInfo;
}




class JSONGUIBasicInfoWidget : public QWidget
{
	Q_OBJECT

public:
	explicit JSONGUIBasicInfoWidget(const JGMTopRef & inTop, QWidget *parent = nullptr);
	~JSONGUIBasicInfoWidget();
	
	void prepareToBeDeleted();

public slots:
	void descriptionFieldUsed();
	void creditFieldUsed();
	void categoriesFieldUsed();
	void vsnFieldUsed();

private:
	Ui::JSONGUIBasicInfo *ui;
	
	//VVISF::ISFDocRef		doc = nullptr;
	JGMTopRef				_top = nullptr;
};




#endif // JSONGUIBASICINFO_H
