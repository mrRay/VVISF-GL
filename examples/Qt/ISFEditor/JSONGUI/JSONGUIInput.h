#ifndef JSONGUIINPUTWIDGET_H
#define JSONGUIINPUTWIDGET_H

#include <QObject>
class QLabel;
class QComboBox;
class QLineEdit;
class QLabelClickable;

#include "JGMObject.h"




class JSONGUIInput
{
public:
	JSONGUIInput(const JGMInputRef & inInput);
	~JSONGUIInput();
	
	void prepareInputNameEdit(QLineEdit & inputNameEdit);
	void prepareLabelField(QLineEdit & labelField);
	void prepareTypeCBox(QComboBox & typeCB);
	void prepareDeleteLabel(QLabelClickable & deleteLabel);
	
	void refreshInputNameEdit(QLineEdit & inputNameEdit);
	void refreshLabelField(QLineEdit & labelField);
	void refreshTypeCBox(QComboBox & typeCB);
	
	virtual void prepareUIItems() {};
	virtual void refreshUIItems() {};
	
protected:
	JGMInputRef		_input = nullptr;
};




#endif // JSONGUIINPUTWIDGET_H
