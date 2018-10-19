#include "JSONGUIPass.h"

#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QDebug>

#include "QLabelClickable.h"
#include "JGMTop.h"
#include "JSONScrollWidget.h"




void JSONGUIPass::prepareDeleteLabel(QLabelClickable & deleteLabel)	{
	QObject::disconnect(&deleteLabel, 0, 0, 0);
	
	QObject::connect(&deleteLabel, &QLabelClickable::clicked, [&]()	{
		if (_pass == nullptr)
			return;
		JGMTop		*top = _pass->top();
		if (top == nullptr)
			return;
		if (top->deletePass(_pass))	{
			RecreateJSONAndExport();
		}
	});
}
void JSONGUIPass::prepareBufferNameEdit(QLineEdit & bufferNameEdit)	{
	QObject::disconnect(&bufferNameEdit, 0, 0, 0);
	
	QObject::connect(&bufferNameEdit, &QLineEdit::editingFinished, [&]()	{
		//	if the name hasn't changed, bail
		QJsonValue		origNameValue = _pass->value("TARGET");
		QString			origName = (origNameValue.isString()) ? origNameValue.toString() : QString("");
		QString			proposedName = bufferNameEdit.text();
		if (proposedName == origName)
			return;
		//	if there is already a buffer with that name, restore the original name and bail
		JGMTop			*top = _pass->top();
		QVector<JGMPassRef>		matchingPasses = top->getPassesRenderingToBufferNamed(proposedName);
		if (matchingPasses.size() > 0)	{
			bufferNameEdit.setText(origName);
			return;
		}
		//	update the attribute
		if (proposedName.size() < 1)
			_pass->setValue("TARGET", QJsonValue::Undefined);
		else
			_pass->setValue("TARGET", QJsonValue(proposedName));
		//	deselect the text, remove focus from the widget
		bufferNameEdit.deselect();
		bufferNameEdit.clearFocus();
		//	recreate the json & export the file
		RecreateJSONAndExport();
	});
}
void JSONGUIPass::preparePBufferCBox(QCheckBox & pbufferCBox)	{
	QObject::disconnect(&pbufferCBox, 0, 0, 0);
	
	QObject::connect(&pbufferCBox, &QCheckBox::clicked, [&]()	{
		//	if it's checked, add it
		if (pbufferCBox.isChecked())	{
			_pass->setValue("PERSISTENT", QJsonValue(true));
		}
		//	else it's not checked- just remove the value entirely
		else	{
			_pass->setValue("PERSISTENT", QJsonValue::Undefined);
		}
		//	recreate the JSON & export the file
		RecreateJSONAndExport();
	});
}
void JSONGUIPass::prepareFBufferCBox(QCheckBox & fbufferCBox)	{
	QObject::disconnect(&fbufferCBox, 0, 0, 0);
	
	QObject::connect(&fbufferCBox, &QCheckBox::clicked, [&]()	{
		//	if it's checked, add it
		if (fbufferCBox.isChecked())	{
			_pass->setValue("FLOAT", QJsonValue(true));
		}
		//	else it's not checked- just remove the value entirely
		else	{
			_pass->setValue("FLOAT", QJsonValue::Undefined);
		}
		//	recreate the JSON & export the file
		RecreateJSONAndExport();
	});
}
void JSONGUIPass::prepareCustWidthEdit(QLineEdit & custWidthEdit)	{
	QObject::disconnect(&custWidthEdit, 0, 0, 0);
	
	QObject::connect(&custWidthEdit, &QLineEdit::editingFinished, [&]()	{
		//	if the width string hasn't changed, bail
		QJsonValue		origWidthVal = _pass->value("WIDTH");
		QString			origWidth = (origWidthVal.isUndefined()) ? QString("") : origWidthVal.toString();
		QString			proposedWidth = custWidthEdit.text();
		if (origWidth == proposedWidth)
			return;
		//	update the value
		if (proposedWidth.size() < 1)
			_pass->setValue("WIDTH", QJsonValue::Undefined);
		else
			_pass->setValue("WIDTH", proposedWidth);
		//	deselect the text, remove focus from the widget
		custWidthEdit.deselect();
		custWidthEdit.clearFocus();
		//	recreate the json & export the file
		RecreateJSONAndExport();
	});
}
void JSONGUIPass::prepareCustHeightEdit(QLineEdit & custHeightEdit)	{
	QObject::disconnect(&custHeightEdit, 0, 0, 0);
	
	QObject::connect(&custHeightEdit, &QLineEdit::editingFinished, [&]()	{
		//	if the height string hasn't changed, bail
		QJsonValue		origHeightVal = _pass->value("HEIGHT");
		QString			origHeight = (origHeightVal.isUndefined()) ? QString("") : origHeightVal.toString();
		QString			proposedHeight = custHeightEdit.text();
		if (origHeight == proposedHeight)
			return;
		//	update the value
		if (proposedHeight.size() < 1)
			_pass->setValue("HEIGHT", QJsonValue::Undefined);
		else
			_pass->setValue("HEIGHT", proposedHeight);
		//	deselect the text, remove focus from the widget
		custHeightEdit.deselect();
		custHeightEdit.clearFocus();
		//	recreate the json & export the file
		RecreateJSONAndExport();
	});
}


