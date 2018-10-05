#include "JGMCDict.h"

#include <QJsonArray>
#include <QDebug>

#include "JGMTop.h"




JGMCDict::JGMCDict(const JGMDictType & inType, JGMTop * inTop, QObject *parent) :
	QObject(parent),
	_groupType(inType),
	_top(inTop)
{
}




JGMCPBufferDict::JGMCPBufferDict(JGMTop * inTop, QObject *parent) :
	JGMCDict(JGMDictType_PBuffer, inTop, parent)
{
	//qDebug() << __PRETTY_FUNCTION__;
	
	//	get the top-level ISF dict: we're going to parse its contents to populate our _dict ivar
	QJsonObject		&isfDict = _top->isfDict();
	
	//	get the PERSISTENT_BUFFERS value from the top-level ISF dict, parse it- we just need a name...
	if (isfDict.contains("PERSISTENT_BUFFERS"))	{
		auto			pbuffersVal = isfDict["PERSISTENT_BUFFERS"];
		if (!pbuffersVal.isUndefined())	{
			//	if PERSISTENT_BUFFERS is an array
			if (pbuffersVal.isArray())	{
				for (const QJsonValue & pbufferNameVal : pbuffersVal.toArray())	{
					if (!pbufferNameVal.isString())
						continue;
					QString			pbufferName = pbufferNameVal.toString();
					JGMPBufferRef		newPBuffer(new JGMPBuffer(pbufferName, inTop));
					if (newPBuffer != nullptr)
						_contents.insert(pbufferName, newPBuffer);
				}
			}
			//	else if PERSISTENT_BUFFERS is an object
			else if (pbuffersVal.isObject())	{
				for (const QString & pbufferKey : pbuffersVal.toObject().keys())	{
					//	make a persistent buffer from the name (it will populate itself)
					JGMPBufferRef		newPBuffer(new JGMPBuffer(pbufferKey, inTop));
					if (newPBuffer != nullptr)
						_contents.insert(pbufferKey, newPBuffer);
				}
			}
		}
	}
	//	run through all the PASSES, looking for a pass dict with a PERSISTENT flag
	QJsonValue			tmpVal = isfDict["PASSES"];
	if (tmpVal.isArray())	{
		for (const auto & passObjValue : tmpVal.toArray())	{
			if (!passObjValue.isObject())
				continue;
			QJsonObject		passObj = passObjValue.toObject();
			//	if there's a valid PERSISTENT flag in this pass dict and it's indicating a positive
			if (!passObj.contains("PERSISTENT") || !passObj.contains("TARGET"))
				continue;
			if (passObj["PERSISTENT"].toInt() < 1)
				continue;
			//	make a persistent buffer object (it will populate itself), insert it
			QString				pbufferName = passObj["TARGET"].toString();
			JGMPBufferRef		newPBuffer(new JGMPBuffer(pbufferName, inTop));
			if (newPBuffer != nullptr)
				_contents.insert(pbufferName, newPBuffer);
		}
	}
}


JGMPBufferRef JGMCPBufferDict::value(const QString & n)
{
	if (!_contents.contains(n))
		return nullptr;
	return _contents[n];
}

