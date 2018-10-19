#ifndef JSONGUIPASS_H
#define JSONGUIPASS_H

#include <QObject>
class QLabel;
class QLineEdit;
class QCheckBox;
class QLabelClickable;
#include "JGMObject.h"




class JSONGUIPass
{
public:
	JSONGUIPass(const JGMPassRef & inPass) : _pass(inPass)	{}
	~JSONGUIPass()	{}
	
	void prepareDeleteLabel(QLabelClickable & deleteLabel);
	void prepareBufferNameEdit(QLineEdit & bufferNameEdit);
	void preparePBufferCBox(QCheckBox & pbufferCBox);
	void prepareFBufferCBox(QCheckBox & fbufferCBox);
	void prepareCustWidthEdit(QLineEdit & custWidthEdit);
	void prepareCustHeightEdit(QLineEdit & custHeightEdit);
	
	void refreshPassTitleLabel(QLabel & passNameLabel);
	void refreshBufferNameEdit(QLineEdit & bufferNameEdit);
	void refreshPBufferCBox(QCheckBox & pbufferCBox);
	void refreshFBufferCBox(QCheckBox & fbufferCBox);
	void refreshCustWidthEdit(QLineEdit & custWidthEdit);
	void refreshCustHeightEdit(QLineEdit & custHeightEdit);
	
	virtual void prepareUIItems() {};
	virtual void refreshUIItems() {};
	
protected:
	JGMPassRef		_pass = nullptr;
};




#endif // JSONGUIPASS_H