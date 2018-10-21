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


