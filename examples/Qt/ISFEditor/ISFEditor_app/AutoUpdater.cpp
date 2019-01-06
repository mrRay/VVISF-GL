#include "AutoUpdater.h"

#include <QDebug>
#include <QDir>
#include <QCoreApplication>
#include <updater.h>




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
	qDebug() << "AutoUpdater built for mac";
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
	qDebug() << "AutoUpdater built for win";
	QString			tmpPath = QString("C:/Qt/MaintenanceTool");
#else
	//	linux builds will fail to compile somewhere around here.  not sure where Qt is installed, don't have one handy!
#endif
	qDebug() << "making QtAutoUpdater with path to maintenance tool " << tmpPath;
	_uc = new QtAutoUpdater::UpdateController(tmpPath, inParent);
	
	
//#elif defined(QT_NO_DEBUG)	//	else QT_DEBUG not defined, release build
//	qDebug() << "AutoUpdater built in RELEASE mode";


//	//	no platform-specific code here, the maintenance tool is expected to be part of the install
//	_uc = new QtAutoUpdater::UpdateController(inParent);


//#endif
	
	
	
	
	
	QObject::connect(_uc, &QtAutoUpdater::UpdateController::runningChanged, [&](bool running) {
		qDebug() << "Running changed:" << running;
		if (_uc != nullptr)	{
			QtAutoUpdater::Updater	*u = _uc->updater();
			if (!u->exitedNormally())	{
				qDebug() << "\tupdate controller did NOT exit normally!";
				qDebug() << "\terror code is " << u->errorCode();
				qDebug() << "\terror is " << u->errorLog();
			}
		}
		//if(!running)
		//	qApp->quit();
	});
	
#if defined(Q_OS_MAC)
	/*
	//	macs need the auto updater to run as admin b/c they need to install filters in /Library/Graphics/ISF...
	_uc->setRunAsAdmin(true);
	*/
#endif
	//	start the update check -> AskLevel to give the user maximum control
	//_uc->start(QtAutoUpdater::UpdateController::AskLevel);
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
