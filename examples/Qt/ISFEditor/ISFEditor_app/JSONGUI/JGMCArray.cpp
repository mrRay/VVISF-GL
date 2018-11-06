#include "JGMCArray.h"

#include <QJsonArray>
#include <QDebug>

#include "JGMObject.h"
#include "JGMTop.h"




JGMCArray::JGMCArray(const JGMCArrayType & inType, JGMTop * inTop, QObject *parent) : QObject(parent)	{
	_groupType = inType;
	top = inTop;
}






JGMCInputArray::JGMCInputArray(JGMTop * inTop, QObject *parent) :
	JGMCArray(JGMCArrayType_Input, inTop, parent)
{
	//qDebug() << __PRETTY_FUNCTION__;
	
	QJsonObject		&isfDict = top->isfDict();
	if (isfDict.contains("INPUTS"))	{
		auto		inputsVal = isfDict["INPUTS"];
		if (inputsVal.isArray())	{
			QJsonArray	inputsArray = inputsVal.toArray();
			for (const QJsonValue inputVal : inputsArray)	{
				if (inputVal.isObject())	{
					QJsonObject		inputObj = inputVal.toObject();
					JGMInputRef		newRef = JGMInputRef(new JGMInput(inputObj, top));
					_contents.append(newRef);
				}
			}
		}
	}
}








JGMCPassArray::JGMCPassArray(JGMTop * inTop, QObject *parent) :
	JGMCArray(JGMCArrayType_Pass, inTop, parent)
{
	//qDebug() << __PRETTY_FUNCTION__;
	
	QJsonObject		&isfDict = top->isfDict();
	if (isfDict.contains("PASSES"))	{
		auto		inputsVal = isfDict["PASSES"];
		if (inputsVal.isArray())	{
			QJsonArray	passesArray = inputsVal.toArray();
			for (const QJsonValue passVal : passesArray)	{
				if (passVal.isObject())	{
					QJsonObject		inputObj = passVal.toObject();
					JGMPassRef		newRef = JGMPassRef(new JGMPass(inputObj, top));
					_contents.append(newRef);
				}
			}
		}
	}
}
