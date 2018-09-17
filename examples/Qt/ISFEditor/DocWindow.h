#ifndef DOCWINDOW_H
#define DOCWINDOW_H

#include <mutex>

#include <QWidget>
#include <QTimer>
#include <QFile>

#include "VVISF.hpp"

class SimpleSourceCodeEdit;
class QTableView;
class QListWidget;




namespace Ui {
	class DocWindow;
}




class DocWindow : public QWidget
{
	Q_OBJECT

public:
	explicit DocWindow(QWidget *parent = nullptr);
	~DocWindow();
	
	void clearFileAndVars();
	void updateContentsFromISFController();
	
	void saveOpenFile();
	void reloadFileFromTableView();
	bool contentsNeedToBeSaved();
	QString fragFilePath();
	
	
private:
	Ui::DocWindow		*ui;
	
	std::recursive_mutex	propLock;	//	used to lock access to all these vars
	//	we own all of these strings, and are responsible for freeing them
	QString				*_fragFilePath = nullptr;
	QString				*_fragFilePathContentsOnOpen = nullptr;	//	we retain the contents of the file as it was opened for comparison purposes
	bool				_fragEditsPerformed = false;
	QString				*_vertFilePath = nullptr;
	QString				*_vertFilePathContentsOnOpen = nullptr;
	bool				_vertEditsPerformed = false;
	
	QTimer				*_tmpFileSaveTimer = nullptr;	//	we save tmp files after ~2 sec of inactivity (this makes the backend reload the tmp file, which recompiles it automatically)
	
private slots:
	void tmpSaveTimerSlot();
};




//	gets the global singleton for this class, which is created in main()
DocWindow * GetDocWindow();




#endif // DOCWINDOW_H
