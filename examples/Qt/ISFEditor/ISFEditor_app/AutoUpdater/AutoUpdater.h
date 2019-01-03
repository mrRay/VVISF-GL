#ifndef AUTOUPDATER_H
#define AUTOUPDATER_H

#include <QObject>




class AutoUpdater : public QObject
{
	Q_OBJECT
	
public:
	explicit AutoUpdater(QObject * parent=nullptr);
	
public slots:
	Q_SLOT void checkForUpdates();
	
private:
};




AutoUpdater * GetGlobalAutoUpdater();




#endif // AUTOUPDATER_H