void JSONGUIPass::refreshPassTitleLabel(QLabel & passNameLabel)	{
	JGMTop			*top = _pass->top();
	int				passIndex = top->indexOfPass(*_pass);
	passNameLabel.setText( QString("PASSINDEX %1").arg(passIndex) );
}
void JSONGUIPass::refreshBufferNameEdit(QLineEdit & bufferNameEdit)	{
	QString			tmpString = _pass->value("TARGET").toString();
	bufferNameEdit.setText(tmpString);
}
void JSONGUIPass::refreshPBufferCBox(QCheckBox & pbufferCBox)	{
	QJsonValue		targetNameVal = _pass->value("TARGET");
	QString			targetName;
	if (targetNameVal.isString())
		targetName = targetNameVal.toString();
	else
		targetName = QString("");
	
	JGMTop			*top = _pass->top();
	JGMPassRef		pbuffer = (targetName.size()<1) ? nullptr : top->getPersistentPassNamed(targetName);
	pbufferCBox.setCheckState( (pbuffer==nullptr) ? Qt::Unchecked : Qt::Checked );
}
void JSONGUIPass::refreshFBufferCBox(QCheckBox & fbufferCBox)	{
	//qDebug() << __PRETTY_FUNCTION__;
	QJsonValue			tmpVal = _pass->value("FLOAT");
	fbufferCBox.setCheckState(((tmpVal.isBool()&&tmpVal.toBool()) || (tmpVal.isDouble()&&tmpVal.toDouble()>0.0)) ? Qt::Checked : Qt::Unchecked);
	//int				tmpInt = _pass->value("FLOAT").toInt();
	//qDebug() << "\ttmpInt is " << tmpInt;
	//fbufferCBox.setCheckState( (tmpInt>0) ? Qt::Checked : Qt::Unchecked );
}
void JSONGUIPass::refreshCustWidthEdit(QLineEdit & custWidthEdit)	{
	QJsonValue		tmpVal = _pass->value("WIDTH");
	QString			tmpString;
	if (!tmpVal.isString())
		tmpString = QString("");
	else
		tmpString = tmpVal.toString();
	custWidthEdit.setText(tmpString);
}
void JSONGUIPass::refreshCustHeightEdit(QLineEdit & custHeightEdit)	{
	QJsonValue		tmpVal = _pass->value("HEIGHT");
	QString			tmpString;
	if (!tmpVal.isString())
		tmpString = QString("");
	else
		tmpString = tmpVal.toString();
	custHeightEdit.setText(tmpString);
}

