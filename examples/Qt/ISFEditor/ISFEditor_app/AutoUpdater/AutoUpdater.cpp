#include "AutoUpdater.h"

#include <QDebug>




static AutoUpdater * globalAutoUpdater = nullptr;




AutoUpdater::AutoUpdater(QObject * parent) :
	QObject(parent)
{
	qDebug() << __PRETTY_FUNCTION__;
}


void AutoUpdater::checkForUpdates()	{
	//src.checkForUpdates();
}


AutoUpdater * GetGlobalAutoUpdater()	{
	qDebug() << __PRETTY_FUNCTION__;
	
	if (globalAutoUpdater == nullptr)	{
		globalAutoUpdater = new AutoUpdater();
	}
	return globalAutoUpdater;
}
