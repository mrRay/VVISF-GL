#ifndef AUTOUPDATER_H
#define AUTOUPDATER_H

#include <QObject>
#include <updatecontroller.h>




class AutoUpdater : public QObject
{
	Q_OBJECT
	
public:
	explicit AutoUpdater(QObject * inParent=qApp);
	~AutoUpdater();
	
	void setParentWindow(QWidget * inParentWindow=nullptr);

public slots:
	Q_SLOT void checkForUpdates();

private:
	QtAutoUpdater::UpdateController		*_uc = nullptr;
};




AutoUpdater * GetGlobalAutoUpdater();




#endif // AUTOUPDATER_H