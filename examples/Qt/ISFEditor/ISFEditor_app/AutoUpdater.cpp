#include "AutoUpdater.h"

#include <QDebug>
#include <QDir>
#include <QCoreApplication>
#include <QSettings>
#include <updater.h>

#include "VVGL.hpp"




static AutoUpdater		*_globalAutoUpdater = nullptr;




AutoUpdater::AutoUpdater(QObject * inParent) :
	QObject(inParent)
{
	//	create the update controller with the application as parent -> will live long enough start the tool on exit
	//	since there is no parent window, all dialogs will be top-level windows
//#if defined(QT_DEBUG)
//	qDebug() << "AutoUpdater built in DEBUG mode";
	if (_globalAutoUpdater == nullptr)
		_globalAutoUpdater = this;



#if defined(Q_OS_MAC)
	//qDebug() << "AutoUpdater built for mac";
	//QString			tmpPath = QString("/Users/testadmin/Qt/MaintenanceTool.app");
#if defined(QT_DEBUG)
	QDir			macDir = QDir("/Applications/ISF Editor/maintenancetool.app/Contents/MacOS");
#elif defined(QT_NO_DEBUG)
	QDir			macDir = QDir(QCoreApplication::applicationDirPath());
#endif
	QString			tmpPath = QString("/Applications/ISF Editor/maintenancetool.app");
	if (!macDir.cdUp())
		qDebug() << "ERR: Contents dir doesnt exist";
	else	{
		if (!macDir.cdUp())
			qDebug() << "ERR: app bundle dir doesnt exist";
		else	{
			if (!macDir.cdUp())
				qDebug() << "ERR: parent dir doesnt exist";
			else	{
				tmpPath = macDir.path() + "/maintenancetool.app";
			}
		}
	}
#elif defined(Q_OS_WIN)
	//qDebug() << "AutoUpdater built for win";
#if defined(QT_DEBUG)
	QDir			exeDir = QDir("C:/Program Files (x86)/ISF Editor");
#elif defined(QT_NO_DEBUG)
	QDir			exeDir = QDir(QCoreApplication::applicationDirPath());
#endif
	QString			tmpPath;
	if (!exeDir.exists())
		tmpPath = QString("C:/Qt/MaintenanceTool.exe");
	else
		tmpPath = exeDir.path() + "/maintenancetool.exe";
#else
	//	linux builds will fail to compile somewhere around here.  not sure where Qt is installed, don't have one handy!
#endif
	//qDebug() << "making QtAutoUpdater with path to maintenance tool " << tmpPath;
	_uc = new QtAutoUpdater::UpdateController(tmpPath, inParent);
	
	QObject::connect(_uc, &QtAutoUpdater::UpdateController::runningChanged, [&](bool running) {
		Q_UNUSED(running);
		//qDebug() << "Running changed:" << running;
		if (_uc != nullptr)	{
			QtAutoUpdater::Updater	*u = _uc->updater();
			if (!u->exitedNormally())	{
				qDebug() << "\tupdate controller did NOT exit normally!";
				qDebug() << "\terror code is " << u->errorCode();
				qDebug() << "\terror is " << u->errorLog();
			}
		}
	});
	
#if defined(Q_OS_MAC)
	/*
	//	macs need the auto updater to run as admin b/c they need to install filters in /Library/Graphics/ISF...
	_uc->setRunAsAdmin(true);
	*/
#endif
	QSettings		settings;
	if (settings.contains("updateCheckDate"))	{
		QDate			tmpDate = settings.value("updateCheckDate").toDate();
		QDate			nowDate = QDate::currentDate();
		if (tmpDate.daysTo(nowDate) >= 7)	{
			settings.setValue("updateCheckDate", nowDate);
			//	start the update check -> AskLevel to give the user maximum control
			_uc->start(QtAutoUpdater::UpdateController::AskLevel);
		}
	}
	//	else there's no update check date- insert one (don't check for an update immediately)
	else {
		settings.setValue("updateCheckDate", QDate::currentDate());
	}
}
AutoUpdater::~AutoUpdater()	{
	qDebug() << __PRETTY_FUNCTION__;
}


void AutoUpdater::setParentWindow(QWidget * inParentWindow)	{
	if (_uc != nullptr)	{
		_uc->setParentWindow(inParentWindow);
	}
}


void AutoUpdater::checkForUpdates()	{
	qDebug() << __PRETTY_FUNCTION__;
	//	start the update check -> AskLevel to give the user maximum control
	_uc->start(QtAutoUpdater::UpdateController::AskLevel);
}




AutoUpdater * GetGlobalAutoUpdater()	{
	if (_globalAutoUpdater == nullptr)
		_globalAutoUpdater = new AutoUpdater();
	return _globalAutoUpdater;
}
