#ifndef SPOUTSOURCESWATCHER_H
#define SPOUTSOURCESWATCHER_H

#include <QObject>
#include <QDebug>
#include <QTimer>

#include "VVGL.hpp"
#include "Spout.h"




//	this class is meant to be used as a singleton.  it emits a signal when the list of spout sources has changed (connect to the singleton to be notified of changes to the list of spout sources)
class SpoutSourcesWatcher : public QObject
{
	Q_OBJECT

public:
	explicit SpoutSourcesWatcher(QObject *parent=nullptr) : QObject(parent) {
		qDebug() << __PRETTY_FUNCTION__;
		//	call the 'checkListOfSources' slot, which will call itself again
		_checkListOfSources();
	}
	~SpoutSourcesWatcher()	{
		qDebug() << __PRETTY_FUNCTION__;
	}

	//	use this static member function to retrieve a ptr to the global singleton watcher
	static SpoutSourcesWatcher * GetGlobalWatcher()	{
		static SpoutSourcesWatcher		_globalWatcher;
		return &_globalWatcher;
	}

	//	use this static member function to retrieve a QStringList with the current source names
	static QStringList GetListOfSources()	{
		SpoutSourcesWatcher		*watcher = GetGlobalWatcher();
		if (watcher == nullptr)
			return QStringList();
		return watcher->_getListOfSources();
	}

signals:
	//	connect to this signal on the global watcher (retrieved via GetGlobalWatcher()) to be notified of changes to the list of spout sources
	Q_SIGNAL void spoutSourcesUpdated();

private:
	SpoutReceiver		rxr;	//	this is the rxr we use to check spout
	QStringList			srcs;	//	the last list of sources we retrieved- we check this

	QStringList _getListOfSources() {
		QStringList		returnMe;
		int			senderCount = rxr.GetSenderCount();
		char		tmpChars[256];
		for (int i=0; i<senderCount; ++ i)	{
			memset(tmpChars, 0, 256);
			if (rxr.GetSenderName(i, tmpChars))	{
				QString		tmpString(tmpChars);
				//qDebug() << "\tshould be adding file named " << tmpString;
				returnMe.append(tmpString);
			}
			else
				qDebug() << "\tERR: problem getting sender name in " << __PRETTY_FUNCTION__;
		}
		return returnMe;
	}

private slots:
	//	the timer hits this once a second- it retrieves the current list of spout servers, and compares it to the last list.
	Q_SLOT void _checkListOfSources()	{
		//qDebug() << __PRETTY_FUNCTION__;
		//	get a list of sources now, compare it to the last list of sources, emit a signal if there's a change
		QStringList		currentList = _getListOfSources();
		if (currentList != srcs)	{
			srcs = currentList;
			emit spoutSourcesUpdated();
		}
		//	call this function again after a second to check again
		QTimer::singleShot(1000, this, &SpoutSourcesWatcher::_checkListOfSources);
	}
};




#endif // SPOUTSOURCESWATCHER_H
