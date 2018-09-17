#ifndef LOADINGWINDOW_H
#define LOADINGWINDOW_H

#include <QWidget>
class QItemSelection;
class QScrollArea;




namespace Ui {
	class LoadingWindow;
}




class LoadingWindow : public QWidget
{
	Q_OBJECT

public:
	explicit LoadingWindow(QWidget *parent = nullptr);
	~LoadingWindow();
	
	QScrollArea * getScrollArea();
	inline QString getBaseDirectory() { return baseDirectory; }
	
	void on_createNewFile();
	void on_loadFile(const QString & n);
	void on_saveFile();

public slots:
	void updateRenderSizeUI();

private slots:
	void on_loadUserISFsButton_clicked();
	void on_loadSystemISFsButton_clicked();
	void on_halveRenderRes_clicked();
	void on_doubleRenderRes_clicked();
	void on_renderResWidthWidget_valueChanged(int arg1);
	void on_renderResHeightWidget_valueChanged(int arg1);
	void on_saveUIValsToDefault_clicked();
	/*
	void newFileSelected(const QItemSelection &selected, const QItemSelection &deselected);
	*/
	
private:
	//static LoadingWindow	*globalIvar;
	Ui::LoadingWindow		*ui;
	QString					baseDirectory = QString("");
	
	void setBaseDirectory(const QString & inBaseDir);
};




//	gets the global singleton for this class, which is created in main()
LoadingWindow * GetLoadingWindow();




#endif // LOADINGWINDOW_H
