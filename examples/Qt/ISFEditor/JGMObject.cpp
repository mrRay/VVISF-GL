#include "JGMObject.h"

#include <QJsonArray>
#include <QDebug>

#include "JGMTop.h"




JGMObject::JGMObject(const QJsonObject & inJsonObj, JGMTop * inTop, QObject *parent) :
	QObject(parent), _dict(inJsonObj), _top(inTop)
{
}




JGMInput::JGMInput(const QJsonObject & inJsonObj, JGMTop * inTop, QObject *parent) :
	JGMObject(inJsonObj, inTop, parent)
{
	//qDebug() << __PRETTY_FUNCTION__;
}
/*
QJsonObject JGMInput::createExportObject()	{
	return _dict;
}
*/




JGMPass::JGMPass(const QJsonObject & inJsonObj, JGMTop * inTop, QObject *parent) :
	JGMObject(inJsonObj, inTop, parent)
{
	//qDebug() << __PRETTY_FUNCTION__;
}
/*
QJsonObject JGMPass::createExportObject()	{
	return _dict;
}
*/




JGMPBuffer::JGMPBuffer(const QString & inName, JGMTop * inTop, QObject *parent) :
	JGMObject(QJsonObject(), inTop, parent),
	_name(inName)
{
	//qDebug() << __PRETTY_FUNCTION__;
	
	//	get the top-level ISF dict: we're going to parse its contents to populate our _dict ivar
	QJsonObject		&isfDict = _top->isfDict();
	
	//	check the PERSISTENT_BUFFERS dict (or maybe array?) for info relevant to the buffer i'm looking for
	auto			pbuffersVal = isfDict["PERSISTENT_BUFFERS"];
	if (!pbuffersVal.isUndefined())	{
		if (pbuffersVal.isArray())	{
			//	do nothing- the array only contains strings/buffer names, and we already have the name...
		}
		else if (pbuffersVal.isObject())	{
			//	use the passed name to look up the sub-dict which describes the persistent buffer
			QJsonValue		pbufferVal = pbuffersVal.toObject().value(inName);
			if (pbufferVal.isObject())	{
				//	populate our _dict ivar with everything in this persistent buffer dict
				QJsonObject		pbuffer = pbufferVal.toObject();
				QJsonObject::iterator		it;
				for (it=pbuffer.begin(); it!=pbuffer.end(); ++it)	{
					_dict.insert(it.key(), it.value());
				}
			}
		}
	}
	auto			passesVal = isfDict["PASSES"];
	//	if there's a PASSES array
	if (passesVal.isArray())	{
		//	run through the passes
		QJsonArray		passes = passesVal.toArray();
		for (const QJsonValueRef & passJsonVal : passes)	{
			if (!passJsonVal.isObject())
				continue;
			//	if this pass's TARGET isn't the buffer we're looking for, go to the next pass
			QJsonObject		passJson = passJsonVal.toObject();
			if (passJson["TARGET"].toString()!=inName)
				continue;
			
			//	copy the WIDTH, HEIGHT, and FLOAT keys from the pass dict to our _dict ivar
			QJsonValue		tmpVal;
			
			tmpVal = passJson["WIDTH"];
			if (!tmpVal.isUndefined())
				_dict.insert("WIDTH", tmpVal.toString());
			
			tmpVal = passJson["HEIGHT"];
			if (!tmpVal.isUndefined())
				_dict.insert("HEIGHT", tmpVal.toString());
			
			tmpVal = passJson["FLOAT"];
			if (!tmpVal.isUndefined())	{
				if (tmpVal.isBool())
					_dict.insert("FLOAT", tmpVal.toBool());
				else
					_dict.insert("FLOAT", tmpVal.toInt());
			}
		}
	}
}
/*
QJsonObject JGMPBuffer::createExportObject()	{
	return QJsonObject();
}
*/


