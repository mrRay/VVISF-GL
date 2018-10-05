#include "JGMTop.h"

#include <QJsonObject>
#include <QJsonArray>




JGMInputRef JGMTop::getInputNamed(const QString & n)	{
	QVector<JGMInputRef>		&inputsArray = _inputs.contents();
	for (const JGMInputRef & input : inputsArray)	{
		QJsonValue		inputName = input->value(QString("NAME"));
		if (inputName.isString() && inputName.toString()==n)	{
			return input;
		}
	}
	return nullptr;
}

QVector<JGMPassRef> JGMTop::getPassesRenderingToBufferNamed(const QString & n)	{
	QVector<JGMPassRef>		returnMe;
	QVector<JGMPassRef>		passesArray = _passes.contents();
	for (const JGMPassRef & pass : passesArray)	{
		QJsonValue		passTarget = pass->value(QString("TARGET"));
		if (passTarget.isString() && passTarget.toString()==n)	{
			returnMe.append(pass);
		}
	}
	return returnMe;
}

JGMPBufferRef JGMTop::getPersistentBuferNamed(const QString & n)	{
	return _buffers.value(n);
}

int JGMTop::indexOfInput(const JGMInput & n)	{
	QVector<JGMInputRef>			inputsArray = _inputs.contents();
	int				i=0;
	int				returnMe = -1;
	for (const auto & input : inputsArray)	{
		if (&(*input) == &n)	{
			returnMe = i;
			break;
		}
		++i;
	}
	return returnMe;
}

int JGMTop::indexOfPass(const JGMPass & n)	{
	QVector<JGMPassRef>			passesArray = _passes.contents();
	int				i=0;
	int				returnMe = -1;
	for (const auto & pass : passesArray)	{
		if (&(*pass) == &n)	{
			returnMe = i;
			break;
		}
		++i;
	}
	return returnMe;
}

QString JGMTop::createNewInputName()	{
	QString		returnMe;
	int			count = 1;
	do	{
		if (count == 1)
			returnMe = QString("tmpInputName");
		else
			returnMe = QString("tmpInputName%1").arg(count);
		if (getInputNamed(returnMe) != nullptr)
			returnMe = QString("");
		++count;
	} while (returnMe.length()<1);
	return returnMe;
}

QJsonObject JGMTop::createJSONExport()	{
	QJsonObject		returnMe = _isfDict;
	QJsonArray		tmpArray;
	
	tmpArray = createJSONForInputs();
	if (tmpArray.size() < 1)
		returnMe.remove("INPUTS");
	else
		returnMe["INPUTS"] = QJsonValue(tmpArray);
	
	tmpArray = createJSONForPasses();
	if (tmpArray.size() < 1)
		returnMe.remove("PASSES");
	else
		returnMe["PASSES"] = QJsonValue(tmpArray);
	
	return returnMe;
}




QJsonArray JGMTop::createJSONForInputs()	{
	QJsonArray					returnMe;
	QVector<JGMInputRef>		&inputRefs = _inputs.contents();
	for (const JGMInputRef & inputRef : inputRefs)	{
		if (inputRef == nullptr)
			continue;
		QJsonObject		exportObj = inputRef->createExportObject();
		returnMe.append( QJsonValue(exportObj) );
	}
	return returnMe;
}
QJsonArray JGMTop::createJSONForPasses()	{
	QJsonArray					returnMe;
	QVector<JGMPassRef>			&passRefs = _passes.contents();
	for (const JGMPassRef & passRef : passRefs)	{
		if (passRef == nullptr)
			continue;
		QJsonObject		exportObj = passRef->createExportObject();
		returnMe.append( QJsonValue(exportObj) );
	}
	return returnMe;
}

/*
QJsonArray JGMTop::makeInputsArray()	{
	QJsonArray		returnMe;
	QVector<JSONGUIInputRef>			inputsArray = _inputs.contents();
	return returnMe;
}

QJsonArray JGMTop::makePassesArray()	{
}

QJsonObject JGMTop::makeBuffersObject()	{
}
*/
