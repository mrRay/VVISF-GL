#include "JGMTop.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>




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

JGMPassRef JGMTop::getPersistentPassNamed(const QString & n)	{
	auto			tmpPasses = getPassesRenderingToBufferNamed(n);
	for (const auto & tmpPass : tmpPasses)	{
		if (tmpPass->value("TARGET").toString() != n)	{
			continue;
		}
		QJsonValue		tmpVal = tmpPass->value("PERSISTENT");
		if (tmpVal.isUndefined())	{
			continue;
		}
		if ((tmpVal.isBool()&&tmpVal.toBool()) || (tmpVal.isDouble()&&tmpVal.toDouble()>0.0))	{
			return tmpPass;
		}
	}
	return nullptr;
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

bool JGMTop::deleteInput(const JGMInputRef & n)	{
	qDebug() << __PRETTY_FUNCTION__ << "... " << this;
	
	if (n.isNull())
		return false;
	JGMInput		*tmpInput = n.data();
	
	QVector<JGMInputRef>	&inputsArray = _inputs.contents();
	bool					foundInput = false;
	for (auto it = inputsArray.begin(); it!=inputsArray.end(); ++it)	{
		if (it->data() == tmpInput)	{
			foundInput = true;
			inputsArray.erase(it);
			break;
		}
	}
	
	return foundInput;
}
bool JGMTop::deletePass(const JGMPassRef & n)	{
	if (n.isNull())
		return false;
	JGMPass		*tmpPass = n.data();
	
	QVector<JGMPassRef>		&passArray = _passes.contents();
	bool					foundPass = false;
	for (auto it = passArray.begin(); it!=passArray.end(); ++it)	{
		if (it->data() == tmpPass)	{
			foundPass = true;
			passArray.erase(it);
			break;
		}
	}
	
	return foundPass;
}

QJsonObject JGMTop::createJSONExport()	{
	//	start off with the full-blown ISF dict
	QJsonObject		returnMe = _isfDict;
	QJsonArray		tmpArray;
	
	//	replace its INPUTS with a value we calculated
	tmpArray = createJSONForInputs();
	if (tmpArray.size() < 1)
		returnMe.remove("INPUTS");
	else
		returnMe["INPUTS"] = QJsonValue(tmpArray);
	
	//	replace its PASSES with a value we calcualted
	tmpArray = createJSONForPasses();
	if (tmpArray.size() < 1)
		returnMe.remove("PASSES");
	else
		returnMe["PASSES"] = QJsonValue(tmpArray);
	
	//	clear PERSISTENT_BUFFERS, it's an artifact of ISFv1
	returnMe.remove("PERSISTENT_BUFFERS");
	
	//	make sure we're flagged as ISFVSN 2.0
	returnMe["ISFVSN"] = QJsonValue(QString("2"));
	
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
