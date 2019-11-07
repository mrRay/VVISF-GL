#ifndef DOCWINDOW_H
#define DOCWINDOW_H

#include <mutex>

#include <QMainWindow>
#include <QTimer>
#include <QFile>

#include "VVISF.hpp"

namespace SimpleSourceCodeEdit	{
	class SimpleSourceCodeEditor;
}
using namespace SimpleSourceCodeEdit;

class QTableView;
class QListWidget;




namespace Ui {
	class DocWindow;
}




class DocWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit DocWindow(QWidget *parent = nullptr);
	~DocWindow();
	
	void updateContentsFromISFController();
	
	bool saveOpenFile();
	void reloadFileFromTableView();
	bool contentsNeedToBeSaved();
	QString fragFilePath();
	void pauseAutoSaveTimer();
	
	void reloadColorsAndSyntaxFormats();
	
public slots:
	Q_SLOT void on_actionNew_triggered();
	Q_SLOT void on_actionSave_triggered();
	Q_SLOT void on_actionImport_from_GLSLSandbox_triggered();
	Q_SLOT void on_actionImport_from_Shadertoy_triggered();
	Q_SLOT void on_actionPreferences_triggered();
	Q_SLOT void on_actionQuit_triggered();
	
	Q_SLOT void on_actionFind_triggered();
	Q_SLOT void on_actionFind_Previous_triggered();
	Q_SLOT void on_actionFind_Next_triggered();
	Q_SLOT void on_actionUse_selection_for_next_Find_triggered();
	
	Q_SLOT void on_actionCheck_for_Updates_triggered();
	Q_SLOT void on_actionAbout_triggered();
	Q_SLOT void on_actionGet_Help_triggered();

protected:
	virtual void closeEvent(QCloseEvent * event) Q_DECL_OVERRIDE;
	virtual void showEvent(QShowEvent * event) Q_DECL_OVERRIDE;
	void appQuitEvent();
	
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
	
	SimpleSourceCodeEditor * focusedSourceCodeEditor();
	
private slots:
	void tmpSaveTimerSlot();
};




//	gets the global singleton for this class, which is created in main()
DocWindow * GetDocWindow();




#endif // DOCWINDOW_H
