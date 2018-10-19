#include "JSONGUIInput.h"

#include <QObject>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QDebug>

#include "JSONScrollWidget.h"
#include "JGMTop.h"
#include "QLabelClickable.h"




JSONGUIInput::JSONGUIInput(const JGMInputRef & inInput) : _input(inInput)	{
}
JSONGUIInput::~JSONGUIInput()	{
}
void JSONGUIInput::prepareInputNameEdit(QLineEdit & inputNameEdit)	{
	QObject::disconnect(&inputNameEdit, 0, 0, 0);
	
	//	pressing enter on a line
	QObject::connect(&inputNameEdit, &QLineEdit::editingFinished, [&]()	{
		//qDebug() << __FUNCTION__ << " QLineEdit::editingFinished";
		
		JGMTop		*top = (_input==nullptr) ? nullptr : _input->top();
		if (top == nullptr)
			return;
		
		QString		origAttrName = _input->value("NAME").toString();
		QString		newString = inputNameEdit.text();
		//	if the new input name isn't valid (because an input by that name already exists)
		if (top->getInputNamed(newString)!=nullptr)	{
			//	if the new input name is the same as the existing name
			if (origAttrName == newString)	{
				//	deselect the text, remove focus from the widget
				inputNameEdit.deselect();
				inputNameEdit.clearFocus();
			}
			//	else the new input name is different- just...not valid (probably a dupe)
			else	{
				//	reset the displayed text and return (should still be editing)
				inputNameEdit.setText(origAttrName);
			}
		}
		//	else the new input name is valid
		else	{
			//	update the attribute
			_input->setValue("NAME", QJsonValue(newString));
			//	deselect the text, remove focus from the widget
			inputNameEdit.deselect();
			inputNameEdit.clearFocus();
			//	recreate the json & export the file
			RecreateJSONAndExport();
		}
		
	});
}
void JSONGUIInput::prepareLabelField(QLineEdit & labelField)	{
	QObject::disconnect(&labelField, 0, 0, 0);
	
	QObject::connect(&labelField, &QLineEdit::editingFinished, [&]()	{
		if (_input == nullptr)
			return;
		QString		tmpString = labelField.text();
		if (tmpString.length() < 1)
			_input->setValue("LABEL", QJsonValue::Undefined);
		else
			_input->setValue("LABEL", tmpString);
		RecreateJSONAndExport();
	});
}
void JSONGUIInput::prepareTypeCBox(QComboBox & typeCB)	{
	QObject::disconnect(&typeCB, 0, 0, 0);
	
	typeCB.clear();
	typeCB.addItem( "event" );
	typeCB.addItem( "bool" );
	typeCB.addItem( "long" );
	typeCB.addItem( "float" );
	typeCB.addItem( "point2D" );
	typeCB.addItem( "color" );
	typeCB.addItem( "image" );
	typeCB.addItem( "audio" );
	typeCB.addItem( "audioFFT" );
	
	QObject::connect(&typeCB, QOverload<const QString &>::of(&QComboBox::activated), [&](const QString & inText)	{
		if (_input == nullptr)
			return;
		_input->setValue("TYPE", inText);
		RecreateJSONAndExport();
	});
}
void JSONGUIInput::prepareDeleteLabel(QLabelClickable & deleteLabel)	{
	QObject::disconnect(&deleteLabel, 0, 0, 0);
	
	QObject::connect(&deleteLabel, &QLabelClickable::clicked, [&]()	{
		if (!_input.isNull())	{
			if (_input->top()->deleteInput(_input))	{
				RecreateJSONAndExport();
			}
		}
	});
}




void JSONGUIInput::refreshInputNameEdit(QLineEdit & inputNameEdit)	{
	//qDebug() << __PRETTY_FUNCTION__;
	QString		tmpString = (_input==nullptr) ? "" : _input->value("NAME").toString();
	inputNameEdit.setText(tmpString);
}
void JSONGUIInput::refreshLabelField(QLineEdit & labelField)	{
	QString		tmpString = (_input==nullptr) ? "" : _input->value("LABEL").toString();
	labelField.setText(tmpString);
}
void JSONGUIInput::refreshTypeCBox(QComboBox & typeCB)	{
	QString		tmpString = (_input==nullptr) ? "" : _input->value("TYPE").toString();
	typeCB.setCurrentText(tmpString);
}
